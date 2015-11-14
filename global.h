#include <iostream>
#include <string>
//当前测试方便，用map代替符号表
#include <map>

#ifndef GLOBAL_H
#define GLOBAL_H

//结构和枚举声明
typedef enum MyEnum
{
	NUM = 256, IDENT, PLUS, MINUS, TIMES, SLASH, BEGIN, NUL, ERR
} symEnum;

typedef struct table_t
{
	std::string name;
	int type;
	int val;
} table;

//函数声明
int getsym();
#endif