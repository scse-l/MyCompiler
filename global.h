#pragma once
#include <iostream>
#include <map>
#include <vector>
#include "lex.h"
#include "table.h"

#ifndef GLOBAL_H
#define GLOBAL_H

//结构和枚举声明

typedef enum AST_Type_e ASTType;


//语法树所用的类型说明
enum AST_Type_e 
{
	PROGRAM, STATEMENT, TERMINAL, ROOT,
	IFSTAT, FORSTAT, ASSIGNSTAT, WRITESTAT, DOSTAT, READSTAT, STATS, EMPTY,CALL,
	CONSTDECL, CONSTDEF, VARDECL, VARDEF, PRODECL, PRODEF, PROHEAD, FUNDECL, FUNDEF, FUNHEAD,
	ARGLIST,ARGS,EXPRESSION, TERM, FACTOR, CONDITION

};

//语法树结构定义
typedef struct AST_t* AST;
typedef AST AST_node;

struct AST_t {
	ASTType ast_type;										//记录当前节点的语法类型
	LexType lex_symbol;										//记录当前节点的词法类型
	AST_node parent;										//记录当前节点的父节点
	std::vector<AST_node> *children;						//记录当前节点的子节点
	int lineNo;												//记录当前节点的位置
	int level;												//记录当前节点的层数
	union 
	{
		std::string* ident;										//记录当前节点的单词
		int value;												//记录当前节点的值(如果是数字)
	} val;

	//其他属性待确定

	tableItem* tableItem;										//记录当前节点在符号表中的位置
	Table *symTable;											//如果当前节点是Program，则该节点存放改分程序的符号表，其余类型则为NULL
};

//符号表操作
/*
	查找操作：若在所给符号表中找到则返回指向该符号表项的指针，否则返回NULL
	t:待查找的符号表
	name:欲查找的符号
	parent:当前节点的父节点
*/
tableItem* tableFind(Table &t, std::string name, AST_node parent);

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
	int args;						//记录参数的个数
	std::vector<LexType>* types;		//记录参数的类型
	int totalSpace;
} procedureTemplet;

typedef struct {
	int args;						//记录参数的个数
	std::vector<LexType>* types;		//记录参数的类型
	int totalSpace;

} functionTemplet;

typedef struct {
	int length;						//记录数组的上界
	LexType type;					//记录数组的类型
} arrayTemplet;
#endif
