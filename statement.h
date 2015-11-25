#pragma once
/*
	语句语法分析函数
*/

AST_node statement(AST_node);				//语句语法分析函数
AST_node stat_assign(AST_node);				//赋值语句
AST_node stat_list(AST_node);				//复合语句
AST_node stat_for(AST_node);					//for语句
AST_node stat_do(AST_node);					//当循环语句
AST_node stat_if(AST_node);					//条件语句
AST_node condition(AST_node);				//条件
AST_node stat_procedure(AST_node);			//过程调用语句
AST_node arg_list(AST_node);					//实在参数表
AST_node stat_write(AST_node);				//写语句
AST_node stat_read(AST_node);				//读语句
