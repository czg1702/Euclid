#include <stdio.h>
#include <string.h>

#include "mdd.h"
#include "mdx.h"
#include "command.h"
#include "cfg.h"
#include "net.h"
#include "vce.h"

static md_gid lastest_md_gid = -1;

static ArrayList *dims_pool;
static ArrayList *member_pool;
static ArrayList *cubes_pool;
static Member *_create_member_lv1(Dimension *dim, char *mbr_name);
static Member *_create_member_child(Member *parent, char *child_name);

static ArrayList *select_def__build_axes(SelectDef *);

static Cube *select_def__get_cube(SelectDef *);

static MddTuple *cube__basic_ref_vector(Cube *);

static MddTuple *ax_def__head_ref_tuple(AxisDef *, MddTuple *, Cube *);

static MddTuple *tuple__merge(MddTuple *cxt_tuple, MddTuple *tuple_frag);

static MddAxis *ax_def__build(AxisDef *, MddTuple *, Cube *);

static unsigned int mdd_ax__len(MddAxis *);

static unsigned int mdd_set__len(MddSet *);

static MddTuple *ids_tupledef__build(TupleDef *t_def, MddTuple *context_tuple, Cube *cube);

Member *_new_member(char *name, md_gid dim_gid, md_gid parent_gid, __u_short lv);

Dimension *create_dimension(char *dim_name)
{
	if (strlen(dim_name) >= MD_ENTITY_NAME_BYTSZ)
	{
		printf("[WARN] - dim name too long <%s>\n", dim_name);
		return NULL;
	}

	// 1 - create a dimension object.
	Dimension *dim = (Dimension *)mem_alloc_0(sizeof(Dimension));
	dim->gid = gen_md_gid();
	memcpy(dim->name, dim_name, strlen(dim_name));
	printf("[INFO] create dimension [ %ld ] %s\n",dim->gid,dim->name);

	// 2 - save the dim-obj into a persistent file.
	append_file_data(META_DEF_DIMS_FILE_PATH, (char *)dim, sizeof(Dimension));

	// 3 - put the dim-obj into a mem-container.
	if (dims_pool == NULL)
		dims_pool = als_create(128, "dimensions pool");

	als_add(dims_pool, dim);
	// printf("========================= dim->name %s\n", dim->name);
	return dim;
}

int create_dims(ArrayList *dim_names)
{
	__uint32_t i, sz = als_size(dim_names);
	for (i = 0; i < sz; i++)
	{
		char *dim_name = (char *)als_get(dim_names, i);
		create_dimension(dim_name);
	}
	return 0;
}

md_gid gen_md_gid()
{
	while (1)
	{
		long microseconds = now_microseconds();
		if (microseconds > lastest_md_gid)
			return lastest_md_gid = microseconds;
		usleep(2);
	}
}

Member *create_member(ArrayList *mbr_path)
{
	unsigned int sz = als_size(mbr_path);
	// // printf("[debug] @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ ----------------------------- sz = %u\n", sz);
	int i = 0;
	int new_leaf_mbr_lv = sz - 1;

	Dimension *dim = find_dim_by_name(als_get(mbr_path, 0));
	if (dim == NULL)
		dim = create_dimension(als_get(mbr_path, 0));

	Member *mbr_lv1 = find_member_lv1(dim, als_get(mbr_path, 1));
	if (mbr_lv1 == NULL)
	{
		mbr_lv1 = _create_member_lv1(dim, als_get(mbr_path, 1));
		// if (mdd_mbr__level(mbr_lv1) > new_leaf_mbr_lv)
		// 	mdd_mbr__set_as_leaf(mbr_lv1);
		// // printf("[debug] @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@% 32s    level = %d\n", mbr_lv1->name, mbr_lv1->lv);
		append_file_data(META_DEF_MBRS_FILE_PATH, (char *)mbr_lv1, sizeof(Member));

		als_add(member_pool, mbr_lv1);
	}

	if (mbr_lv1->lv < new_leaf_mbr_lv && mdd_mbr__is_leaf(mbr_lv1))
	{
		mdd_mbr__set_as_leaf(mbr_lv1);
		append_file_data(META_DEF_MBRS_FILE_PATH, (char *)mbr_lv1, sizeof(Member));
	}

	Member *p_m = mbr_lv1;
	Member *m = mbr_lv1;
	for (i = 2; i < sz; i++)
	{
		m = find_member_child(p_m, als_get(mbr_path, i));
		if (m == NULL)
		{
			m = _create_member_child(p_m, als_get(mbr_path, i));
			// // printf("[debug] @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@% 32s    level = %d\n", m->name, m->lv);
			append_file_data(META_DEF_MBRS_FILE_PATH, (char *)m, sizeof(Member));
			als_add(member_pool, m);
		}

		if (m->lv < new_leaf_mbr_lv && mdd_mbr__is_leaf(m))
		{
			mdd_mbr__set_as_leaf(m);
			append_file_data(META_DEF_MBRS_FILE_PATH, (char *)m, sizeof(Member));
		}

		p_m = m;
	}

	return m;
}

