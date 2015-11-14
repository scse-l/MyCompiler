#include <iostream>
#include "global.h"

#pragma warning(disable:4996)

/*
 * 全局变量声明
*/
//用map代替table时的符号表和查找结果声明
std::map<std::string, int> symTable;
std::map<std::string, int>::iterator res;

unsigned int numMax = 10;  //允许的数字最大长度
char ch;     //从文件读入的一个字符
int symbol;  //一个token的属性
long long value;  //一个token的值:如果token是NUM,则val是其值;如果token是IDENT,则val是其在符号表中的位置
std::string ident;

int main()
{
	freopen("in.txt", "r", stdin);
	freopen("out.txt", "w", stdout);
	while (symbol != EOF)
	{
		symbol = getsym();
		if (symbol == IDENT)
		{
			printf("Type:Identifier\tName:%s\n", ident.c_str());
		}
		else if(symbol == NUM)
		{
			printf("Type:Number\tVallue:%d\n", value);
		}
		else if (symbol == ERR)
		{
		}
		else
		{
			printf("--------------------------\nWrong Type:%s\n------------------------\n",ident.c_str());
		}
	}
	printf("File Complete!\n");
	return 0;
}