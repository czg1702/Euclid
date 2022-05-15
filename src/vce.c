// #include <stdlib.h>
#include <stdio.h>
#include <string.h>
// #include <sys/stat.h>
// #include <unistd.h>

#include "mdd.h"
// #include "mdx.h"
// #include "command.h"
// #include "cfg.h"
// #include "net.h"
#include "vce.h"
#include "utils.h"

static ArrayList *coor_sys_ls;
static ArrayList *space_ls;

void vce_init()
{
    coor_sys_ls = als_create(16, "CoordinateSystem *");
    space_ls = als_create(16, "MeasureSpace *");
}

int vce_append(EuclidCommand *ec)
{
    __uint32_t pkg_capacity = *((__uint32_t *)ec->bytes);
    size_t i = sizeof(int) + sizeof(short);
    char *data = ec->bytes;

    __uint64_t cs_id = *((__uint64_t *)slide_over_mem(data, sizeof(__uint64_t), &i));
    __uint32_t axes_count = *((__uint32_t *)slide_over_mem(data, sizeof(__uint32_t), &i));
    __uint32_t vals_count = *((__uint32_t *)slide_over_mem(data, sizeof(__uint32_t), &i));

    // printf("cs_id %lu, axes_count %ld, vals_count %ld\n", cs_id, axes_count, vals_count);

    CoordinateSystem *cs;
    int j, csz = als_size(coor_sys_ls);
    for (j = 0; j < csz; j++)
    {
        cs = als_get(coor_sys_ls, j);
        if (cs->id == cs_id)
        {
            // coordinate system is exist
            // Unload the existing logical multidimensional array.
            space_unload(cs_id);
            goto f_0;
        }
    }

    // create persistent file about the CoordinateSystem
    char cs_file_path[64];
    sprintf(cs_file_path, "/data/coor-sys-%lu", cs_id);
    append_file_data(cs_file_path, (void *)&cs_id, sizeof(cs_id));
    append_file_data(cs_file_path, (void *)&axes_count, sizeof(axes_count));
    append_file_data(cs_file_path, (void *)&vals_count, sizeof(vals_count));

    // Create a coordinate system object in memory
    cs = cs_create(&cs_id);
    for (j = 0; j < axes_count; j++)
    {
        Axis *axis = ax_create();
        cs_add_axis(cs, axis);
    }
    als_add(coor_sys_ls, cs);

f_0:
    while (i < pkg_capacity)
    {
        for (j = 0; j < axes_count; j++)
        {
            __uint32_t coor_pointer_len = *((__uint32_t *)slide_over_mem(data, sizeof(__uint32_t), &i));
            void *fragments = slide_over_mem(data, coor_pointer_len * sizeof(__uint64_t), &i);
            // Code for testing >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
            int i___;
            for (i___ = 0; i___ < coor_pointer_len; i___++)
            {
                printf("%lu  ", ((md_gid *)fragments)[i___]);
            }
            printf("\n");
            // Code for testing >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

            // In the new data range, traverse all the scales of each coordinate axis, and store all of them in the corresponding axis file.
            char axis_meta_file[128];
            sprintf(axis_meta_file, "/data/coor-sys-%lu-axis%d", cs_id, j);
            append_file_data(axis_meta_file, (void *)&coor_pointer_len, sizeof(__uint32_t));
            append_file_data(axis_meta_file, fragments, coor_pointer_len * sizeof(__uint64_t));

            // At the same time, hang the scale objects on the corresponding axis in the coordinate system object.
            Scale *scale = scal_create();
            scal_put_fragments(scale, coor_pointer_len, fragments);
            Axis *axis = cs_get_axis(cs, j);
            ax_set_scale(axis, scale);
        }
        i += (sizeof(double) + sizeof(char)) * vals_count;
    }

    // Store the newly added measure data and its positioning information in the data file.
    char data_file[128];
    sprintf(data_file, "/data/coor-sys-%lu-data", cs_id);

    // _offset = sizeof(pkg_capacity) + sizeof(intent) + sizeof(cs_id) + sizeof(axes_count) + sizeof(vals_count)
    int _offset = sizeof(size_t) + sizeof(short) + 3 * sizeof(int);

    append_file_data(data_file, (void *)(data + _offset), pkg_capacity - _offset);

    // Calculate the actual size of the multidimensional array corresponding to the coordinate system.
    unsigned long space_capacity = 1;
    for (j = 0; j < axes_count; j++)
    {
        Axis *axis = cs_get_axis(cs, j);
        ax_reordering(axis);
        int sz = ax_size(axis);
        // FIXME May exceed the maximum value of the long data type, if so, you need to customize the data type.
        space_capacity *= sz;
    }

    // specify the partition rule of the logical multidimensional array.
    __uint64_t partition_scope = space_capacity / SPACE_DEF_PARTITION_COUNT;
    if (space_capacity % SPACE_DEF_PARTITION_COUNT)
        partition_scope++;

    printf("[debug] space_capacity < %lu >, SPACE_DEF_PARTITION_COUNT < %d >, partition_scope < %lu >\n", space_capacity, SPACE_DEF_PARTITION_COUNT, partition_scope);

    // Creates a new logical multidimensional array object in memory.
    MeasureSpace *space = space_create(SPACE_DEF_PARTITION_COUNT, partition_scope, vals_count);

    // struct stat data_file_stat;
    // file_stat(data_file, &data_file_stat);

    // Traverse the data file and insert all measure data into the logical multidimensional array.
    FILE *data_f = open_file(data_file, "r");

    __uint8_t tmp_buf[0x01 << 10]; // 1024
    while (1)
    {
        __uint64_t measure_space_idx = 0;
        for (j = 0; j < axes_count; j++)
        {
            if (fread(tmp_buf, sizeof(int), 1, data_f) < 1)
                goto f_1;

            int scale_len = *((int *)tmp_buf);
            // printf(">>>>>>>>>>>>>> scale_len = scale_len%d\n", scale_len);
            fread(tmp_buf, sizeof(md_gid), scale_len, data_f);
            Axis *axis = cs_get_axis(cs, j);
            __uint64_t sc_posi = ax_scale_position(axis, scale_len, tmp_buf);
            __uint64_t ax_span = cs_axis_span(cs, j);
            measure_space_idx += sc_posi * ax_span;
        }
        size_t cell_mem_sz = vals_count * (sizeof(double) + sizeof(char));
        void *cell = mem_alloc_0(sizeof(measure_space_idx) + cell_mem_sz);
        *((unsigned long *)cell) = measure_space_idx;
        fread(cell + sizeof(measure_space_idx), cell_mem_sz, 1, data_f);

        space_add_measure(space, measure_space_idx, cell);
    }

f_1:
    fclose(data_f);

    space_plan(space);

    als_add(space_ls, space);

    return 0;
}

