#pragma once
/*
	生成汇编
*/
#include "global.h"

int asmMaker(AST_node cur, AST_node parent);
void emitASM(std::string *ins, std::string *op1, std::string *op2);
