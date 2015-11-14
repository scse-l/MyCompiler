#include <iostream>
#include <string>
#include <map>
#include "global.h"

extern unsigned int numMax;
extern char ch;
extern long long value;
extern std::string ident;
extern std::map<std::string, int> symTable;
extern std::map<std::string, int>::iterator res;

int getsym()
{
	while (true)
	{
		ch = getchar();
		ident.clear();
		ident.append(sizeof(char), ch);
		if (isalpha(ch))
		{
			//字母开头
			ident.clear();          //清除上一个ident留下的内容
			ident.append(sizeof(char), ch);     //将当前字符添加至ident中
			while (isalnum(ch = getchar()))
			{
				//如果当前是字母或者数字，则其应该是当前标识符中的一部分
				ident.append(sizeof(char), ch);  //将当前字符添加至ident中
			}
			//标识符结束
			ungetc(ch, stdin);
			res = symTable.find(ident);
			if (res == symTable.end())
			{
				//没有找到当前标识符
				symTable.insert(std::pair<std::string, int>(ident, IDENT));
			}
			else
			{
				if (symTable[ident] != IDENT)
				{
					printf("类型不匹配！\n");
				}
			}
			return IDENT;
		}
		else if (isdigit(ch))
		{
			//数字
			ident.clear();
			ident.append(sizeof(char), ch);
			while (isdigit(ch = getchar()))
			{
				//保存数值
				ident.append(sizeof(char), ch);
			}
			ungetc(ch, stdin);
			if (ident.length() > numMax)
			{
				//数据溢出
				value = 0;
				printf("----------------------------\n当前数字(%s)溢出!\n", ident.c_str());
				return ERR;
			}
			else {
				//计算数值
				value = 0;
				for (std::string::iterator itr = ident.begin(); itr != ident.end();itr++)
				{
					value = value * 10 + (*itr - '0');
				}
			}
			return NUM;
		}
		else if (isspace(ch))
		{
			//跳过接下来的所有空白符
			ident.clear();
			while (isspace(ch))
			{
				ch = getchar();
			}
			ungetc(ch, stdin);
		}
		else if(ch == EOF)
		{
			return EOF;
		}
		else
		{
			//调到下一个标识符或者数字开始的位置
			while (!isalnum(ch) && ch != EOF)
			{
				ch = getchar();
			}
			if (ch == EOF)
			{
				return EOF;
			}
			ungetc(ch, stdin);
			return NUL;
		}
	}


}