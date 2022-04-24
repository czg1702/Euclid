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
static ArrayList *mbrs_pool;
static ArrayList *cubes_pool;
static Member *_create_member_lv1(Dimension *dim, char *mbr_name);
static Member *_create_member_child(Member *parent, char *child_name);

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
	// printf(">>> create_member >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	unsigned int sz = als_size(mbr_path);
	int i = 0;
	// while (i < sz)
	// {
	// 	printf("%s\n", als_get(mbr_path, i));
	// 	i++;
	// }

	Dimension *dim = find_dim_by_name(als_get(mbr_path, 0));
	if (dim == NULL)
		dim = create_dimension(als_get(mbr_path, 0));

	Member *m, *mbr_lv1 = find_member_lv1(dim, als_get(mbr_path, 1));
	if (mbr_lv1 == NULL)
	{
		mbr_lv1 = _create_member_lv1(dim, als_get(mbr_path, 1));
		append_file_data(META_DEF_MBRS_FILE_PATH, (char *)mbr_lv1, sizeof(Member));

		if (mbrs_pool == NULL)
			mbrs_pool = als_create(256, "members pool");

		als_add(mbrs_pool, mbr_lv1);
		m = mbr_lv1;
	}

	Member *p_m = mbr_lv1;
	for (i = 2; i < sz; i++)
	{
		m = find_member_child(p_m, als_get(mbr_path, i));
		if (m == NULL)
		{
			m = _create_member_child(p_m, als_get(mbr_path, i));
			append_file_data(META_DEF_MBRS_FILE_PATH, (char *)m, sizeof(Member));
			als_add(mbrs_pool, m);
		}
		p_m = m;
	}

	return m;
}

int create_members(ArrayList *mbrs_info_als)
{
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
	if (mbrs_pool == NULL)
		return NULL;

	__uint32_t i = 0, sz = als_size(mbrs_pool);
	while (i < sz)
	{
		Member *mbr = als_get(mbrs_pool, i++);
		if ((strcmp(mbr_name, mbr->name) == 0) && dim->gid == mbr->dim_gid)
			return mbr;
	}
	return NULL;
}

Member *find_member_child(Member *parent_mbr, char *child_name)
{
	__uint32_t i = 0, sz = als_size(mbrs_pool);
	while (i < sz)
	{
		Member *mbr = als_get(mbrs_pool, i++);
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
	// printf("******************************** mbr->name %s\n", mbr->name);
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

	// Create several dimensional role objects and associate them to the cube.
	size_t i, dr_sz = als_size(dim_role_ls);
	for (i = 0; i < dr_sz; i += 2)
	{
		char *dim_name = als_get(dim_role_ls, i);
		char *dim_role_name = als_get(dim_role_ls, i + 1);
		Dimension *dim = find_dim_by_name(dim_name);

		DimensionRole *d_role = mem_alloc_0(sizeof(DimensionRole));
		memcpy(d_role->name, dim_role_name, strlen(dim_role_name));
		d_role->gid = gen_md_gid();
		d_role->cube_gid = cube->gid;
		d_role->dim_gid = dim->gid;

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
	return NULL; // TODO should be return a multi-dim-result
}