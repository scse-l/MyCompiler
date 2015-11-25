/*
	分程序语法分析:<分程序>::=[<常量说明部分>][<变量说明部分>]{[<过程说明部分>]|[<函数说明部分>]}<复合语句>
*/

#include "declaration.h"
#include "statement.h"
#include "global.h"
#include "support.h"

extern int symbol;

AST_node program(AST_node parent)
{
	AST_node t = makeNode(PROGRAM, parent);
	AST_node temp = NULL;

	constdecl(t);
	vardecl(t);
	prodecl(t);
	fundecl(t);
	
	if (symbol != BEGIN)
	{
		error("No begin found at the begin of program");
		recovery(8, IF, FOR, DO, IDENT, SEMICOLON, READ, WRITE, BEGIN);
		stat_list(t);
	}
	else
	{
		stat_list(t);
	}
	return t;
}