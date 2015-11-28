#pragma once
#include "global.h"

int sematic(AST root);								//语义分析程序
int typeCheck(AST root);							//类型检查函数
int tableCheck(AST root, int level);							//符号表填查函数
std::vector<int>* argsTypes(AST_node t, int level);							//对参数类型进行分析
void args(AST_node t, std::vector<int> *types, int level);						//对参数段进行分析
LexType termCheck(AST_node term);
LexType factorCheck(AST_node factor);
LexType expCheck(AST_node expression);
int readStatCheck(AST_node t);
int proCallCheck(AST_node t);
int argsCheck(AST_node t, const void *addr);
