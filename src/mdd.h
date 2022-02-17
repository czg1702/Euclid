#ifndef EUCLID__MDD_H
#define EUCLID__MDD_H 1

#include "utils.h"
#include "command.h"

#define MD_ENTITY_NAME_BYTSZ 62

#define META_DEF_DIMS_FILE_PATH "/meta/dims"
#define META_DEF_MBRS_FILE_PATH "/meta/mbrs"

#define MEASURE_DIM_COMM_NAME "measure"

typedef long md_gid;

typedef struct _stct_dim_
{
	md_gid gid;
	char name[MD_ENTITY_NAME_BYTSZ];
} Dimension;

typedef struct _stct_mbr_
{
	char name[MD_ENTITY_NAME_BYTSZ];
	unsigned short lv;
	md_gid gid;
	md_gid p_gid;
	md_gid dim_gid;
} Member;

int create_dims(ArrayList *dim_names);

md_gid gen_md_gid();

int create_members(ArrayList *mbrs_info_als);

Dimension *find_dim_by_name(char *dim_name);

Dimension *find_dim_by_gid(md_gid dim_gid);

Member *find_member_lv1(Dimension *dim, char *mbr_name);

Member *find_member_child(Member *parent_mbr, char *child_name);

typedef struct _euclid_cube_stct_
{
	md_gid gid;
	char name[MD_ENTITY_NAME_BYTSZ];
	ArrayList *dim_role_ls;
	Dimension *measure_dim;
	ArrayList *measure_mbrs;
} Cube;

typedef struct _dim_role_stct_
{
	md_gid gid;
	char name[MD_ENTITY_NAME_BYTSZ];
	md_gid cube_gid;
	md_gid dim_gid;
} DimensionRole;

int build_cube(char *name, ArrayList *dim_role_ls, ArrayList *measures);

int insert_cube_measure_vals(char *cube_name, ArrayList *ls_ids_vctr_mear);

Cube *find_cube_by_name(char *cube_name);

void *gen_member_gid_abs_path(Cube *cube, ArrayList *mbr_path_str);

int store_measure(EuclidCommand *ec);

int distribute_store_measure(EuclidCommand *ec);

#endif