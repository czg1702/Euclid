SRC_FILES = cfg.c net.c utils.c command.c mdxext-lex.yy.c mdxext-yacc.tab.c mdx.c mdd.c vce.c rb-tree.c
OBJ_FILES = cfg.o net.o utils.o command.o mdxext-lex.yy.o mdxext-yacc.tab.o mdx.o mdd.o vce.o rb-tree.o

euclid : Euclid.c Euclid-cli.c $(SRC_FILES) Euclid.o Euclid-cli.o $(OBJ_FILES)
	cc  -lpthread      $(OBJ_FILES)  Euclid.o      -o euclid
	cc  -lpthread  -g  $(SRC_FILES)  Euclid.c      -o euclid.out
	cc  -lpthread      $(OBJ_FILES)  Euclid-cli.o  -o euclid-cli
	cc  -lpthread  -g  $(SRC_FILES)  Euclid-cli.c  -o euclid-cli.out

mdxext-lex.yy.c: mdxext-lex.l
	flex -o mdxext-lex.yy.c mdxext-lex.l

mdxext-yacc.tab.h mdxext-yacc.tab.c: mdxext-yacc.y
	bison -d mdxext-yacc.y

clean :
	rm -f *.o
	rm -f *.out
	rm -f euclid
	rm -f euclid-cli
	rm -f *yacc.tab.h *yacc.tab.c *lex.yy.c
