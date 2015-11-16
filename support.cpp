/*
 * 辅助函数
 */
#include <iostream>
#include <string>

extern char ch;
extern int lineNo;
//输出相关信息的函数
void print(std::string type, std::string attr, std::string value)
{
	printf("Type:%s\t%s:%s\n", type.c_str(), attr.c_str(), value.c_str());
}
//检测字符是否匹配
bool match(char target)
{
	if (ch == target)
	{
		ch = getchar();
		return true;
	}
	else
	{
		return false;
	}
}
//错误报告
void error(std::string msg)
{
	printf(">>>>>>>>>>>>>>>>>>>>>>>>\n\tError: line %d\t\n\tError message: %s\n>>>>>>>>>>>>>>>>>>>>>>>>\n", lineNo, msg.c_str());
}