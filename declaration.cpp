/*
	 声明部分
	 包括：常量说明部分、变量说明部分、过程说明部分、函数说明部分
*/

#include "global.h"
#include "support.h"
#include "lex.h"
#include "declaration.h"
#include "program.h"

extern int symbol;
extern std::string ident;

//常量声明
//<常量说明部分>::=const<常量定义>{,<常量定义>};
AST_node constdecl(AST_node parent)
{
	AST_node t = makeNode(CONSTDECL, parent);
	if (!match(CONST, t))
	{
		return NULL;
	}
	printf("----------------CONST DECLARATION BEGINS--------------\n");
	//常量定义开始
	constdef(t);
	while (!match(SEMICOLON, t))
	{
		//常量声明没有结束
		if (match(COMMA,t))
		{
			constdef(t);
		}
		else
		{
			error("Syntax Error: Neither a semicolon nor a comma");
			//错误恢复
			recovery(6, IDENT, SEMICOLON, VAR, PRO, FUN, BEGIN);
			if (symbol == IDENT)
			{
				constdef(t);
			}
			else if (symbol != SEMICOLON)
			{
				break;
			}
		}
	}
	printf("----------------CONST DECLARATION END--------------\n");
	return t;
}

//常量定义
//<常量定义>::=<标识符>＝<常量>
AST_node constdef(AST_node parent)
{
	AST_node t = makeNode(CONSTDEF, parent);
	if (match(IDENT,t) && match(EQL,t))
	{
		if (match(CH,t))
		{
			//<标识符>＝<字符>
			
		}
		else if (match(NUM,t))
		{
			//<标识符>＝<无符号整数>

		}
		else if ((match(MINUS, t) || match(PLUS, t)) && match(NUM, t))
		{
			//<标识符>＝[+-]<无符号整数>
		}
		else
		{
			error("Const Declaration Error");
			//错误恢复
			recovery(2, COMMA, SEMICOLON);
			return NULL;
		}
	}
	else
	{
		error("Const Declaration Error");
		//错误恢复
		recovery(2, COMMA, SEMICOLON);
		return NULL;
	}
	return t;
}

//变量声明
//<变量说明部分>::=var<变量说明>;{<变量说明>;}
AST_node vardecl(AST_node parent)
{
	AST_node t = makeNode(VARDECL, parent);
	if (!match(VAR,t))
	{
		return NULL;
	}
	printf("----------------VAR DECLARATION BEGINS--------------\n");
	//变量定义开始
	do
	{
		vardef(t);
		if (!match(SEMICOLON, t))
		{
			error("No Semicolon Found After a var declaration");
			recovery(5, SEMICOLON, IDENT, PRO, FUN, BEGIN);
		}
	} while (symbol == IDENT);
	printf("----------------VAR DECLARATION END--------------\n");
	return t;
}

//变量定义
//<变量说明>::=<标识符>{,<标识符>}:<类型>
AST_node vardef(AST_node parent)
{
	AST_node t = makeNode(VARDEF, parent);
	if (!match(IDENT, t))
	{
		error("Var Declaration Error");
		recovery(5, SEMICOLON, IDENT, PRO, FUN, BEGIN);
	}
	while (match(COMMA,t))
	{
		if (!match(IDENT,t))
		{
			error("Var Declaration Error");
			recovery(2, COLON, COMMA);
			break;
		}
	}
	//多个标识符已经匹配完毕
	if (match(COLON,t))
	{
		if (match(INT,t) || match(CHAR,t))
			{
				//基本类型
			}
		else if (match(ARRAY,t))
			{
				//数组声明
				if (match(LBRACKET,t) && match(NUM,t) && match(RBRACKET,t) &&
					match(OF,t) && (match(INT,t) || match(CHAR,t)))
				{
				}
				else
				{
					error("Array Declaration Error");
					//错误恢复：跳至分号
					recovery(2, SEMICOLON, IDENT);
					return NULL;
				}
			}
		else
			{
				//类型错误
				error("Unknown Var Type");
				//错误恢复：跳至分号
				recovery(2, SEMICOLON, IDENT);
				return NULL;
			}
	}
	else
	{
		//没有冒号
		error("Var Declaration Error");
		//错误恢复：跳至分号
		recovery(2, SEMICOLON, IDENT);
	}
	return t;
}