CoordinateSystem *cs_create(__uint64_t *id_addr)
{
    CoordinateSystem *cs = mem_alloc_0(sizeof(CoordinateSystem));
    if (id_addr != NULL)
        cs->id = *id_addr;
    cs->axes = als_create(32, "Axis *");
    return cs;
}

Axis *ax_create()
{
    Axis *ax = mem_alloc_0(sizeof(Axis));
    ax->rbtree = rbt_create("struct _axis_scale *", scal_cmp, scal__destory);
    return ax;
}

Axis *cs_get_axis(CoordinateSystem *cs, int axis_position)
{
    return als_get(cs->axes, axis_position);
}

void *scal__destory(void *scale)
{
    Scale *s = (Scale *)scale;
    release_mem(s->fragments);
    release_mem(s);
}

Scale *scal_create()
{
    return mem_alloc_0(sizeof(Scale));
}

void cs_add_axis(CoordinateSystem *cs, Axis *axis)
{
    als_add(cs->axes, axis);
}

void scal_put_fragments(Scale *scale, int fgs_len, void *fragments)
{
    scale->fragments_len = fgs_len;
    scale->fragments = mem_alloc_0(fgs_len * sizeof(__uint64_t));
    memcpy(scale->fragments, fragments, fgs_len * sizeof(__uint64_t));
}

Scale *scal__alloc(int fgs_len, void *fragments)
{
    Scale *s = scal_create();
    scal_put_fragments(s, fgs_len, fragments);
    return s;
}

void ax_set_scale(Axis *axis, Scale *scale)
{
    rbt_add(axis->rbtree, scale);
}

int scal_cmp(void *_one, void *_other)
{
    Scale *one = (Scale *)_one, *other = (Scale *)_other;

    int i, len_c = one->fragments_len < other->fragments_len ? one->fragments_len : other->fragments_len;
    __uint64_t *f = one->fragments;
    __uint64_t *o_f = other->fragments;
    for (i = 0; i < len_c; i++)
    {
        if (*(f + i) < *(o_f + i))
            return -1;
        if (*(f + i) > *(o_f + i))
            return 1;
    }
    return 0;
}

void space_unload(__uint64_t id)
{
    int i;
    for (i = 0; i < als_size(space_ls); i++)
    {
        MeasureSpace *space = (MeasureSpace *)als_get(space_ls, i);
        if (space->id == id)
        {
            als_remove(space_ls, space);
            space__destory(space);
            break;
        }
    }
}

void ax_reordering(Axis *axis)
{
    rbt__reordering(axis->rbtree);
}

int ax_size(Axis *axis)
{
    return rbt__size(axis->rbtree);
}

/**
 * @param cell is a block of memory
 *
 * cell structure:
 * 8 bytes - position
 * (
 *   8 bytes - measure value
 *   1 byte - null flag
 * )+
 *
 */
