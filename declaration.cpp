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
extern std::map<std::string, int> symTable;

//常量声明
//<常量说明部分>::=const<常量定义>{,<常量定义>};
int constdecl()
{
	if (!match(CONST))
	{
		return 0;
	}
	printf("----------------CONST DECLARATION BEGINS--------------\n");
	//常量定义开始
	constdef();
	while (!match(SEMICOLON))
	{
		//常量声明没有结束
		if (match(COMMA))
		{
			constdef();
		}
		else
		{
			error("Syntax Error: Neither a semicolon nor a comma");
			//错误恢复
			while (!match(SEMICOLON))
			{
				symbol = lex();
			}
			return 1;
		}
	}
	return 0;
}

//常量定义
//<常量定义>::=<标识符>＝<常量>
int constdef()
{
	if (match(IDENT) && match(EQL))
	{
		if (match(CHAR))
		{
			//<标识符>＝<字符>

		}
		else if (match(NUM))
		{
			//<标识符>＝<无符号整数>

		}
		else if ((match(MINUS) || match(PLUS)) && match(NUM))
		{
			//<标识符>＝[+-]<无符号整数>
		}
		else
		{
			error("Const Declaration Error");
			//错误恢复
			while (!match(COMMA) && !match(SEMICOLON))
			{
				symbol = lex();
			}
			return 1;
		}
	}
	else
	{
		error("Const Declaration Error");
		//错误恢复
		while (!match(COMMA) && !match(SEMICOLON))
		{
			symbol = lex();
		}
		return 1;
	}
	return 0;
}

//变量声明
//<变量说明部分>::=var<变量说明>;{<变量说明>;}
int vardecl()
{
	if (!match(VAR))
	{
		return 0;
	}
	printf("----------------VAR DECLARATION BEGINS--------------\n");
	//变量定义开始
	//<变量说明>::=<标识符>{,<标识符>}:<类型>
	while (match(IDENT))
	{
		vardef();
	}
	return 0;
}

//变量定义
int vardef()
{
	while (match(COMMA))
	{
		if (!match(IDENT))
		{
			error("Var Declaration Error");
			//错误恢复：跳至冒号或者分号
			recovery(2, COLON, SEMICOLON);
			break;
		}
	}
	//多个标识符已经匹配完毕
	if (match(COLON))
	{
		if (match(INT) || match(CHAR))
			{
				//基本类型
				if (!match(SEMICOLON))
				{
					//没有分号
					error("Var Declaration Error");
					//错误恢复：跳至分号
					recovery(1, SEMICOLON);
					match(SEMICOLON);
				}
			}
		else if (match(ARRAY))
			{
				//数组声明
				if (match(LBRACKET) && match(NUM) && match(RBRACKET) &&
					match(OF) && (match(INT) || match(CHAR)))
				{
					if (!match(SEMICOLON))
					{
						//没有分号
						error("Array Declaration Error");
						//错误恢复：跳至分号
						recovery(1, SEMICOLON);
						match(SEMICOLON);
					}
				}
				else
				{
					error("Array Declaration Error");
					//错误恢复：跳至分号
					recovery(1, SEMICOLON);
					match(SEMICOLON);
					return 1;
				}
			}
		else
			{
				//类型错误
				error("Unknown Var Type");
				//错误恢复：跳至分号
				while (!match(SEMICOLON))
				{
					symbol = lex();
				}
				return 1;
			}
	}
	else
	{
		//没有冒号
		error("Var Declaration Error");
		//错误恢复：跳至分号
		recovery(1, SEMICOLON);
		match(SEMICOLON);
	}
	return 0;
}

//过程声明
//<过程说明部分>::=<过程首部><分程序>;{过程说明部分}
int prodecl()
{
	while (symbol == PRO)
	{
		printf("----------------PROCEDURE DECLARATION BEGINS--------------\n");
		//过程声明开始
		prohead();					//过程首部语法分析函数
		program();					//分程序语法分析函数
		if (!match(SEMICOLON))
		{
			error("No Semicolon After Procedure");
			recovery(3, PRO, FUN, BEGIN);
		}
	}
	return 0;
}

//过程首部
//<过程首部>::=procedure<标识符>[<形式参数表>];
int prohead()
{
	match(PRO);
	if (!match(IDENT))
	{
		error("Procedure declaration error");
		//错误恢复：跳至
		recovery(1, SEMICOLON);
	}
	arglist();
	if (!match(SEMICOLON))
	{
		error("No Semicolon");
		//错误恢复：跳至分号
		recovery(5, CONST,VAR,PRO,FUN,BEGIN);
	}
	return 0;
}

//形式参数表
//<形式参数表>::='('<形式参数段>{;<形式参数段>}')'
int arglist()
{
	//因为形式参数表是可选的，所以没有else部分
	if (match(LPARENT))
	{
		//形式参数段语法分析函数
		args();
		while (match(SEMICOLON))
		{
			args();
		}
		//形式参数段均分析完毕
		if (!match(RPARENT))
		{
			error("No Right Parent");
			//错误恢复：
			recovery(1, SEMICOLON);
			match(SEMICOLON);
			return 1;
		}
	}
	return 0;
}

//形式参数段语法分析函数
//<形式参数段>::=[var]<标识符>{,<标识符>}:<基本类型>
int args()
{
	match(VAR);
	if (!match(IDENT))
	{
		error("Wrong Args");
		//错误恢复
		recovery(2, COMMA, COLON);
	}
	while (match(COMMA))
	{
		if (!match(IDENT))
		{
			error("Not a identifier");
			//错误恢复
			recovery(2, COMMA, COLON);
		}
	}
	if (match(COLON))
	{
		if (match(INT))
		{
			return 0;
		}
		else if (match(CHAR))
		{
			return 0;
		}
		else
		{
			//非基本类型
			error("Wrong Type:must be basic type");
			//错误恢复：跳至分号或者右括号
			recovery(2, SEMICOLON, RPARENT);
			return 1;
		}
	}
	else
	{
		error("No Colon Found");
		//错误恢复：跳至分号或者右括号
		recovery(2, SEMICOLON, RPARENT);
		return 1;
	}
}

//函数声明
//<函数说明部分>::=<函数首部><分程序>;{<函数说明部分>}
int fundecl()
{
	while (symbol == FUN)
	{
		printf("----------------FUNCTION DECLARATION BEGINS--------------\n");
		//函数声明开始
		funhead();
		program();
		if (!match(SEMICOLON))
		{
			error("No Semicolon Found After Function");
			recovery(2, FUN, BEGIN);
		}
	}
	return 0;
}

//函数首部
//<函数首部>::=function<标识符>[<形式参数表>]:<基本类型>;
int funhead()
{
	match(FUN);
	if (!match(IDENT))
	{
		error("Function Declaration Error");
		recovery(2, COLON, SEMICOLON);
	}
	arglist();
	if (!match(COLON))
	{
		error("No Colon Found After Args");
		recovery(3, INT, CHAR, SEMICOLON);
	}
	if (!match(INT))
	{
		if (!match(CHAR))
		{
			error("No Return Type Found");
			recovery(1, SEMICOLON);
		}
	}
	if (!match(SEMICOLON))
	{
		error("No Semicolon After Function Head");
		recovery(5,CONST, VAR, PRO, FUN, BEGIN);
	}
	return 0;
}