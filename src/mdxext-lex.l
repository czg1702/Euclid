%{
// lex_f_001
#include "mdxext-yacc.tab.h"
extern int yyerror(const char *);
int yywrap(void);
// lex_f_002
%}

%%

[ \n\t]+	{
	// ignore white space
}

create	{
	//printf("-------> token yytext[%s] yyleng[%d]\n", yytext, yyleng);
	return CREATE;
}

dimensions {
	//printf("-------> token yytext[%s] yyleng[%d]\n", yytext, yyleng);
	return DIMENSIONS;
}

members {
	//printf("-------> token yytext[%s] yyleng[%d]\n", yytext, yyleng);
	return MEMBERS;
}

build	{
	//printf("-------> token yytext[%s] yyleng[%d]\n", yytext, yyleng);
	return BUILD;
}

cube	{
	//printf("-------> token yytext[%s] yyleng[%d]\n", yytext, yyleng);
	return CUBE;
}

measures	{
	//printf("-------> token yytext[%s] yyleng[%d]\n", yytext, yyleng);
	return MEASURES;
}

insert	{
	//printf("-------> token yytext[%s] yyleng[%d]\n", yytext, yyleng);
	return INSERT;
}

select	{
	return SELECT;
}

from	{
	return FROM;
}

on	{
	return ON;
}

[_a-zA-Z][_a-zA-Z0-9]*	{
	//printf("-------> token yytext[%s] yyleng[%d]\n", yytext, yyleng);
	return VAR;
}

[0-9]+(\x2E[0-9]+)? {
	// ASCII 2E is .
	return DECIMAL;
}

","	{
	//printf("-------> token yytext[%s] yyleng[%d]\n", yytext, yyleng);
	return COMMA;
}

"." {
	//printf("-------> token yytext[%s] yyleng[%d]\n", yytext, yyleng);
	return DOT;
}

"("	{
	return ROUND_BRACKET_L;
}

")"	{
	return ROUND_BRACKET_R;
}

"{" {
	return BRACE_L;
}

"}" {
	return BRACE_R;
}

\x5B[^\x5B\x5D]+\x5D	{
	// ASCII 5B is [
	// ASCII 5D is ]
	//printf("-------> token yytext[%s] yyleng[%d]\n", yytext, yyleng);
	return BLOCK;
}

.	{ 
	//printf("<Characters that do not meet the rules> yytext[%s] yyleng[%d]\n", yytext, yyleng);
}

%%
// lex_f_003

static YY_BUFFER_STATE my_string_buffer;

int my_scan_string(const char *s) {

    // insist on cleaning up any existing buffer before setting up a new one
	if (my_string_buffer != NULL)
		return -1;

    // Set up and switch to a buffer for scanning the contents of the
    // specified string.  A copy of the string will be made.
    my_string_buffer = yy_scan_string(s);
    return my_string_buffer == NULL ? -1 : 0;
}

void my_cleanup() {
    // No effect if my_string_buffer is NULL
    yy_delete_buffer(my_string_buffer);
    // ... but avoid trying to free the same buffer twice
    my_string_buffer = NULL;
}

int yywrap(void)
{
	return 1;
}
// lex_f_004
