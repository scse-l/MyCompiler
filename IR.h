#pragma once
#include "global.h"
#include <string>
typedef enum 
{
	/*
		四元式指令
	*/
	//PLUS,MINUS,TIMES,DIV,EUQ
} IR_INS;

std::string* IREmit(AST_node root);
std::string* assignStatEmit(AST_node t);
std::string* ifStatEmit(AST_node t);
std::string* conditionEmit(AST_node t);
std::string* expEmit(AST_node t);
std::string *factorEmit(AST_node t);
std::string *termEmit(AST_node t);
std::string *makeTempReg(void);
std::string *makeNewLable(void);
std::string* emit(std::string op, std::string *res, std::string *op1, std::string *op2);
std::string* emit(std::string *lable, std::string op, std::string *res, std::string *op1, std::string *op2);
void putLable(std::string *lable);