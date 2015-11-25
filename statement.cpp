/*
	所有语句的语法分析程序
	<语句>::=<赋值语句>|<条件语句>|<当循环语句>|<过程调用语句>|<复合语句>|
				<读语句>|<写语句>|<for循环语句>|<空>
*/

#include "global.h"
#include "support.h"
#include "statement.h"
#include "express.h"

extern int symbol;
extern int lex();

//语句
AST_node statement(AST_node parent)
{
	AST_node t = makeNode(STATEMENT, parent);
	if (symbol == IDENT)
	{
		//赋值语句或者过程调用语句
		//<赋值语句>::=<标识符>:=<表达式>|<函数标识符>:=<表达式>|<标识符>'['<表达式>']':=<表达式>
		//<过程调用语句>::=<标识符>[<实在参数表>]
		match(IDENT,t);
		if (symbol == BECOMES || symbol == LBRACKET)
		{
			//赋值语句
			stat_assign(t);
		}
		else if (symbol == LPARENT || symbol == SEMICOLON)
		{
			//过程调用语句
			stat_procedure(t);
		}
		else
		{
			error("Unknown Statement!");
			recovery(2, SEMICOLON, END);
		}
	}
	else if (symbol == IF)
	{
		//条件语句
		//<条件语句>::=if<条件>then<语句>|if<条件>then<语句>else<语句>
		stat_if(t);
	}
	else if (symbol == DO)
	{
		//当循环语句
		//<当循环语句>::=do<语句>while<条件>
		stat_do(t);
	}
	else if (symbol == FOR)
	{
		//for循环语句
		//<for循环语句>::=for<标识符>:=<表达式>（downto|to）<表达式>do<语句>//步长为1
		stat_for(t);
	}
	else if (symbol == BEGIN)
	{
		//复合语句
		//<复合语句>::=begin<语句>{;<语句>}end
		stat_list(t);
	}
	else if (symbol == READ)
	{
		//读语句
		//<读语句>::=read'('<标识符>{,<标识符>}')'
		stat_read(t);
	}
	else if (symbol == WRITE)
	{
		//写语句
		//<写语句>::=write'('<字符串>,<表达式>')'|write'('<字符串>')'|write'('<表达式>')'
		stat_write(t);
	}
	else
	{
		//空语句
		printf("----------------Empty Statement--------------\n");
	}
	return 0;
}

//复合语句
//<复合语句>::=begin<语句>{;<语句>}end
AST_node stat_list(AST_node t)
{
	printf("----------------Statement List--------------\n");
	match(BEGIN,t);
	statement(t);
	while (match(SEMICOLON,t))
	{
		statement(t);
	}
	if (!match(END,t))
	{
		error("Missing Semicolon or \"end\"");
	}
	printf("----------------End of Statement List--------------\n");
	return t;
}

//赋值语句
//<赋值语句>::=<标识符>:=<表达式>|<函数标识符>:=<表达式>|<标识符>'['<表达式>']':=<表达式>
AST_node stat_assign(AST_node t)
{
	//由于区别赋值语句与过程调用语句，所以IDENT已经被匹配
	printf("---------------------Assignment--------------\n");	
	if (match(LBRACKET, t))
	{
		express(t);
		if (!match(RBRACKET,t))
		{
			error("Missing right bracket");
			recovery(1, BECOMES);
		}
	}
	match(BECOMES,t);
	express(t);
	printf("----------------------End of Assignment---------\n");
	return t;
}

//过程调用语句
//<过程调用语句>::=<标识符>[<实在参数表>]
AST_node stat_procedure(AST_node t)
{
	printf("-------------------Procedure Call-----------\n");
	if (symbol == LPARENT)
	{
		//参数表
		arg_list(t);
	}
	printf("----------------End of Procedure Call---------\n");
	return t;
}

//实在参数表
//<实在参数表>::='('<实在参数>{,<实在参数>}')'
AST_node arg_list(AST_node parent)
{
	AST_node t = makeNode(ARGLIST, parent);
	match(LPARENT,t);
	express(t);
	while (match(COMMA,t))
	{
		//实在参数
		//<实在参数>::=<表达式>
		express(t);
	}
	if (!match(RPARENT,t))
	{
		error("Missing Right Parent");
		recovery(3, RPARENT, SEMICOLON, END);
	}
	return t;
}

