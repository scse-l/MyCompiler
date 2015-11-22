/*
 * 辅助函数
 */
#include <iostream>
#include <string>
#include <cstdarg>
#include <set>
#include <vector>
#include "global.h"
#include "lex.h"

#pragma warning(disable:4996)

extern char ch;
extern int lineNo;
extern int symbol;
extern std::string ident;
extern long long value;
extern unsigned int errorCount;
extern std::vector<std::string> errorMsg;

//输出相关信息的函数
void print(std::string type, std::string attr, std::string value)
{
	printf("Type:%s\t%s:%s\n", type.c_str(), attr.c_str(), value.c_str());
}
void print(int type)
{
	switch (type)
	{
		case NUM:printf("Type:Number\tValue:%d\n", value); break;
		case IDENT:printf("Type:Ident\tName:%s\n", ident.c_str()); break;
		case CH:printf("Type:CHAR\tValue:%s\n", ident.c_str()); break;
		default:
			break;
	}
	
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
		print(type);
		symbol = lex();
		return true;
	}
	else
	{
		return false;
	}
}

//记录错误信息
void error(std::string msg)
{
	errorCount++;
	char _message[10];
	char* message = itoa(lineNo, _message, 10);
	std::string s = message;
	msg = ">>>>>>  line " + s + "   " + msg;
	errorMsg.push_back(msg);
}

//输出错误信息
void errorRep()
{
	for (std::vector<std::string>::iterator i = errorMsg.begin(); i != errorMsg.end(); i++)
	{
		std::cout << *i << std::endl;
	}
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
		if (symbol == EOF)
		{
			//在错误恢复时遇到文件末尾
			error("File is incompleted");
			exit(1);
		}
		//不是错误恢复集合中的token
		symbol = lex();
	}
	return;
}