int create_members(ArrayList *mbrs_info_als)
{
	if (member_pool == NULL)
		member_pool = als_create(256, "members pool | Member *");

	unsigned int size = als_size(mbrs_info_als);
	int i = 0;
	while (i < size)
	{
		create_member(als_get(mbrs_info_als, i++));
	}
	return 0;
}

Dimension *find_dim_by_name(char *dim_name)
{
	if (dims_pool == NULL)
		return NULL;

	unsigned int i = 0, sz = als_size(dims_pool);
	while (i < sz)
	{
		Dimension *dim = als_get(dims_pool, i++);
		if (strcmp(dim_name, dim->name) == 0)
			return dim;
	}

	return NULL;
}

Dimension *find_dim_by_gid(md_gid dim_gid)
{
	if (dims_pool == NULL)
		return NULL;

	unsigned int i = 0, sz = als_size(dims_pool);
	while (i < sz)
	{
		Dimension *dim = als_get(dims_pool, i++);
		if (dim->gid == dim_gid)
			return dim;
	}

	return NULL;
}

Member *find_member_lv1(Dimension *dim, char *mbr_name)
{
	if (member_pool == NULL)
		return NULL;

	__uint32_t i = 0, sz = als_size(member_pool);
	while (i < sz)
	{
		Member *mbr = als_get(member_pool, i++);
		if ((strcmp(mbr_name, mbr->name) == 0) && dim->gid == mbr->dim_gid)
			return mbr;
	}
	return NULL;
}

Member *find_member_child(Member *parent_mbr, char *child_name)
{
	__uint32_t i = 0, sz = als_size(member_pool);
	while (i < sz)
	{
		Member *mbr = als_get(member_pool, i++);
		if ((strcmp(child_name, mbr->name) == 0) && parent_mbr->gid == mbr->p_gid)
			return mbr;
	}
	return NULL;
}

static Member *_create_member_lv1(Dimension *dim, char *mbr_name)
{
	return _new_member(mbr_name, dim->gid, 0, 1);
}

static Member *_create_member_child(Member *parent, char *child_name)
{
	return _new_member(child_name, parent->dim_gid, parent->gid, parent->lv + 1);
}

Member *_new_member(char *name, md_gid dim_gid, md_gid parent_gid, __u_short lv)
{
	if (strlen(name) >= MD_ENTITY_NAME_BYTSZ)
		return NULL;

	Member *mbr = mem_alloc_0(sizeof(Member));
	memcpy(mbr->name, name, strlen(name));
	mbr->gid = gen_md_gid();
	mbr->dim_gid = dim_gid;
	mbr->p_gid = parent_gid;
	mbr->lv = lv;
	printf("[INFO] new Member - dim_gid [ %ld ] p_gid [% 17ld ] gid [ %ld ] name [ %s ] lv [ %d ]\n",mbr->dim_gid,mbr->p_gid,mbr->gid,mbr->name,mbr->lv);

	// printf("******************************** mbr->name %s\n", mbr->name);
	// Code for testing ! >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	// printf("// Code for testing ! >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	// Member_print(mbr);
	// Code for testing ? >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	return mbr;
}

