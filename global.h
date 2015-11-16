#include <iostream>
#include <string>
//当前测试方便，用map代替符号表
#include <map>

#ifndef GLOBAL_H
#define GLOBAL_H

//结构和枚举声明
enum MyEnum
{
	NUM = 256, IDENT, NUL, ERR,
	//保留字集
	BEGIN, END, FOR, PRO, FUN, DO, WHILE, VAR, ELSE, INT, CHAR, ARRAY, 
	CONST, IF, THEN, READ, WRITE, TO, DOWNTO,
	//符号集
	PLUS, MINUS, TIMES, SLASH, COMMA, SEMICOLON,LPARENT,RPARENT,EQL,NEQ,
	PERIOD, LESS, LEQ, GREATER, GEQ, BECOMES, COLON
};

typedef struct table_t
{
	std::string name;
	int type;
	int val;
} table;

//函数声明
int lex();				//词法分析函数
#endif