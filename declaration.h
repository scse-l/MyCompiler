#pragma once
/*
	声明语句
*/
#include "global.h"

AST_node constdecl(AST_node perent);            //常量声明
AST_node constdef(AST_node parent);				//常量定义
AST_node vardecl(AST_node parent);									//变量声明
AST_node vardef(AST_node parent);									//变量定义
AST_node prodecl(AST_node parent);									//过程说明
AST_node prodef(AST_node parent);									//过程声明
AST_node prohead(AST_node parent);									//过程首部
AST_node arglist(AST_node parent);									//形式参数表
AST_node args(AST_node parent);										//形式参数段
AST_node fundecl(AST_node parent);									//函数说明
AST_node fundef(AST_node parent);									//函数声明
AST_node funhead(AST_node parent);									//函数首部
