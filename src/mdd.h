#ifndef EUCLID__MDD_H
#define EUCLID__MDD_H 1

#include "utils.h"
#include "command.h"
#include "mdx.h"

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

#define MDD_MEMBER__BIN_ATTR_FLAG__NON_LEAF 1

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

int mdd_mbr__is_leaf(Member *);

void mdd_mbr__set_as_leaf(Member *);

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

// TODO should be return a multi-dim-result
void *exe_multi_dim_queries(SelectDef *);

typedef struct mdd_tuple
{
	ArrayList *mr_ls;
} MddTuple;

MddTuple *mdd_tp__create();

typedef struct mdd_set
{
	ArrayList *tuples;
} MddSet;

typedef struct mdd_axis
{
	MddSet *set;
	unsigned short posi;
} MddAxis;

typedef struct mdd_mbr_role
{
	Member *member;
	DimensionRole *dim_role;
} MddMemberRole;

/**
 * When the DimensionRole parameter is empty, it indicates the measure member role.
 */
MddMemberRole *mdd_mr__create(Member *, DimensionRole *);

MddSet *mdd_set__create();

MddAxis *mdd_ax__create();

void mdd_tp__add_mbrole(MddTuple *, MddMemberRole *);

MddTuple *ids_setdef__head_ref_tuple(SetDef *set_def, MddTuple *context_tuple, Cube *cube);

MddMemberRole *ids_mbrsdef__build(MemberDef *m_def, MddTuple *context_tuple, Cube *cube);

DimensionRole *cube__dim_role(Cube *cube, char *dim_role_name);

Member *dim__find_mbr(Dimension *dim, ArrayList *mbr_name_path);

MddSet *ids_setdef__build(SetDef *set_def, MddTuple *ctx_tuple, Cube *cube);

void mddset__add_tuple(MddSet *, MddTuple *);

MddTuple *mdd_ax__get_tuple(MddAxis *, int);

MddTuple *_MddTuple__mergeTuples(MddTuple **tps, int count);

#endif