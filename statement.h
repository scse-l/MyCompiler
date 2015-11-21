#pragma once
/*
	语句语法分析函数
*/

int statement();				//语句语法分析函数
int stat_assign();				//赋值语句
int stat_list();				//复合语句
int stat_for();					//for语句
int stat_do();					//当循环语句
int stat_if();					//条件语句
int condition();				//条件
int stat_procedure();			//过程调用语句
int stat_write();				//写语句
int stat_read();				//读语句
