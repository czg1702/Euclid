//#include <arpa/inet.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

//#include "net.h"
//#include "cfg.h"
//#include "utils.h"
//#include "command.h"

#include "mdx.h"
#include "obj-type-def.h"

MemberDef *ids_mbrdef_new__mbr_abs_path(ArrayList *mbr_abs_path)
{
    MemberDef *def = (MemberDef *)mem_alloc_0(sizeof(MemberDef));
    def->t_cons = MEMBER_DEF__MBR_ABS_PATH;
    def->mbr_abs_path = mbr_abs_path;
    return def;
}

MemberDef *MemberDef_creat(ids_ct t_cons) {
    MemberDef *mdef = obj_alloc(sizeof(MemberDef), OBJ_TYPE__MemberDef);
    mdef->t_cons = t_cons;
    return mdef;
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

Factory *Factory_creat() {
    return (Factory *)mem_alloc_0(sizeof(Factory));
}

Term *Term_creat() {
    Term *t = (Term *)mem_alloc_0(sizeof(Term));
    t->mul_factories = als_create(32, "Factory *");
    t->div_factories = als_create(32, "Factory *");
    return t;
}

void Term_mul_factory(Term *t, Factory *f) {
    als_add(t->mul_factories, f);
}

void Term_div_factory(Term *t, Factory *f) {
    als_add(t->div_factories, f);
}

Expression *Expression_creat() {
    Expression *e = (Expression *)mem_alloc_0(sizeof(Expression));
    e->plus_terms = als_create(32, "Term *");
    e->minus_terms = als_create(32, "Term *");
    return e;
}

void Expression_plus_term(Expression *e, Term *t) {
    als_add(e->plus_terms, t);
}

void Expression_minus_term(Expression *e, Term *t) {
    als_add(e->minus_terms, t);
}

MemberFormula *MemberFormula_creat() {
    return (MemberFormula *)mem_alloc_0(sizeof(MemberFormula));
}

SetFormula *SetFormula_creat() {
    return obj_alloc(sizeof(SetFormula), OBJ_TYPE__SET_FORMULA);
}

FormulaContext *FormulaContext_creat() {
    FormulaContext *fc = (FormulaContext *)mem_alloc_0(sizeof(FormulaContext));
    fc->member_formulas = als_create(32, "MemberFormula *");
    fc->set_formulas = als_create(32, "SetFormula *");
    return fc;
}

MDContext *MDContext_creat() {
    return obj_alloc(sizeof(MDContext), OBJ_TYPE__MD_CONTEXT);
}

SetFnChildren *SetFnChildren_creat(MemberDef *m_def) {
    SetFnChildren *fn = obj_alloc(sizeof(SetFnChildren), OBJ_TYPE__SET_FN_CHILDREN);
    fn->m_def = m_def;
    return fn;
}

MemberFnParent *MemberFnParent_creat(MemberDef *child_def) {
    MemberFnParent *fn = obj_alloc(sizeof(MemberFnParent), OBJ_TYPE__MemberFnParent);
    fn->child_def = child_def;
    return fn;
}