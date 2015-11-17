/*
	 声明部分
	 包括：常量说明部分、变量说明部分、过程说明部分、函数说明部分
*/

#include "global.h"
#include "support.h"
#include "lex.h"

extern int symbol;
extern std::string ident;
extern std::map<std::string, int> symTable;

int declaration()
{
	if (match(CONST))
	{
		constdecl();
	}
	else if (match(VAR))
	{
		vardecl();
	}
	else if (match(PRO))
	{
		prodecl();
	}
	else if (match(FUN))
	{
		fundecl();
	}
	else
	{
		error("Not a declaration");
		return 1;
	}
	return 0;
}

//常量声明
int constdecl()
{
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
int constdef()
{
	if (match(IDENT) && match(EQL))
	{
		if (match(CHAR) || match(NUM))
		{
			//<标识符>＝<字符> || <标识符>＝<无符号整数>

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
int vardecl()
{
	if (match(VAR))
	{
		//变量定义开始
		while (match(IDENT))
		{
			vardef();
		}
	}
	else
	{
		error("Var Declaration Error");
		//错误恢复：跳至分号或者过程声明语句或者函数声明语句
		recovery(3, SEMICOLON, PRO, FUN);
		return 1;
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
			//错误恢复：跳至冒号
			recovery(1, COLON);
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
					}
				}
				else
				{
					error("Array Declaration Error");
					//错误恢复：跳至分号
					recovery(1, SEMICOLON);
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
	}
}

int prodecl()
{

}

int fundecl()
{


}