int build_cube(char *name, ArrayList *dim_role_ls, ArrayList *measures)
{
	if (strlen(name) >= MD_ENTITY_NAME_BYTSZ)
		return -1;

	if (cubes_pool == NULL)
		cubes_pool = als_create(32, "cubes pool");

	// Create a cube object.
	Cube *cube = (Cube *)mem_alloc_0(sizeof(Cube));
	memcpy(cube->name, name, strlen(name));
	cube->gid = gen_md_gid();
	cube->dim_role_ls = als_create(24, "DimensionRole *");
	cube->measure_mbrs = als_create(12, "Member *");
	printf("[INFO] new Cube - gid [ %ld ] name [ %s ]\n", cube->gid, cube->name);

	// Create several dimensional role objects and associate them to the cube.
	size_t i, dr_sz = als_size(dim_role_ls);
	for (i = 0; i < dr_sz; i += 2)
	{
		char *dim_name = als_get(dim_role_ls, i);
		char *dim_role_name = als_get(dim_role_ls, i + 1);
		Dimension *dim = find_dim_by_name(dim_name);

		DimensionRole *d_role = mem_alloc_0(sizeof(DimensionRole));
		d_role->sn = i / 2;
		memcpy(d_role->name, dim_role_name, strlen(dim_role_name));
		d_role->gid = gen_md_gid();
		d_role->cube_gid = cube->gid;
		d_role->dim_gid = dim->gid;
		printf("[INFO] new DimensionRole - Cube [ %ld % 16s ] Dim [ %ld % 16s ] DR [ %ld % 16s ]\n", 
			cube->gid, cube->name, dim->gid, dim->name, d_role->gid, d_role->name);

		als_add(cube->dim_role_ls, d_role);
	}

	// Create a measure dimension object.
	Dimension *mear_dim = (Dimension *)mem_alloc_0(sizeof(Dimension));
	mear_dim->gid = gen_md_gid();
	memcpy(mear_dim->name, MEASURE_DIM_COMM_NAME, strlen(MEASURE_DIM_COMM_NAME));
	cube->measure_dim = mear_dim;

	// Create several measure dimension members.
	size_t mea_sz = als_size(measures);
	for (i = 0; i < mea_sz; i++)
	{
		Member *mea_mbr = _new_member(als_get(measures, i), mear_dim->gid, 0, 1);
		als_add(cube->measure_mbrs, mea_mbr);
	}

	// Each cube uses a persistent file separately.
	char cube_file[128];
	sprintf(cube_file, "/meta/cube_%lu", cube->gid);
	append_file_data(cube_file, (char *)cube, sizeof(Cube));
	append_file_uint(cube_file, als_size(cube->dim_role_ls));
	__uint32_t r_sz = als_size(cube->dim_role_ls);
	for (i = 0; i < r_sz; i++)
	{
		DimensionRole *_d_r = als_get(cube->dim_role_ls, i);
		append_file_data(cube_file, (char *)_d_r, sizeof(DimensionRole));
	}
	append_file_data(cube_file, (char *)cube->measure_dim, sizeof(Dimension));
	append_file_uint(cube_file, als_size(cube->measure_mbrs));
	__uint32_t mm_sz = als_size(cube->measure_mbrs);
	for (i = 0; i < mm_sz; i++)
	{
		Member *mea_mbr = als_get(cube->measure_mbrs, i);
		append_file_data(cube_file, (char *)mea_mbr, sizeof(Member));
	}

	als_add(cubes_pool, cube);

	return 0;
}

// Measure values will be stored in local memory and disk.
int store_measure(EuclidCommand *ec)
{
	// Store in the current node.
	return vce_append(ec);
}

int distribute_store_measure(EuclidCommand *ec)
{
	if (d_nodes_count() < 1 || rand() % 2)
	{
		return store_measure(ec); // Store in the current node.
	}

	return send(random_child_sock(), ec->bytes, *((int *)(ec->bytes)), 0) == (ssize_t)(*((int *)(ec->bytes)));
}

