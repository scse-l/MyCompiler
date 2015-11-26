#include <iostream>
#include <map>
#include <vector>
#include "global.h"        //结构和枚举声明
#include "support.h"       //辅助函数
#include "lex.h"           //词法分析函数
#include "program.h"
#include "table.h"

#pragma warning(disable:4996)

/*
	全局变量声明
*/
std::map<std::string, int> keywordTable;							//保留字表
Table symTable;														//符号表
std::vector<std::string> errorMsg;									//错误记录		

unsigned int errorCount = 0;			//错误总数
unsigned int numMax = 10;				//允许的数字最大长度
int lineNo = 1;							//当前行号
char ch = 0;							//从文件读入的一个字符
int symbol = 0;							//一个token的属性
long long value = 0;					//一个token的值:如果token是NUM,则val是其值;如果token是IDENT,则val是其在符号表中的位置
std::string ident;						//存储当前字符串
unsigned int level;						//记录当前层数 

/*
	初始函数声明
*/
void init();								//初始化过程
int syntax(AST root);						//语法分析程序
int sematic(AST root);								//语义分析程序
int typeCheck(AST root);							//类型检查函数
int tableCheck(AST root, int level);							//符号表填查函数
std::vector<int>* argsTypes(AST_node t);							//对参数类型进行分析

/*
	对参数段进行分析
	t:语法树类型为ARGS的节点
*/
void args(AST_node t, std::vector<int> *types);

int main()
{
	AST root = makeNode(ROOT,NULL);

	freopen("in_py.txt", "r", stdin);
	freopen("out.txt", "w", stdout);

	//初始化
	init();
	//语法分析程序
	syntax(root);

	if (errorCount != 0)
	{
		printf("%d error found!\n", errorCount);
		errorRep();
		return 0;
	}
	printf("File Complete!\n\n");
//	printAST(root, 0);
	errorClean();
	//语义分析程序
	sematic(root);
	if (errorCount != 0)
	{
		printf("%d error found\n", errorCount);
		errorRep();
		return 0;
	}
	
	freopen("Table.csv", "w", stdout);

	//for (std::map<std::string, itemList>::iterator i = symTable.begin();
	//	i != symTable.end(); i++)
	//{
	//	itemList::iterator j = (*i).second.begin();
	//	if (j->attribute == INT)
	//	{
	//		std::cout << *((int *)j->addr) << std::endl;
	//	}
	//	else if (j->attribute == CH)
	//	{
	//		std::cout << *((char *)j->addr) << std::endl;
	//	}
	//	else
	//	{
	//		std::cout << ((procedureTemplet *)j->addr)->args << std::endl;
	//	}
	//}


	printTable(symTable);

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

	//变量初始化
	ch = getchar();
	symbol = lex();

}

//语法分析程序
int syntax(AST root)
{
	//<程序>::=<分程序>.
	while (symbol != EOF)
	{
		program(root);
		if (!match(PERIOD, root))
		{
			error("Missing Period!Program is not completed");
			recovery(1, EOF);
		}
	}
	return 0;
}

//语义分析程序
int sematic(AST root)
{

	//填查符号表
	tableCheck(root,0);
	//类型检查
	typeCheck(root);

	return 0;
}

//类型检查函数
int typeCheck(AST root)
{

	return 0;
}

