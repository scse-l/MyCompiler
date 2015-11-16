#pragma once
/*
 辅助函数
 */
#include <string>

#ifndef SUP_H
#define SUP_H

//输出相关信息
void print(std::string type, std::string attr, std::string value);
//检测字符是否匹配
bool match(char target);

#endif