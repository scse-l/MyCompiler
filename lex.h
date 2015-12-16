#pragma once
/*
	词法分析函数的声明
*/
#ifndef LEX_H
#define LEX_H

typedef enum Lex_Type_e LexType;
//词法分析得出的类型
enum Lex_Type_e
{
	//类型说明
	//CH为字符类型，与CHAR保留字区分
	NUM = 256, IDENT, NUL, ERR, CH,
	//保留字集
	BEGIN, END, FOR, PRO, FUN, DO, WHILE, VAR, ELSE, INT, CHAR, ARRAY,
	CONST, IF, THEN, READ, WRITE, TO, DOWNTO, STRING, OF,
	//符号集
	PLUS, MINUS, TIMES, SLASH, COMMA, SEMICOLON, LPARENT, RPARENT, EQL, NEQ,
	PERIOD, LESS, LEQ, GREATER, GEQ, BECOMES, COLON, QUT, SGLQUT, LBRACKET, RBRACKET,
	//临时变量
	TEMP
};

int lex();                  //词法分析函数

#endif