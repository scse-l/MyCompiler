#include <iostream>
#include <map>
#include "global.h"        //结构和枚举声明
#include "support.h"       //辅助函数
#include "lex.h"           //词法分析函数

#pragma warning(disable:4996)

/*
 * 全局变量声明
*/
//用map代替table时的符号表和查找结果声明
std::map<std::string, int> keywordTable;
std::map<std::string, int> symTable;
std::map<std::string, int>::iterator res;

unsigned int numMax = 10;  //允许的数字最大长度
int lineNo = 1;            //当前行号
char ch = 0;               //从文件读入的一个字符
int symbol = 0;            //一个token的属性
long long value = 0;       //一个token的值:如果token是NUM,则val是其值;如果token是IDENT,则val是其在符号表中的位置
std::string ident;         //存储当前字符串

//初始化过程
void init();

int main()
{

	freopen("in_py.txt", "r", stdin);
	freopen("out.txt", "w", stdout);

	init();
	ch = getchar();
	while (symbol != EOF)
	{
		symbol = lex();
		if (symbol == IDENT)
		{
			printf("Type:Identifier\tName:%s\n", ident.c_str());
		}
		else if(symbol == NUM)
		{
			printf("Type:Number\tVallue:%d\n", value);
		}
		else if (symbol == STRING)
		{
			print("String", "Value", ident);
		}
		else if (symbol == CHAR)
		{
			print("Char", "Value", ident);
		}
		else if(symbol == NUL)
		{
			error("Unknown Character: " + ident);
		}
	}
	printf("File Complete!\n");
	return 0;
}

//初始化过程:添加保留字
void init()
{
	//保留字集
	keywordTable.insert(std::pair<std::string, int>("begin",BEGIN));
	keywordTable.insert(std::pair<std::string, int>("end", END));
	keywordTable.insert(std::pair<std::string, int>("for", FOR));
	keywordTable.insert(std::pair<std::string, int>("while", WHILE));
	keywordTable.insert(std::pair<std::string, int>("procedure", PRO));
	keywordTable.insert(std::pair<std::string, int>("function", FUN));
	keywordTable.insert(std::pair<std::string, int>("do", DO));
	keywordTable.insert(std::pair<std::string, int>("to", TO));
	keywordTable.insert(std::pair<std::string, int>("downto", DOWNTO));
	keywordTable.insert(std::pair<std::string, int>("var", VAR));
	keywordTable.insert(std::pair<std::string, int>("else", ELSE));
	keywordTable.insert(std::pair<std::string, int>("integer", INT));
	keywordTable.insert(std::pair<std::string, int>("char", CHAR));
	keywordTable.insert(std::pair<std::string, int>("array", ARRAY));
	keywordTable.insert(std::pair<std::string, int>("const", CONST));
	keywordTable.insert(std::pair<std::string, int>("if", IF));
	keywordTable.insert(std::pair<std::string, int>("then", THEN));
	keywordTable.insert(std::pair<std::string, int>("read", READ));
	keywordTable.insert(std::pair<std::string, int>("write", WRITE));
	keywordTable.insert(std::pair<std::string, int>("of", OF));

	//符号集
	keywordTable.insert(std::pair<std::string, int>("+", PLUS));
	keywordTable.insert(std::pair<std::string, int>("-", MINUS));
	keywordTable.insert(std::pair<std::string, int>("*", TIMES));
	keywordTable.insert(std::pair<std::string, int>("/", SLASH));
	keywordTable.insert(std::pair<std::string, int>(",", COMMA));
	keywordTable.insert(std::pair<std::string, int>(";", SEMICOLON));
	keywordTable.insert(std::pair<std::string, int>("(", LPARENT));
	keywordTable.insert(std::pair<std::string, int>(")", RPARENT));
	keywordTable.insert(std::pair<std::string, int>("=", EQL));
	keywordTable.insert(std::pair<std::string, int>(".", PERIOD));
	keywordTable.insert(std::pair<std::string, int>("<", LESS));
	keywordTable.insert(std::pair<std::string, int>(">", GREATER));
	keywordTable.insert(std::pair<std::string, int>(":", COLON));
	keywordTable.insert(std::pair<std::string, int>("'", SGLQUT));
	keywordTable.insert(std::pair<std::string, int>("\"", QUT));
	keywordTable.insert(std::pair<std::string, int>("[", LBRACKET));
	keywordTable.insert(std::pair<std::string, int>("]", RBRACKET));
}

