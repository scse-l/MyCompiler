/*
	分程序语法分析:<分程序>::=[<常量说明部分>][<变量说明部分>]{[<过程说明部分>]|[<函数说明部分>]}<复合语句>
*/
#include "declaration.h"
#include "statement.h"

int program()
{
	constdecl();
	vardecl();
	prodecl();
	fundecl();
	stat_list();
	return 0;
}