int insert_cube_measure_vals(char *cube_name, ArrayList *ls_ids_vctr_mear)
{
	size_t data_m_capacity = 2 * 1024 * 1024, data_m_sz = sizeof(__uint32_t) + sizeof(__uint16_t);
	char *data = mem_alloc_0(data_m_capacity);
	*((__uint16_t *)(data + sizeof(__uint32_t))) = INTENT__INSERT_CUBE_MEARSURE_VALS;

	Cube *cube = find_cube_by_name(cube_name);
	*((md_gid *)(data + data_m_sz)) = cube->gid;
	data_m_sz += sizeof(md_gid);

	*((__uint32_t *)(data + data_m_sz)) = als_size(cube->dim_role_ls);
	data_m_sz += sizeof(__uint32_t);

	*((__uint32_t *)(data + data_m_sz)) = als_size(cube->measure_mbrs);
	data_m_sz += sizeof(__uint32_t);

	__uint32_t i, j, k, sz = als_size(ls_ids_vctr_mear);
	for (i = 0; i < sz; i++)
	{
		IDSVectorMears *ids_vm = als_get(ls_ids_vctr_mear, i);
		__uint32_t vct_sz = als_size(ids_vm->ls_vector);
		for (j = 0; j < vct_sz; j++)
		{
			ArrayList *mbr_path_str = als_get(ids_vm->ls_vector, j);

			__uint32_t mbr_path_len = als_size(mbr_path_str);

			void *abs_path = gen_member_gid_abs_path(cube, mbr_path_str);
			size_t ap_bsz = *((__uint32_t *)abs_path) * sizeof(md_gid) + sizeof(__uint32_t);
			memcpy(data + data_m_sz, abs_path, ap_bsz);
			data_m_sz += ap_bsz;
		}

		__uint32_t cube_mmbrs_sz = als_size(cube->measure_mbrs);
		__uint32_t mv_sz = als_size(ids_vm->ls_mears_vals);
		for (j = 0; j < cube_mmbrs_sz; j++)
		{
			Member *mm = als_get(cube->measure_mbrs, j);

			// Set the null-value flag bit, 1 means the measure-value is null.
			*(data + data_m_sz + sizeof(double)) = 1;

			for (k = 0; k < mv_sz; k++)
			{
				char *mm_name = als_get(ids_vm->ls_mears_vals, k);
				if (strcmp(mm_name, mm->name) != 0)
					continue;

				*((double *)(data + data_m_sz)) = *((double *)(als_get(ids_vm->ls_mears_vals, k + 1)));
				*(data + data_m_sz + sizeof(double)) = 0;
				break;
			}
			data_m_sz += sizeof(double) + sizeof(char);
		}
	}

	*((__uint32_t *)data) = data_m_sz; // set data package capacity
	char *_ec_data_ = mem_alloc_0(data_m_sz);
	memcpy(_ec_data_, data, data_m_sz);
	release_mem(data);

	EuclidCommand *_ec_ = create_command(_ec_data_);

	// Store measure values locally or distribute it to downstream nodes for processing
	return distribute_store_measure(_ec_);
}

Cube *find_cube_by_name(char *cube_name)
{
	__uint32_t i, sz = als_size(cubes_pool);
	for (i = 0; i < sz; i++)
	{
		Cube *cube = als_get(cubes_pool, i);
		if (strcmp(cube_name, cube->name) == 0)
			return cube;
	}
	return NULL;
}

Cube *find_cube_by_gid(md_gid id)
{
	__uint32_t i, sz = als_size(cubes_pool);
	for (i = 0; i < sz; i++)
	{
		Cube *cube = als_get(cubes_pool, i);
		if (cube->gid = id)
			return cube;
	}
	return NULL;
}

void *gen_member_gid_abs_path(Cube *cube, ArrayList *mbr_path_str)
{
	char *dim_role_name = als_get(mbr_path_str, 0);
	DimensionRole *dr;
	Dimension *dim;
	Member *lv1_mbr, *mbr;
	__uint32_t i, num_drs = als_size(cube->dim_role_ls);
	for (i = 0; i < num_drs; i++)
	{
		dr = als_get(cube->dim_role_ls, i);
		if (strcmp(dim_role_name, dr->name) != 0)
			continue;
		dim = find_dim_by_gid(dr->dim_gid);
		mbr = lv1_mbr = find_member_lv1(dim, (char *)als_get(mbr_path_str, 1));
		break;
	}

	// printf("dim_role_name [ %s ], dim->name [ %s ], lv1_mbr->name [ %s ]\n", dim_role_name, dim->name, lv1_mbr->name);

	__uint32_t sz = als_size(mbr_path_str);
	// printf("char *abs_path = mem_alloc_0(  %lu  );", sizeof(__uint32_t) + sizeof(md_gid) * (sz - 1));
	char *abs_path = mem_alloc_0(sizeof(__uint32_t) + sizeof(md_gid) * (sz - 1));
	*((__uint32_t *)abs_path) = sz - 1;
	*((md_gid *)(abs_path + sizeof(__uint32_t))) = lv1_mbr->gid;

	for (i = 2; i < sz; i++)
	{
		mbr = find_member_child(mbr, als_get(mbr_path_str, i));
		*((md_gid *)(abs_path + sizeof(__uint32_t) + sizeof(md_gid) * (i - 1))) = mbr->gid;
		// printf("::>> child member name - %s\n", mbr->name);
	}

	return abs_path;
}