//符号表填查函数
int tableCheck(AST root, int level)
{
	std::vector<AST_node>::iterator i = root->children->begin();

	if (root->ast_type == CONSTDEF)
	{
		std::string name;
		void *addr = NULL;
		//接下是常量定义
		//<常量定义>::=<标识符>＝<常量>
		for (; i != root->children->end(); i++)
		{
			if ((*i)->lex_symbol == IDENT)
			{
				//当前节点是标识符
				name = *((*i)->val.ident);
			}
			else if ((*i)->lex_symbol == NUM)
			{
				addr = (int *)malloc(sizeof(int));
				*((int *)addr) = (*i)->val.value;
				(*i)->tableItem = tableInsert(symTable, name, CONST, NUM, level, addr, (*i)->lineNo);
			}
			else if ((*i)->lex_symbol == CH)
			{
				addr = (char *)malloc(sizeof(char));
				char c = (*i)->val.ident->c_str()[0];
				*((char *)addr) = c;
				(*i)->tableItem = tableInsert(symTable, name, CONST, CH, level, addr, (*i)->lineNo);
			}
		}
		return 0;
	}
	else if (root->ast_type == VARDEF)
	{
		int attribute = 0;
		void *addr = NULL;
		std::vector<std::string> names;
		//<变量说明>::=<标识符>{,<标识符>}:<类型>
		for (; i != root->children->end(); i++)
		{
			if ((*i)->lex_symbol == IDENT)
			{
				names.push_back(*((*i)->val.ident));
			}
			else if ((*i)->lex_symbol == ARRAY)
			{
				//当前类型是数组
				//数组类型::=array'['<无符号整数>']'of <基本类型>
				attribute = ARRAY;
				addr = (arrayTemplet *)malloc(sizeof(arrayTemplet));
			}
			else if ((*i)->lex_symbol == NUM)
			{
				if(addr != NULL)
					((arrayTemplet*) addr)->length = (*i)->val.value;
			}
			else if ((*i)->lex_symbol == INT || (*i)->lex_symbol == CHAR)
			{
				if (addr != NULL)
				{
					//当前变量类型是数组
					((arrayTemplet*)addr)->type = (*i)->lex_symbol;
				}
				else
				{
					//当前变量类型是基本类型
					attribute = (*i)->lex_symbol;
					if ((*i)->lex_symbol == INT)
					{
						for (unsigned int j = 0; j < names.size(); j++)
						{
							addr = (int *)malloc(sizeof(int));
							*((int *)addr) = 0;
							tableItem* item = tableInsert(symTable, names[j], VAR, attribute, level, addr, (*i)->lineNo);
						}
					}
					else
					{
						for (unsigned int j = 0; j < names.size(); j++)
						{
							addr = (char *)malloc(sizeof(char));
							*((char *)addr) = 't';
							tableItem* item = tableInsert(symTable, names[j], VAR, attribute, level, addr, (*i)->lineNo);
						}
					}
				}
			}
		}
		return 0;
	}
	else if (root->ast_type == PROHEAD)
	{
		procedureTemplet *addr = (procedureTemplet *)malloc(sizeof(procedureTemplet));
		addr->types = NULL;
		addr->args = 0;
		//当前节点是过程首部
		//<过程首部>::=procedure<标识符>[<形式参数表>];
		for (; i != root->children->end(); i++)
		{
			if ((*i)->lex_symbol == IDENT)
			{
				tableInsert(symTable, *((*i)->val.ident), PRO, 0, level, addr, (*i)->lineNo);
			}
			else if ((*i)->ast_type == ARGLIST)
			{
				addr->types = argsTypes(*i);
				addr->args = addr->types->size();
			}
		}
		return 0;
	}
	else if (root->ast_type == FUNHEAD)
	{
		functionTemplet *addr = (functionTemplet *)malloc(sizeof(functionTemplet));
		std::string name;
		addr->args = 0;
		addr->types = NULL;
		//当前节点是函数首部
		//<函数首部>::=function<标识符>[<形式参数表>]:<基本类型>;
		for (; i != root->children->end(); i++)
		{
			if ((*i)->lex_symbol == IDENT)
			{
				name = *((*i)->val.ident);
				
			}
			else if ((*i)->ast_type == ARGLIST)
			{
				addr->types = argsTypes(*i);
				addr->args = addr->types->size();
			}
			else if ((*i)->lex_symbol == INT || (*i)->lex_symbol == CHAR)
			{
				tableInsert(symTable, name, FUN, (*i)->lex_symbol, level, addr, (*i)->lineNo);
			}
		}
		return 0;
	}
	else if (root->ast_type == PROGRAM)
	{
		//遍历对每个子节点进行符号表填查
		for (; i != root->children->end(); i++)
		{
				tableCheck(*i, level+1);
		}
		return 0;
	}
	else
	{
		//遍历对每个子节点进行符号表填查
		for (; i != root->children->end(); i++)
		{
			tableCheck(*i, level);
		}
		return 0;
	}

	return 0;
}

/*
	对参数表进行分析
	t:语法树类型为ARGLIST的节点
*/
std::vector<int>* argsTypes(AST_node t)							
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::vector<int>* types = new std::vector<int>;

	//<形式参数表>:: = '('<形式参数段>{; <形式参数段>}')'
	for (; i != t->children->end(); i++)
	{
		if ((*i)->ast_type == ARGS)
		{
			//当前节点是个参数段
			args(*i, types);
		}
	}
	return types;
}

/*
	对参数段进行分析
	t:语法树类型为ARGS的节点
*/
void args(AST_node t, std::vector<int> *types)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	int cnt = 0;

	//<形式参数段>::=[var]<标识符>{,<标识符>}:<基本类型>
	for (; i != t->children->end(); i++)
	{
		if ((*i)->lex_symbol == IDENT)
		{
			cnt++;
		}
		else if ((*i)->lex_symbol == INT || (*i)->lex_symbol == CHAR)
		{
			break;
		}
	}
	while (cnt > 0)
	{
		types->push_back((*i)->lex_symbol);
		cnt--;
	}

	return;
}