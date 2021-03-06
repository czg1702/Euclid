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
#define MEMBER_DEF__MBR_FUNCTION 2

typedef struct member_definition
{
    ids_ct t_cons;
    ArrayList *mbr_abs_path;
    void *member_fn;
} MemberDef;

MemberDef *ids_mbrdef_new__mbr_abs_path(ArrayList *);
MemberDef *MemberDef_creat(ids_ct t_cons);

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


#define SET_DEF__TUP_DEF_LS     1
#define SET_DEF__SET_FUNCTION   2
#define SET_DEF__VAR_OR_BLOCK   3

typedef struct set_definition
{
    ids_ct t_cons;
    ArrayList *tuple_def_ls;
    void *set_fn;
    char *var_block;
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
    ArrayList *member_formulas;
    ArrayList *set_formulas;
    CubeDef *cube_def;
    ArrayList *ax_def_ls;
    TupleDef *where_tuple_def;
} SelectDef;

SelectDef *ids_selectdef_new(CubeDef *, ArrayList *);


typedef struct _MD_context_
{
    SelectDef *select_def;
} MDContext;
MDContext *MDContext_creat();


#define FACTORY_DEF__TUP_DEF    1
#define FACTORY_DEF__DECIMAL    2
#define FACTORY_DEF__EXPRESSION 3
typedef struct factory_definition
{
    ids_ct t_cons;
    TupleDef *tuple_def;
    double decimal;
    void *exp;
} Factory;
Factory *Factory_creat();


typedef struct term_definition
{
    ArrayList *mul_factories;
    ArrayList *div_factories;
} Term;
Term *Term_creat();
void Term_mul_factory(Term *t, Factory *f);
void Term_div_factory(Term *t, Factory *f);


typedef struct term_expression
{
    ArrayList *plus_terms;
    ArrayList *minus_terms;
} Expression;
Expression *Expression_creat();
void Expression_plus_term(Expression *e, Term *t);
void Expression_minus_term(Expression *e, Term *t);


typedef struct member_formula
{
    ArrayList *path;
    Expression *exp;
} MemberFormula;
MemberFormula *MemberFormula_creat();

typedef struct set_formula
{
    char *var_block;
    SetDef *set_def;
} SetFormula;
SetFormula *SetFormula_creat();

typedef struct formula_context
{
    ArrayList *member_formulas;
    ArrayList *set_formulas;
} FormulaContext;
FormulaContext *FormulaContext_creat();

typedef struct set_fn_children
{
    MemberDef *m_def;
} SetFnChildren;
SetFnChildren *SetFnChildren_creat(MemberDef *m_def);

typedef struct mbr_fn_parent
{
    MemberDef *child_def;
} MemberFnParent;
MemberFnParent *MemberFnParent_creat(MemberDef *child_def);

#endif
