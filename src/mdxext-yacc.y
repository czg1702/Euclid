%{
// yacc_f_001
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "mdx.h"

int yyerror(const char *);
extern int yylex();
extern int yyparse();

extern char *yytext;

Stack YC_STC = { 0 };

// yacc_f_002
%}

/* key words */
%token CREATE		/* create */
%token DIMENSIONS	/* dimensions */
%token MEMBERS		/* members */
%token BUILD		/* build */
%token CUBE			/* cube */
%token MEASURES		/* measures */
%token INSERT		/* insert */

/* punctuations */
%token COMMA				/* , */
%token DOT					/* . */

%token ROUND_BRACKET_L		/* ( */
%token ROUND_BRACKET_R		/* ) */

%token VAR
%token BLOCK

%token DECIMAL

%%

statement:
	create_dimensions {
		stack_push(&YC_STC, IDS_STRLS_CRTDIMS);
	}
  |	create_members {
		stack_push(&YC_STC, IDS_STRLS_CRTMBRS);
	}
  |	build_cube {
		stack_push(&YC_STC, IDS_OBJLS_BIUCUBE);
	}
  | insert_cube_measures {
	  	stack_push(&YC_STC, IDS_CXOBJ_ISRTCUBEMEARS);
		printf("stack_push(&YC_STC, IDS_CXOBJ_ISRTCUBEMEARS);\n");
  }
;

insert_cube_measures:
	INSERT var_or_block vector_measures {
		ArrayList *ls_vms = als_create(128, "{ insert_cube_measures ::= }, { IDSVectorMears * }");
		IDSVectorMears *ids_vm;
		stack_pop(&YC_STC, (void **) &ids_vm);
		als_add(ls_vms, ids_vm);
		stack_push(&YC_STC, ls_vms);
	}
  | insert_cube_measures COMMA vector_measures {
		IDSVectorMears *ids_vm;
		stack_pop(&YC_STC, (void **) &ids_vm);
		ArrayList *ls_vms;
		stack_pop(&YC_STC, (void **) &ls_vms);
		als_add(ls_vms, ids_vm);
		stack_push(&YC_STC, ls_vms);
	}
;

vector_measures:
	ROUND_BRACKET_L vector MEASURES measures_values ROUND_BRACKET_R {
		ArrayList *ls_vector, *ls_mears_vals;
		stack_pop(&YC_STC, (void **) &ls_mears_vals);
		stack_pop(&YC_STC, (void **) &ls_vector);
		IDSVectorMears *ids_vm = mem_alloc_0(sizeof(IDSVectorMears));
		ids_vm->ls_vector = ls_vector;
		ids_vm->ls_mears_vals = ls_mears_vals;
		stack_push(&YC_STC, ids_vm);
	}
;

vector:
	mdm_entity_path {
		ArrayList *ls_vector = als_create(16, "{ vector ::= }, { ArrayList * }");
		ArrayList *ls_mep;
		stack_pop(&YC_STC, (void **) &ls_mep);
		als_add(ls_vector, ls_mep);
		stack_push(&YC_STC, ls_vector);
	}
  | vector COMMA mdm_entity_path {
		ArrayList *ls_mep, *ls_vector;
		stack_pop(&YC_STC, (void **) &ls_mep);
		stack_pop(&YC_STC, (void **) &ls_vector);
		als_add(ls_vector, ls_mep);
		stack_push(&YC_STC, ls_vector);
	}
;

measures_values:
	var_or_block DECIMAL {
		char *mmbr_name;
		stack_pop(&YC_STC, (void **) &mmbr_name);
		double *val = mem_alloc_0(sizeof(double));
		*val = atof(yytext);
		ArrayList *mear_vals_ls = als_create(16, "{ yacc measures_values ::= }, { 0,2,4 ... char * }, { 1,3,5 ... double * }");
		als_add(mear_vals_ls, mmbr_name);
		als_add(mear_vals_ls, val);
		stack_push(&YC_STC, mear_vals_ls);
	}
  | measures_values var_or_block DECIMAL {
		char *mmbr_name;
		stack_pop(&YC_STC, (void **) &mmbr_name);
		ArrayList *mear_vals_ls;
		stack_pop(&YC_STC, (void **) &mear_vals_ls);
		double *val = mem_alloc_0(sizeof(double));
		*val = atof(yytext);
		als_add(mear_vals_ls, mmbr_name);
		als_add(mear_vals_ls, val);
		stack_push(&YC_STC, mear_vals_ls);
	}
;

mdm_entity_path:
	var_or_block {
		ArrayList *path_ls = als_create(12, "yacc mdm_entity_path ::= , type of elements is char *");
		char *str;
		stack_pop(&YC_STC, (void **) &str);
		als_add(path_ls, str);
		stack_push(&YC_STC, path_ls);
	}
  | mdm_entity_path DOT var_or_block {
		char *str;
		stack_pop(&YC_STC, (void **) &str);
		ArrayList *path_ls;
		stack_pop(&YC_STC, (void **) &path_ls);
		als_add(path_ls, str);
		stack_push(&YC_STC, path_ls);
	}
