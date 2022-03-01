//#include <arpa/inet.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

//#include "net.h"
//#include "cfg.h"
//#include "utils.h"
//#include "command.h"

#include "mdx.h"

MemberDef *ids_mbrdef_new__mbr_abs_path(ArrayList *mbr_abs_path)
{
    MemberDef *def = (MemberDef *)mem_alloc_0(sizeof(MemberDef));
    def->t_cons = MEMBER_DEF__MBR_ABS_PATH;
    def->mbr_abs_path = mbr_abs_path;
    return def;
}

MembersDef *ids_mbrsdef_new(ids_ct t_cons)
{
    MembersDef *def = (MembersDef *)mem_alloc_0(sizeof(MembersDef));
    def->t_cons = t_cons;
    def->mbr_def_ls = als_create(32, "MembersDef *");
    return def;
}

void ids_mbrsdef__add_mbr_def(MembersDef *ms, MemberDef *m)
{
    als_add(ms->mbr_def_ls, m);
}

TupleDef *ids_tupledef_new(ids_ct t_cons)
{
    TupleDef *def = (TupleDef *)mem_alloc_0(sizeof(TupleDef));
    def->t_cons = t_cons;
    return def;
}

void ids_tupledef___set_mbrs_def(TupleDef *t, MembersDef *ms)
{
    t->ms_def = ms;
}

SetDef *ids_setdef_new(ids_ct t_cons)
{
    SetDef *def = (SetDef *)mem_alloc_0(sizeof(SetDef));
    def->t_cons = t_cons;
    return def;
}

void ids_setdef__set_tuple_def_ls(SetDef *sd, ArrayList *ls)
{
    sd->tuple_def_ls = ls;
}

AxisDef *ids_axisdef_new(SetDef *set_def, unsigned short posi)
{
    AxisDef *def = (AxisDef *)mem_alloc_0(sizeof(AxisDef));
    def->posi = posi;
    def->set_def = set_def;
    return def;
}

CubeDef *ids_cubedef_new(char *name)
{
    CubeDef *def = (CubeDef *)mem_alloc_0(sizeof(CubeDef));
    def->name = name;
    return def;
}

SelectDef *ids_selectdef_new(CubeDef *cube_def, ArrayList *ax_def_ls)
{
    SelectDef *def = (SelectDef *)mem_alloc_0(sizeof(SelectDef));
    def->cube_def = cube_def;
    def->ax_def_ls = ax_def_ls;
    return def;
}