//过程声明
//<过程说明部分>::=<过程首部><分程序>;{过程说明部分}
AST_node prodecl(AST_node parent)
{
	AST_node t = makeNode(PRODECL, parent);
	while (symbol == PRO)
	{
		printf("----------------PROCEDURE DECLARATION BEGINS--------------\n");
		//过程声明开始
		prohead(t);					//过程首部语法分析函数
		program(t);					//分程序语法分析函数
		if (!match(SEMICOLON,t))
		{
			error("Missing Semicolon After Procedure");
			recovery(3, PRO, FUN, BEGIN);
		}
	}
	printf("----------------ALL PROCEDURE DECLARATION END--------------\n");
	return 0;
}

//过程首部
//<过程首部>::=procedure<标识符>[<形式参数表>];
AST_node prohead(AST_node parent)
{
	AST_node t = makeNode(PROHEAD, parent);

	match(PRO,t);
	if (!match(IDENT,t))
	{
		error("Procedure declaration error");
		recovery(7, SEMICOLON, LPARENT, CONST, VAR, PRO, FUN, BEGIN);
	}
	arglist(t);
	if (!match(SEMICOLON,t))
	{
		error("Missing Semicolon");
		recovery(5, CONST,VAR,PRO,FUN,BEGIN);
	}
	return t;
}

//形式参数表
//<形式参数表>::='('<形式参数段>{;<形式参数段>}')'
AST_node arglist(AST_node parent)
{
	AST_node t = NULL;
	//因为形式参数表是可选的，所以没有else部分
	if (match(LPARENT))
	{
		t = makeNode(ARGLIST, parent);
		//形式参数段语法分析函数
		args(t);
		while (match(SEMICOLON,t))
		{
			args(t);
		}
		//形式参数段均分析完毕
		if (!match(RPARENT,t))
		{
			error("Missing Right Parent");
			//错误恢复：
			recovery(6, SEMICOLON, VAR, CONST, PRO, FUN, BEGIN);
			match(SEMICOLON);
			return NULL;
		}
	}
	return t;
}

//形式参数段语法分析函数
//<形式参数段>::=[var]<标识符>{,<标识符>}:<基本类型>
AST_node args(AST_node parent)
{
	AST_node t = makeNode(ARGS, parent);
	match(VAR,t);
	if (!match(IDENT, t))
	{
		error("Wrong Args");
		//错误恢复
		recovery(4, COMMA, COLON, SEMICOLON, RPARENT);
	}
	while (match(COMMA, t))
	{
		if (!match(IDENT, t))
		{
			error("Not a identifier");
			//错误恢复
			recovery(4, COMMA, COLON, SEMICOLON, RPARENT);
		}
	}
	if (match(COLON, t))
	{
		if (match(INT, t))
		{
			return t;
		}
		else if (match(CHAR, t))
		{
			return t;
		}
		else
		{
			//非基本类型
			error("Wrong Type:must be basic type");
			//错误恢复：跳至分号或者右括号
			recovery(2, SEMICOLON, RPARENT);
			return NULL;
		}
	}
	else
	{
		error("Missing Colon");
		//错误恢复：跳至分号或者右括号
		recovery(2, SEMICOLON, RPARENT);
		return NULL;
	}
	return t;
}

//函数声明
//<函数说明部分>::=<函数首部><分程序>;{<函数说明部分>}
AST_node fundecl(AST_node parent)
{
	AST_node t = makeNode(FUNDECL,parent);
	while (symbol == FUN)
	{
		printf("----------------FUNCTION DECLARATION BEGINS--------------\n");
		//函数声明开始
		funhead(t);
		program(t);	
		if (!match(SEMICOLON,t))
		{
			error("Missing Semicolon After Function");
			recovery(2, FUN, BEGIN);
		}
	}
	printf("----------------ALL FUNCTION DECLARATION END--------------\n");
	return t;
}

//函数首部
//<函数首部>::=function<标识符>[<形式参数表>]:<基本类型>;
AST_node funhead(AST_node parent)
{
	AST_node t = makeNode(FUNHEAD, parent);
	match(FUN,t);
	if (!match(IDENT,t))
	{
		error("Function Declaration Error");
		recovery(8, COLON, SEMICOLON, LPARENT, VAR, CONST, FUN, PRO, BEGIN);
	}
	arglist(t);
	if (!match(COLON,t))
	{
		error("Missing Colon After Args");
		recovery(8, INT, CHAR, SEMICOLON, VAR, CONST, PRO, FUN, BEGIN);
	}
	if (!match(INT,t))
	{
		if (!match(CHAR,t))
		{
			error("Missing Return Type");
			recovery(6, SEMICOLON, CONST, VAR, FUN, PRO, BEGIN);
		}
	}
	if (!match(SEMICOLON,t))
	{
		error("Missing Semicolon After Function Head");
		recovery(5,CONST, VAR, PRO, FUN, BEGIN);
	}
	return t;
}