#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "table.h"

#ifndef GLOBAL_H
#define GLOBAL_H

//结构和枚举声明
typedef enum Lex_Type_e LexType;
typedef enum AST_Type_e ASTType;

//词法分析得出的类型
enum Lex_Type_e
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

//语法树所用的类型说明
enum AST_Type_e 
{
	PROGRAM, STATEMENT, TERMINAL, ROOT,
	CONSTDECL, CONSTDEF, VARDECL, VARDEF, PRODECL, PROHEAD, FUNDECL, FUNHEAD,
	ARGLIST,ARGS,EXPRESSION, TERM, FACTOR, CONDITION

};

//语法树结构定义
typedef struct AST_t* AST;
typedef AST AST_node;

struct AST_t {
	ASTType ast_type;									//记录当前节点的语法类型
	LexType lex_symbol;								//记录当前节点的词法类型
	AST_node parent;										//记录当前节点的父节点
	std::vector<AST_node> *children;						//记录当前节点的子节点
	int lineNo;												//记录当前节点的位置
	union 
	{
		std::string* ident;										//记录当前节点的单词
		int value;												//记录当前节点的值(如果是数字)
	} val;
	//其他属性待确定
	tableItem* tableItem;										//记录当前节点在符号表中的位置
};
/*
语法树相关操作
*/
/*
产生节点
*/
AST_node makeNode(ASTType ast_type, AST_node parent);
AST_node makeNode(ASTType ast_type, LexType lex_symbol, AST_node parent);
void printAST(AST_node root, int lev);

typedef struct {


} procedureTemplet;

typedef struct {


} functionTemplet;

typedef struct {
	int length;
	int type;
} arrayTemplet;
#endif
