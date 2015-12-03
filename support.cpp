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
		ident.append(sizeof(char), ch);
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


//检测token类型是否匹配，是则生成语法树的节点并将其加入父节点的子节点集合中
bool match(int type, AST_node parent)
{
	if (symbol == type)
	{
		print(type);
		AST_node t = makeNode(TERMINAL, (LexType)type, parent);
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
void error(int lineNo, std::string msg)
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
		//printf("%s\n", (*i).c_str());
	}
}

//清楚错误信息
void errorClean()
{
	errorCount = 0;
	errorMsg.clear();
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


/*
	语法树相关操作
*/
AST_node makeNode(ASTType ast_type, AST_node parent)
{
	AST_node t = (AST_node)malloc(sizeof(AST_t));
	t->ast_type = ast_type;
	t->lex_symbol = (LexType)0;
	t->parent = parent;
	t->lineNo = lineNo;
	t->children = new std::vector<AST_node>;
	t->val.ident = new std::string(ident);
	t->tableItem = NULL;
	t->level = -1;
	if (ast_type == FUNDEF || ast_type == PRODEF || ast_type == ROOT)
		t->symTable = new Table();
	else
	{
		t->symTable = parent == NULL ? NULL : parent->symTable;
	}
	if(parent != NULL)
		parent->children->push_back(t);
	return t;
}

AST_node makeNode(ASTType ast_type, LexType symbol, AST_node parent)
{
	AST_node t = (AST_node)malloc(sizeof(AST_t));
	t->ast_type = ast_type;
	t->parent = parent;
	t->lex_symbol = symbol;
	t->tableItem = NULL;
	t->symTable = parent->symTable;
	if (symbol == NUM)
	{
		t->val.value = value;
	}
	else
	{
		t->val.ident = new std::string(ident);
	}
	t->children = new std::vector<AST_node>;
	t->lineNo = lineNo;
	if(parent != NULL)
		parent->children->push_back(t);
	return t;
}

void printAST(AST_node root, int lev)
{
	std::string *space = new std::string(lev, '\t');
	if (root != NULL)
	{
		switch (root->ast_type)
		{
		case TERMINAL:		
			std::cout << *space << "Terminal\t" << root->lex_symbol << "\t";
			if (root->lex_symbol == NUM)
			{
				std::cout << root->val.value << std::endl;
			}
			else
			{
				std::cout << *(root->val.ident) << std::endl;
			}
			break;
		case CONSTDECL:
			std::cout << *space << "ConstDecl" << std::endl;
			break;
		case PROGRAM:
			std::cout << *space << "Program" << std::endl;
			break;
		case CONSTDEF:
			std::cout << *space << "ConstDef" << std::endl;
			break;
		case CONST:
			std::cout << *space << "Const" << std::endl;
			break;
		case VAR:
			std::cout << *space << "VAR" << std::endl;
			break;
		case VARDECL:
			std::cout << *space << "VarDecl" << std::endl;
			break;
		case VARDEF:
			std::cout << *space << "VarDef" << std::endl;
			break;
		case PRODECL:
			std::cout << *space << "Procedure Declaration" << std::endl;
			break;
		case PROHEAD:
			std::cout << *space << "Procedure Head Declaration" << std::endl;
			break;
		case FUNDECL:
			std::cout << *space << "Function Declaration" << std::endl;
			break;
		case FUNHEAD:
			std::cout << *space << "Function Head Declaration" << std::endl;
			break;
		case ARGLIST:
			std::cout << *space << "Arglist" << std::endl;
			break;
		case ARGS:
			std::cout << *space << "Args" << std::endl;
			break;
		case EXPRESSION:
			std::cout << *space << "Expression" << std::endl;
			break;
		case CONDITION:
			std::cout << *space << "Condition" << std::endl;
			break;
		case TERM:
			std::cout << *space << "Term" << std::endl;
			break;
		case FACTOR:
			std::cout << *space << "Factor" << std::endl;
			break;

		default:
			break;
		}
		if (root->children->size() == 0)
		{
			return;
		}
		for (std::vector<AST_node>::iterator i = root->children->begin(); i != root->children->end(); i++)
		{
			printAST(*i, lev + 1);
		}
	}
	return;
}

