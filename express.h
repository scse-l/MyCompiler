#pragma once
/*
	表达式语法分析
*/
#include "global.h"

AST_node express(AST_node);					//表达式
AST_node term(AST_node);					//项
AST_node factor(AST_node);					//因子
