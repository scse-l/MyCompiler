#include <iostream>
#include <string>
//当前测试方便，用map代替符号表
#include <map>

#ifndef GLOBAL_H
#define GLOBAL_H

//结构和枚举声明
enum MyEnum
{
	//类型说明
	//CH为字符类型，与CHAR保留字区分
	NUM = 256, IDENT, NUL, ERR, CH,
	//保留字集
	BEGIN, END, FOR, PRO, FUN, DO, WHILE, VAR, ELSE, INT, CHAR, ARRAY, 
	CONST, IF, THEN, READ, WRITE, TO, DOWNTO,STRING, OF,
	//符号集
	PLUS, MINUS, TIMES, SLASH, COMMA, SEMICOLON,LPARENT,RPARENT,EQL,NEQ,
	PERIOD, LESS, LEQ, GREATER, GEQ, BECOMES, COLON, QUT, SGLQUT, LBRACKET, RBRACKET
};

typedef struct table_t
{
	int type;
	int val;
} table;

#endif