//for语句
//<for循环语句>::=for<标识符>:=<表达式>（downto|to）<表达式>do<语句>
AST_node stat_for(AST_node t)
{
	printf("----------------For Loop--------------\n");
	match(FOR,t);
	if (!match(IDENT,t))
	{
		error("Missing identifier for \"for-statement\"");
		recovery(1, BECOMES);
	}
	if (!match(BECOMES,t))
	{
		error("Missing assign symbol");
		recovery(5, PLUS, MINUS, IDENT, LPARENT, NUM);
	}
	express(t);
	if (!match(DOWNTO,t))
	{
		if (!match(TO,t))
		{
			error("Missing \"downto\" or \"to\"");
			recovery(5, IDENT, MINUS, PLUS, NUM, LPARENT);
		}
	}
	express(t);
	if (!match(DO,t))
	{
		error("Missing do clause");
		//因为可能是空语句，所以不需要错误恢复
		//recovery(IDENT, IF, FOR, BEGIN, READ, WRITE);
	}
	statement(t);
	printf("----------------End of For Loop--------------\n");
	return t;
}

//当循环语句
//<当循环语句>::=do<语句>while<条件>
AST_node stat_do(AST_node t)
{
	printf("----------------Do While Loop--------------\n");
	match(DO,t);
	statement(t);
	if (!match(WHILE,t))
	{
		error("Missing while clause");
		//错误恢复：跳至条件的first集
		recovery(5, PLUS, MINUS, IDENT, NUM, LPARENT);
	}
	condition(t);
	printf("----------------End of Do While Loop--------------\n");
	return t;
}

//条件语句
//<条件语句>::=if<条件>then<语句>|if<条件>then<语句>else<语句>
AST_node stat_if(AST_node t)
{
	printf("----------------If Statement--------------\n");
	match(IF,t);
	condition(t);
	if (!match(THEN,t))
	{
		error("Missing \"then\" clause");
		recovery(8, IDENT, IF, DO, BEGIN, READ, WRITE, FOR, SEMICOLON, ELSE);
	}
	statement(t);
	if (match(ELSE,t))
	{
		//有else分支
		statement(t);
	}
	printf("----------------End of If--------------\n");
	return t;
}

//条件
//<条件>::=<表达式><关系运算符><表达式>
AST_node condition(AST_node parent)
{
	AST_node t = makeNode(CONDITION, parent);
	express(t);
	if (match(LEQ, t))
	{
	}
	else if (match(LESS, t))
	{
	}
	else if (match(EQL, t))
	{
	}
	else if (match(GEQ, t))
	{
	}
	else if (match(GREATER, t))
	{
	}
	else if (match(NEQ, t))
	{
	}
	else
	{
		error("Missing Relation Operator");
		//错误恢复：跳至<表达式>的first集
		recovery(5, PLUS, MINUS, IDENT, NUM, LPARENT);
	}
	express(t);
	return t;
}

//写语句
//<写语句>::=write'('<字符串>,<表达式>')'|write'('<字符串>')'|write'('<表达式>')'
AST_node stat_write(AST_node t)
{
	printf("----------------Write Statement--------------\n");
	match(WRITE,t);
	if (!match(LPARENT,t))
	{
		error("Missing left parenthesis");
		recovery(6, STRING, MINUS, PLUS, IDENT, LPARENT, NUM);
	}
	if (match(STRING,t))
	{
		if (match(COMMA,t))
		{
			express(t);
		}
	}
	else
	{
		express(t);
	}
	if (!match(RPARENT,t))
	{
		error("Missing right parenthesis");
		recovery(9, SEMICOLON, END, IDENT, IF, DO, BEGIN, READ, WRITE, FOR);
	}
	printf("----------------End of Write--------------\n");
	return t;
}

//读语句
//<读语句>::=read'('<标识符>{,<标识符>}')'
AST_node stat_read(AST_node t)
{
	printf("----------------Read Statement--------------\n");
	match(READ,t);
	if (!match(LPARENT,t))
	{
		error("Missing left parenthesis");
		recovery(1, IDENT);
	}
	if (!match(IDENT,t))
	{
		error("Wrong args");
		recovery(10, COMMA, RPARENT, IDENT, IF, DO, BEGIN, READ, WRITE, FOR, END);
	}
	while (match(COMMA,t))
	{
		if (!match(IDENT,t))
		{
			error("Wrong args");
			recovery(2, COMMA, RPARENT);
		}
	}
	if (!match(RPARENT,t))
	{
		error("Missing right parenthesis");
		recovery(2, END, SEMICOLON);
	}
	printf("----------------End of Read--------------\n");
	return t;
}