void *exe_multi_dim_queries(SelectDef *select_def)
{
	// Build the real axes in this multidimensional query.
	ArrayList *axes = select_def__build_axes(select_def);
	unsigned int x_size = als_size(axes);

	// Cross these axes to generate result set.
	unsigned long rs_len = 1;
	// int i, ax_count = als_size(axes);
	int i;
	for (i = 0; i < x_size; i++)
	{
		MddAxis *ax = als_get(axes, i);
		rs_len *= mdd_ax__len(ax);
	}

	// !!! >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	unsigned int offset_arr[x_size];
	offset_arr[x_size - 1] = 1;

	for (i = x_size - 2; i >= 0; i--)
		offset_arr[i] = offset_arr[i + 1] * mdd_ax__len(als_get(axes, i + 1));

	// MddTuple *tuples_matrix[rs_len][als_size(axes)];
	// MddTuple **tuples_matrix_h = mem_alloc_0(rs_len * x_size);
	MddTuple **tuples_matrix_h = mem_alloc_0(rs_len * x_size * sizeof(void *));

	int matx_col, matx_row, f;
	for (matx_col = 0; matx_col < x_size; matx_col++)
	{
		matx_row = 0;
		MddAxis *ax = als_get(axes, matx_col);
		while (matx_row < rs_len)
		{
			for (i = 0; i < mdd_ax__len(ax); i++)
			{
				MddTuple *tuple = mdd_ax__get_tuple(ax, i);
				for (f = 0; f < offset_arr[matx_col]; f++)
					// tuples_matrix_h[matx_col * rs_len + matx_row++] = tuple;
					tuples_matrix_h[(matx_row++) * x_size + matx_col] = tuple;
			}
		}
	}

	Cube *cube = select_def__get_cube(select_def);
	MddTuple *basic_tuple = cube__basic_ref_vector(cube);

	for (i = 0; i < rs_len; i++)
	{
		tuples_matrix_h[i] = _MddTuple__mergeTuples(tuples_matrix_h + (i * x_size), x_size);
		tuples_matrix_h[i] = tuple__merge(basic_tuple, tuples_matrix_h[i]);
	}

	// 'md_result' is equivalent to a double array whose length is 'rs_len'.
	double *md_result = vce_vactors_values(tuples_matrix_h, rs_len);

	return md_result;
}

static ArrayList *select_def__build_axes(SelectDef *select_def)
{
	ArrayList *ax_def_ls = select_def->ax_def_ls;
	// // printf("[debug] >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ArrayList *ax_def_ls desc \"%s\"\n", ax_def_ls->desc);
	Cube *cube = select_def__get_cube(select_def);
	MddTuple *ref_tuple = cube__basic_ref_vector(cube);
	int ax_count = als_size(ax_def_ls);
	int i, j;
	for (i = 0; i < ax_count; i++)
	{
		for (j = 0; j < ax_count; j++)
		{
			AxisDef *ax_def = als_get(ax_def_ls, j);
			MddTuple *ax_head_ref_tuple = ax_def__head_ref_tuple(ax_def, ref_tuple, cube);

			// // printf("[debug] <><><><><><><><><><><><><>      ax_head_ref_tuple [ %p ] ax_head_ref_tuple->mr_ls [ %p ]\n", ax_head_ref_tuple, ax_head_ref_tuple->mr_ls);

			ref_tuple = tuple__merge(ref_tuple, ax_head_ref_tuple);
		}
	}
	ArrayList *axes_ls = als_create(16, "MddAxis *");
	for (i = 0; i < ax_count; i++)
	{
		AxisDef *ax_def = als_get(ax_def_ls, i);
		MddAxis *ax = ax_def__build(ax_def, ref_tuple, cube);
		als_add(axes_ls, ax);
	}

	return axes_ls;
}

static Cube *select_def__get_cube(SelectDef *select_def)
{
	int i;
	int cubes_count = als_size(cubes_pool);
	Cube *cube;
	for (i = 0; i < cubes_count; i++)
	{
		cube = als_get(cubes_pool, i);
		if (strcmp(cube->name, select_def->cube_def->name) == 0)
			return cube;
	}
	return NULL;
}

