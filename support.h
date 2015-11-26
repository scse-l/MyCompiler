#pragma once
/*
 辅助函数
 */
#ifndef SUP_H
#define SUP_H
#include "global.h"
#include <string>




//输出相关信息
void print(std::string type, std::string attr, std::string value);
//检测字符是否匹配
bool match(char target);
//检测token类型是否匹配
bool match(int type);
//检测token类型是否匹配，是则生成语法树的节点并将其加入父节点的子节点集合中
bool match(int type, AST_node parent);


//记录错误信息
void error(std::string msg);
void error(int lineNo, std::string msg);
//输出错误信息
void errorRep();
//清除错误信息
void errorClean();
//错误恢复函数
void recovery(int n, ...);
#endif
