#ifndef EUCLID__MDX_H
#define EUCLID__MDX_H 1

#include "utils.h"

// IDS - Intermediate Data Structure
#define IDS_STRLS_CRTDIMS ((void *)0x00)
#define IDS_STRLS_CRTMBRS ((void *)0x01)
#define IDS_OBJLS_BIUCUBE ((void *)0x02)
#define IDS_CXOBJ_ISRTCUBEMEARS ((void *)0x03)
#define IDS_MULTI_DIM_SELECT_DEF ((void *)0x04)

typedef struct __vector_measures__
{
    ArrayList *ls_vector;
    ArrayList *ls_mears_vals;
} IDSVectorMears;

typedef unsigned char ids_ct;

#define MEMBER_DEF__MBR_ABS_PATH 1

typedef struct member_definition
{
    ids_ct t_cons;
    ArrayList *mbr_abs_path;
} MemberDef;

MemberDef *ids_mbrdef_new__mbr_abs_path(ArrayList *);

#define MBRS_DEF__MBR_DEF_LS 1

typedef struct members_definition
{
    ids_ct t_cons;
    ArrayList *mbr_def_ls;
} MembersDef;

MembersDef *ids_mbrsdef_new(ids_ct);

void ids_mbrsdef__add_mbr_def(MembersDef *, MemberDef *);

#define TUPLE_DEF__MBRS_DEF 1

typedef struct tuple_definition
{
    ids_ct t_cons;
    MembersDef *ms_def;
} TupleDef;

TupleDef *ids_tupledef_new(ids_ct);

void ids_tupledef___set_mbrs_def(TupleDef *, MembersDef *);

#define SET_DEF__TUP_DEF_LS 1

typedef struct set_definition
{
    ids_ct t_cons;
    ArrayList *tuple_def_ls;
} SetDef;

SetDef *ids_setdef_new(ids_ct);

void ids_setdef__set_tuple_def_ls(SetDef *, ArrayList *);

typedef struct axis_definition
{
    SetDef *set_def;
    unsigned short posi;
} AxisDef;

AxisDef *ids_axisdef_new(SetDef *, unsigned short);

typedef struct cube_definition
{
    char *name;
} CubeDef;

CubeDef *ids_cubedef_new(char *name);

typedef struct select_definition
{
    CubeDef *cube_def;
    ArrayList *ax_def_ls;
} SelectDef;

SelectDef *ids_selectdef_new(CubeDef *, ArrayList *);

#endif