static MddTuple *cube__basic_ref_vector(Cube *cube)
{
	// // printf("[debug] >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	MddTuple *tuple = mdd_tp__create();
	int i, j;
	int r_count = als_size(cube->dim_role_ls);
	for (i = 0; i < r_count; i++)
	{
		DimensionRole *dim_role = als_get(cube->dim_role_ls, i);
		// // printf("[debug] >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  dim_role->dim_gid = %lu\n", dim_role->dim_gid);
		int mp_size = als_size(member_pool);
		for (j = 0; j < mp_size; j++)
		{
			Member *mbr = als_get(member_pool, j);
			if (mbr->dim_gid == dim_role->dim_gid && mdd_mbr__is_leaf(mbr))
			{
				MddMemberRole *mbr_role = mdd_mr__create(mbr, dim_role);
				mdd_tp__add_mbrole(tuple, mbr_role);
				break;
			}

			// // // printf("[debug] >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  mbr->name = %s    %d\n", mbr->name, mdd_mbr__is_leaf(mbr));
			// else if (mbr->dim_gid == cube->measure_dim->gid)
			// 	// // printf("[debug] >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  measure member = %s\n", mbr->name);
		}
	}
	// // printf("[debug] >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  cube->measure_dim->gid = %lu\n", cube->measure_dim->gid);
	// for (i = 0; i < als_size(cube->measure_mbrs); i++)
	// {
	// 	Member *mbr = als_get(cube->measure_mbrs, i);
	// 	// // printf("[debug] >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  measure member = %s    %d\n", mbr->name, mdd_mbr__is_leaf(mbr));
	// }
	MddMemberRole *measure_mr = mdd_mr__create(als_get(cube->measure_mbrs, 0), NULL);
	mdd_tp__add_mbrole(tuple, measure_mr);
	// // printf("[debug] >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	return tuple;
}

static MddTuple *ax_def__head_ref_tuple(AxisDef *ax_def, MddTuple *t, Cube *c)
{
	return ids_setdef__head_ref_tuple(ax_def->set_def, t, c);
}

static MddTuple *tuple__merge(MddTuple *ctx_tuple, MddTuple *tuple_frag)
{
	unsigned int ctx_sz = als_size(ctx_tuple->mr_ls);
	unsigned int frag_sz = als_size(tuple_frag->mr_ls);

	MddTuple *tp = mdd_tp__create();

	int i, j;

	for (i = 0; i < ctx_sz; i++)
	{
		MddMemberRole *ctx_mr = (MddMemberRole *)als_get(ctx_tuple->mr_ls, i);
		for (j = 0; j < frag_sz; j++)
		{
			MddMemberRole *f_mr = (MddMemberRole *)als_get(tuple_frag->mr_ls, j);

			// // // printf("[debug] *******************************************************************************\n");
			// // // // printf("[debug] ******************* ctx_mr = %p \n", ctx_mr);
			// // // printf("[debug] ******************* ctx_mr->dim_role = %p \n", ctx_mr->dim_role);
			// // // // printf("[debug] ******************* ctx_mr->dim_role->gid = %lu \n", ctx_mr->dim_role->gid);
			// // // // printf("[debug] ******************* f_mr = %p \n", f_mr);
			// // // printf("[debug] ******************* f_mr->dim_role = %p \n", f_mr->dim_role);
			// // // // printf("[debug] ******************* f_mr->dim_role->gid = %lu \n", f_mr->dim_role->gid);
			// // // printf("[debug] *******************************************************************************\n");

			if ((ctx_mr->dim_role != NULL && f_mr->dim_role != NULL) && (ctx_mr->dim_role->gid == f_mr->dim_role->gid))
			{
				mdd_tp__add_mbrole(tp, f_mr);
				goto jump_a;
			}

			if (ctx_mr->dim_role == NULL && f_mr->dim_role == NULL)
			{
				mdd_tp__add_mbrole(tp, f_mr);
				goto jump_a;
			}
		}
		mdd_tp__add_mbrole(tp, ctx_mr);
	jump_a:
		i = i;
	}

	for (j = 0; j < frag_sz; j++)
	{
		MddMemberRole *f_mr = (MddMemberRole *)als_get(tuple_frag->mr_ls, j);
		for (i = 0; i < ctx_sz; i++)
		{
			MddMemberRole *ctx_mr = (MddMemberRole *)als_get(ctx_tuple->mr_ls, i);

			if ((ctx_mr->dim_role != NULL && f_mr->dim_role != NULL) && (ctx_mr->dim_role->gid == f_mr->dim_role->gid))
			{
				goto jump_b;
			}

			if (ctx_mr->dim_role == NULL && f_mr->dim_role == NULL)
			{
				goto jump_b;
			}
		}
		mdd_tp__add_mbrole(tp, f_mr);
	jump_b:
		j = j;
	}

	return tp;
}

static MddAxis *ax_def__build(AxisDef *ax_def, MddTuple *ctx_tuple, Cube *cube)
{
	MddSet *_set = ids_setdef__build(ax_def->set_def, ctx_tuple, cube);
	MddAxis *ax = mdd_ax__create();
	ax->set = _set;
	ax->posi = ax_def->posi;
	return ax;
}