int cell_cmp(void *cell, void *other)
{
    unsigned long c_posi = *((unsigned long *)cell);
    unsigned long o_posi = *((unsigned long *)other);
    if (o_posi > c_posi)
        return 1;
    if (o_posi < c_posi)
        return -1;
    return 0;
}

static void *_cell__destory(void *cell)
{
    release_mem(cell);
}

MeasureSpace *space_create(size_t segment_count, size_t segment_scope, int cell_vals_count)
{
    MeasureSpace *s = mem_alloc_0(sizeof(MeasureSpace));
    s->cell_vals_count = cell_vals_count;
    s->segment_count = segment_count;
    s->segment_scope = segment_scope;
    s->tree_ls_h = mem_alloc_0(sizeof(RedBlackTree *) * segment_count);
    s->data_ls_h = mem_alloc_0(sizeof(void *) * segment_count);

    int i;
    for (i = 0; i < segment_count; i++)
        s->tree_ls_h[i] = rbt_create("*cell", cell_cmp, _cell__destory);

    return s;
}

void *build_space_measure(RBNode *node, void *callback_params)
{
    int cell_size = *((int *)callback_params); // bytes count
    void *block_addr = *((void **)(callback_params + sizeof(int)));
    memcpy(block_addr + cell_size * (node->index), node->obj + sizeof(long), cell_size);
}

void space_plan(MeasureSpace *space)
{
    int i;
    for (i = 0; i < space->segment_count; i++)
    {
        RedBlackTree *tree = space->tree_ls_h[i];
        rbt__reordering(tree);

        int cell_size = space->cell_vals_count * (sizeof(double) + sizeof(char));
        unsigned int actual_cells_sz = rbt__size(tree);
        int posi_cell_sz = (sizeof(unsigned long) + cell_size);
        space->data_ls_h[i] = mem_alloc_0(actual_cells_sz * posi_cell_sz);

        char callback_params[sizeof(int) + sizeof(void *)];
        *((int *)callback_params) = cell_size;
        memcpy(&(callback_params[sizeof(int)]), space->data_ls_h + i, sizeof(void *));
        rbt__scan_do(tree, callback_params, build_space_measure);
        rbt__clear(tree);
    }
}

__uint64_t ax_scale_position(Axis *axis, int fgs_len, void *fragments)
{
    Scale *s = scal__alloc(fgs_len, fragments);
    // scal__show(s);
    RBNode *n = rbt__find(axis->rbtree, s);
    return n->index;
}

__uint64_t cs_axis_span(CoordinateSystem *cs, int axis_order)
{
    unsigned int axes_count = als_size(cs->axes);
    unsigned long span = 1;
    int i;
    for (i = axes_count - 1; i > axis_order; i--)
    {
        Axis *x = cs_get_axis(cs, i);
        span *= rbt__size(x->rbtree);
    }
    return span;
}

void space_add_measure(MeasureSpace *space, __uint64_t measure_position, void *cell)
{
    // // // printf("[debug] space_add_measure ()    %lu\n", *((unsigned long *)cell));
    rbt_add(space->tree_ls_h[measure_position / space->segment_scope], cell);
}

void scal__show(Scale *s)
{
    printf("Scale <%p>\n", s);
}

void space__destory(MeasureSpace *s)
{
    size_t i;
    for (i = 0; i < s->segment_count; i++)
    {
        if (s->tree_ls_h[i])
            rbt__destory(s->tree_ls_h[i]);

        if (s->data_ls_h[i])
            release_mem(s->data_ls_h[i]);
    }

    release_mem(s->tree_ls_h);

    release_mem(s->data_ls_h);

    release_mem(s);
}

double *vce_vactors_values(MddTuple **tuples_matrix_h, unsigned long v_len)
{
    printf("// TODO ....................... %s:%d\n", __FILE__, __LINE__);

    MddMemberRole *mr_0 = als_get(tuples_matrix_h[0]->mr_ls, 0);
    Cube *cube = find_cube_by_gid(mr_0->dim_role->cube_gid);

    // code for testing ! >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    Cube_print(cube);
    // code for testing ? >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    unsigned long i, j;

    CoordinateSystem *coor = NULL;
    for (i = 0; i < als_size(coor_sys_ls); i++)
    {
        if (((CoordinateSystem *)als_get(coor_sys_ls, i))->id = cube->gid)
        {
            coor = als_get(coor_sys_ls, i);
        }
    }

    for (i = 0; i < v_len; i++)
    {
        MddTuple *tuple = tuples_matrix_h[i];
        for (j = 0; j < als_size(tuple->mr_ls); j++)
        {
            MddMemberRole *mr = als_get(tuple->mr_ls, j);
            Member *m = mr->member;
            if (!m->abs_path)
                mdd__gen_mbr_abs_path(m);
            Member_print(m);
        }
        Tuple_print(tuple);
    }
    return NULL;
}