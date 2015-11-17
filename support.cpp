/*
 * 辅助函数
 */
#include <iostream>
#include <string>
#include <cstdarg>
#include <set>
#include "lex.h"

extern char ch;
extern int lineNo;
extern int symbol;

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
//检查类型是否匹配
bool match(int type)
{
	if (symbol == type)
	{
		symbol = lex();
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

/*
	错误恢复
	n:接下来参数的个数
*/
void recovery(int n, ...)
{
	va_list ap;
	int type = 0;
	std::set<int> symSet;

	va_start(ap, n);
	while (n-- > 0)
	{
		type = va_arg(ap, int);
		symSet.insert(type);
	}
	va_end(ap);
	while (symSet.find(symbol) == symSet.end())
	{
		//不是错误恢复集合中的token
		symbol = lex();
	}
	return;
}