static unsigned int mdd_ax__len(MddAxis *ax)
{
	return mdd_set__len(ax->set);
}

static unsigned int mdd_set__len(MddSet *set)
{
	return als_size(set->tuples);
}

int mdd_mbr__is_leaf(Member *m)
{
	return (m->bin_attr & MDD_MEMBER__BIN_ATTR_FLAG__NON_LEAF) == 0 ? 1 : 0;
}

void mdd_mbr__set_as_leaf(Member *m)
{
	m->bin_attr = m->bin_attr | MDD_MEMBER__BIN_ATTR_FLAG__NON_LEAF;
}

MddTuple *mdd_tp__create()
{
	MddTuple *tp = (MddTuple *)mem_alloc_0(sizeof(MddTuple));
	tp->mr_ls = als_create(32, "MddMemberRole *");
	// printf("[debug] 创建 MddTuple >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> tp->mr_ls->idx = %u\n", tp->mr_ls->idx);
	return tp;
}

MddMemberRole *mdd_mr__create(Member *m, DimensionRole *dr)
{
	MddMemberRole *mr = mem_alloc_0(sizeof(MddMemberRole));
	mr->member = m;
	mr->dim_role = dr;
	return mr;
}

MddTuple *ids_setdef__head_ref_tuple(SetDef *set_def, MddTuple *context_tuple, Cube *cube)
{
	// printf("************************************************************  [set_def->tuple_def_ls->desc]  %s\n", set_def->tuple_def_ls->desc);
	// printf("************************************************************  [als_size(set_def->tuple_def_ls)]  %d\n", als_size(set_def->tuple_def_ls));
	TupleDef *head_ref_tupdef = (TupleDef *)als_get(set_def->tuple_def_ls, 0);
	return ids_tupledef__build(head_ref_tupdef, context_tuple, cube);
}

static MddTuple *ids_tupledef__build(TupleDef *t_def, MddTuple *context_tuple, Cube *cube)
{
	MddTuple *t = (MddTuple *)mdd_tp__create();
	int i, len = als_size(t_def->ms_def->mbr_def_ls);
	for (i = 0; i < len; i++)
	{
		MemberDef *m_def = als_get(t_def->ms_def->mbr_def_ls, i);
		MddMemberRole *mr = ids_mbrsdef__build(m_def, context_tuple, cube);
		mdd_tp__add_mbrole(t, mr);
	}
	return t;
}

MddMemberRole *ids_mbrsdef__build(MemberDef *m_def, MddTuple *context_tuple, Cube *cube)
{
	if (m_def->t_cons == MEMBER_DEF__MBR_ABS_PATH)
	{
		char *dim_role_name = als_get(m_def->mbr_abs_path, 0);
		DimensionRole *dr = cube__dim_role(cube, dim_role_name);
		Dimension *dim;

		int i, dims_pool_size = als_size(dims_pool);
		for (i = 0; i < dims_pool_size; i++)
		{
			dim = (Dimension *)als_get(dims_pool, i);
			if (dim->gid == dr->dim_gid)
				break;
		}

		ArrayList *mbr_path = als_create(32, "char *");
		int ap_len = als_size(m_def->mbr_abs_path);
		for (i = 1; i < ap_len; i++)
		{
			als_add(mbr_path, als_get(m_def->mbr_abs_path, i));
		}

		Member *mbr = dim__find_mbr(dim, mbr_path);

		MddMemberRole *mr = mdd_mr__create(mbr, dr);

		return mr;
	}
	else
	{
		printf("[error] Unknown type about defining dimension member.\n");
		exit(1);
	}
}

void mdd_tp__add_mbrole(MddTuple *t, MddMemberRole *mr)
{
	als_add(t->mr_ls, mr);
}

DimensionRole *cube__dim_role(Cube *cube, char *dim_role_name)
{
	int i, dr_ls_zs = als_size(cube->dim_role_ls);
	for (i = 0; i < dr_ls_zs; i++)
	{
		DimensionRole *dr = (DimensionRole *)als_get(cube->dim_role_ls, i);
		if (strcmp(dr->name, dim_role_name) == 0)
			return dr;
	}
	printf("[warn] no DimensionRole that name is [%s]\n", dim_role_name);
	return NULL;
}

Member *dim__find_mbr(Dimension *dim, ArrayList *mbr_name_path)
{
	int i, len = als_size(mbr_name_path);
	Member *m = find_member_lv1(dim, als_get(mbr_name_path, 0));
	for (i = 1; i < len; i++)
		m = find_member_child(m, als_get(mbr_name_path, i));

	return m;
}

