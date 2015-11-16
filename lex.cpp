#include <iostream>
#include <string>
#include <map>
#include "global.h"
#include "support.h"

extern unsigned int numMax;
extern char ch;
extern long long value;
extern std::string ident;
extern std::map<std::string, int> keywordTable;
extern std::map<std::string, int> symTable;
extern std::map<std::string, int>::iterator res;

int lex()
{
	while (true)
	{
		//ch = getchar();
		//跳过所有空白符
		while (isspace(ch))
		{
			ch = getchar();
		}
		//ident.clear();
		//ident.append(sizeof(char), ch);
		if (isalpha(ch))
		{
			//字母开头
			ident.clear();						//清除上一个ident留下的内容
			ident.append(sizeof(char), ch);     //将当前字符添加至ident中
			while (isalnum(ch = getchar()))
			{
				//如果当前是字母或者数字，则其应该是当前标识符中的一部分
				ident.append(sizeof(char), ch);  //将当前字符添加至ident中
			}
			//标识符结束
			res = keywordTable.find(ident);
			if (res != keywordTable.end())
			{
				//当前标识符是保留字
				print("Keyword", "Name", ident.c_str());
				return keywordTable[ident];
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
			
			if (ident.length() > numMax)
			{
				//数据溢出
				value = 0;
				printf("----------------------------\n当前数字(%s)溢出!\n", ident.c_str());
				return NUM;
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
		else if (match(':'))
		{
			if (match('='))
			{
				//赋值号:=
				print("Sign", "Name", "BECOMES");
				return BECOMES;
			}
			else
			{
				//冒号':'
				print("Sign", "Name", "COLON");
				return COLON;
			}
		}
		else if (match('<'))
		{
			if (match('>'))
			{
				//不等号'<>'
				print("Sign", "Name", "NOT EQUAL");
				return NEQ;
			}
			else if (match('='))
			{
				//小于等于'<='
				print("Sign", "Name", "LESS OR EQUAL");
				return LEQ;
			}
			else
			{
				//小于号'<'
				print("Sign", "Name", "LESS");
				return LESS;
			}
		}
		else if (match('>'))
		{
			if (match('='))
			{
				//大于等于号'>='
				print("Sign", "Name", "GREATER OR EQUAL");
				return GEQ;
			}
			else
			{
				//大于号'>'
				print("Sign", "Name", "GREATER");
				return GREATER;
			}
		}
		else if (ch == EOF)
		{
			return EOF;
		}
		else 
		{
			//其他符号
			ident.clear();
			ident.append(sizeof(char), ch);
			res = keywordTable.find(ident);
			if (res != keywordTable.end() && res->second != 0)
			{
				//找到当前符号
				print("Sign", "Name", ident);
				ch = getchar();
				return res->second;
			}
			//当前符号为不可识别的符号
			//跳到下一个标识符或者数字开始的位置
			while (!isalnum(ch) && ch != EOF && keywordTable[std::string(1,ch)] == 0)
			{
				ch = getchar();
				ident.append(sizeof(char), ch);
			}
			ident.erase(--(ident.end()));			//去掉下一个标识符的开始字符
			if (ch == EOF)
			{
				return EOF;
			}
			return NUL;
		}
	}


}