;

create_dimensions:
	CREATE DIMENSIONS vars {
		// no need to do anything.
	}
;

create_members:
	CREATE MEMBERS member_absolute_path {
		ArrayList *mbr_path_ls;
		stack_pop(&YC_STC, (void **) &mbr_path_ls);
		ArrayList *mbrs_ls = als_create(128, "ele type: ArrayList *, yacc create_members");
		als_add(mbrs_ls, mbr_path_ls);
		stack_push(&YC_STC, mbrs_ls);
//printf("STACK - pop  : member_absolute_path\n");
//printf("STACK - push : create_members\n");
	}
  |	create_members COMMA member_absolute_path {
		ArrayList *mbr_path_ls;
		stack_pop(&YC_STC, (void **) &mbr_path_ls);
		ArrayList *mbrs_ls;
		stack_pop(&YC_STC, (void **) &mbrs_ls);
		als_add(mbrs_ls, mbr_path_ls);
		stack_push(&YC_STC, mbrs_ls);
//printf("STACK - pop  : member_absolute_path\n");
//printf("STACK - pop  : create_members\n");
//printf("STACK - push : create_members\n");
	}
;

member_absolute_path:
	var_or_block DOT var_or_block {
		char *str_left, *str_right;
		stack_pop(&YC_STC, (void **) &str_right);
		stack_pop(&YC_STC, (void **) &str_left);
		ArrayList *als = als_create(16, "ele type: char *, yacc member_absolute_path");
		als_add(als, str_left);
		als_add(als, str_right);
		stack_push(&YC_STC, als);
//printf("STACK - pop  : var_or_block\n");
//printf("STACK - pop  : var_or_block\n");
//printf("STACK - push : member_absolute_path\n");
	}
  |	member_absolute_path DOT var_or_block {
		char *str;
		stack_pop(&YC_STC, (void **) &str);
		ArrayList *als;
		stack_pop(&YC_STC, (void **) &als);
		als_add(als, str);
		stack_push(&YC_STC, als);
//printf("STACK - pop  : var_or_block\n");
//printf("STACK - pop  : member_absolute_path\n");
//printf("STACK - push : member_absolute_path\n");
	}
;

build_cube:
	BUILD CUBE var_or_block DIMENSIONS dims_and_roles MEASURES vars {
		// do nothing
	}
;

dims_and_roles:
	var_or_block var_or_block {
		char *dim_name, *role_name;
		stack_pop(&YC_STC, (void **) &role_name);
		stack_pop(&YC_STC, (void **) &dim_name);
		ArrayList *dr_ls = als_create(64, "yacc dims_and_roles ::= var_or_block var_or_block");
		als_add(dr_ls, dim_name);
		als_add(dr_ls, role_name);
		stack_push(&YC_STC, dr_ls);
	}
  |	dims_and_roles var_or_block var_or_block {
		char *dim_name, *role_name;
		stack_pop(&YC_STC, (void **) &role_name);
		stack_pop(&YC_STC, (void **) &dim_name);
		ArrayList *dr_ls;
		stack_pop(&YC_STC, (void **) &dr_ls);
		als_add(dr_ls, dim_name);
		als_add(dr_ls, role_name);
		stack_push(&YC_STC, dr_ls);
	}
;

vars:
	var_or_block	{
//printf("STACK - pop  : var_or_block\n");
		char *vb_str;
		stack_pop(&YC_STC, (void **) &vb_str);
		ArrayList *vb_ls = als_create(8, "yacc vars ::=");
		als_add(vb_ls, vb_str);
//printf("STACK - push : vars\n");
		stack_push(&YC_STC, vb_ls);
	}
  |	vars var_or_block	{
//printf("STACK - pop  : var_or_block\n");
		char *vb_str;
		stack_pop(&YC_STC, (void **) &vb_str);
//printf("STACK - pop  : vars\n");
		ArrayList *vb_ls;
		stack_pop(&YC_STC, (void **) &vb_ls);
		als_add(vb_ls, vb_str);
//printf("STACK - push : vars\n");
		stack_push(&YC_STC, vb_ls);
	}
;

var_or_block:
	VAR	{
		stack_push(&YC_STC, str_clone(yytext));
//printf("TOKEN - val  : VAR <%s>\n", yytext);
//printf("STACK - push : var_or_block\n");
	}
  |	BLOCK	{
	  	char *str = str_clone(yytext);
		int i, len = strlen(str);
		for (i = 1; i < len - 1; i++) {
			str[i - 1] = str[i];
		}
		str[len - 2] = '\0';
		stack_push(&YC_STC, str);
//printf("TOKEN - val  : BLOCK <%s>\n", yytext);
//printf("STACK - push : var_or_block\n");
	}

%%
// yacc_f_003

void *parse_mdx(char *mdx)
{
	my_scan_string(mdx);
	yyparse();
	my_cleanup();

	return NULL;
}

int yyerror(const char *s)
{
	//printf("[yy error] <%s>\n", s);
    return -100;
}
// yacc_f_004