MddSet *mdd_set__create()
{
	MddSet *set = mem_alloc_0(sizeof(MddSet));
	set->tuples = als_create(64, "MddTuple *");
	return set;
}

MddAxis *mdd_ax__create()
{
	return (MddAxis *)mem_alloc_0(sizeof(MddAxis));
}

MddSet *ids_setdef__build(SetDef *set_def, MddTuple *ctx_tuple, Cube *cube)
{
	MddSet *set = mdd_set__create();
	if (set_def->t_cons == SET_DEF__TUP_DEF_LS)
	{
		int i, sz = als_size(set_def->tuple_def_ls);
		for (i = 0; i < sz; i++)
		{
			TupleDef *tp_def = als_get(set_def->tuple_def_ls, i);
			MddTuple *tp = ids_tupledef__build(tp_def, ctx_tuple, cube);
			mddset__add_tuple(set, tp);
		}
		return set;
	}
	else
	{
		printf("[warn] wrong SetDef::t_cons\n");
		return NULL;
	}
}

void mddset__add_tuple(MddSet *s, MddTuple *t)
{
	als_add(s->tuples, t);
}

MddTuple *mdd_ax__get_tuple(MddAxis *ax, int idx)
{
	return als_get(ax->set->tuples, idx);
}

MddTuple *_MddTuple__mergeTuples(MddTuple **tps, int count)
{
	if (count < 2)
		return tps[0];
	MddTuple *tuple = tuple__merge(tps[0], tps[1]);
	int i;
	for (i = 2; i < count; i++)
		tuple = tuple__merge(tuple, tps[i]);
	return tuple;
}

void Cube_print(Cube *c)
{
	printf(">>> [ Cube info ] @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ addr < %p >\n", c);
	printf("\t     name - %s\n", c->name);
	printf("\t      gid - %lu\n", c->gid);
}

void Tuple_print(MddTuple *tuple)
{
	printf(">>> [ Tuple info ] @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ addr < %p >\n", tuple);
	unsigned int len = als_size(tuple->mr_ls);
	printf("als_size(tuple->mr_ls) = < %u >\n", len);
	printf("   tuple->mr_ls->desc is < %s >\n", tuple->mr_ls->desc);
	unsigned int i;
	for (i = 0; i < len; i++)
	{
		MemberRole_print(als_get(tuple->mr_ls, i));
	}
}

void MemberRole_print(MddMemberRole *mr)
{
	printf(">>> [ MddMemberRole info ] >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> addr < %p >\n", mr);
	Member_print(mr->member);
	DimensionRole_print(mr->dim_role);
}

void Member_print(Member *m)
{
	printf(">>> [ Member info ] >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> addr < %p >\n", m);
	printf("\t     name - %s\n", m->name);
	printf("\t      gid - %lu\n", m->gid);
	printf("\t    p_gid - %lu\n", m->p_gid);
	printf("\t  dim_gid - %lu\n", m->dim_gid);
	printf("\t       lv - %u\n", m->lv);
	printf("\t bin_attr - %d\n", m->bin_attr);
	printf("\t abs_path - %p\n", m->abs_path);
	/*
typedef struct _stct_mbr_
{
	char name[MD_ENTITY_NAME_BYTSZ];
	md_gid gid;
	md_gid p_gid;
	md_gid dim_gid;
	unsigned short lv;

	// Each binary bit represents an attribute switch.
	// lowest bit, 0 - leaf member, 1 - non-leaf member.
	int bin_attr;
} Member;
	*/
}

void DimensionRole_print(DimensionRole *dr)
{
	printf(">>> [ DimensionRole info ] >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> addr < %p >\n", dr);
}

void mdd__gen_mbr_abs_path(Member *m)
{
	if (m->abs_path)
		return;

	m->abs_path = mem_alloc_0(m->lv * sizeof(md_gid));

	Member *current_m = m;

	int i;
	for (i = m->lv - 1; i >= 0; i--)
	{
		m->abs_path[i] = current_m->gid;
		if (current_m->p_gid)
			current_m = find_member_by_gid(current_m->p_gid);
	}
}

Member *find_member_by_gid(md_gid m_gid)
{
	int i;
	for (i = 0; i < als_size(member_pool); i++)
	{
		Member *m = als_get(member_pool, i);
		if (m->gid == m_gid)
			return m;
	}
	return NULL;
}