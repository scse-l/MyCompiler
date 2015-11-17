#pragma once
/*
 辅助函数
 */
#ifndef SUP_H
#define SUP_H

#include <string>

//输出相关信息
void print(std::string type, std::string attr, std::string value);
//检测字符是否匹配
bool match(char target);
//检测token类型是否匹配
bool match(int type);
//错误报告
void error(std::string msg);
//错误恢复函数
void recovery(int n, ...);
#endif