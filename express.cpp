#include "express.h"
#include "support.h"
#include "statement.h"

extern int symbol;

//表达式
//<表达式>::=[+|-]<项>{<加法运算符><项>}
AST_node express(AST_node parent)
{
	AST_node t = makeNode(EXPRESSION, parent);
	printf("----------------EXPRESSION BEGINS--------------\n");
	match(MINUS,t);
	match(PLUS,t);
	term(t);
	while (match(MINUS,t) || match(PLUS,t))
	{
		term(t);
	}
	printf("----------------EXPRESSION END--------------\n");
	return t;
}

//项
//<项>::=<因子>{<乘法运算符><因子>}
AST_node term(AST_node parent)
{
	AST_node t = makeNode(TERM, parent);
	factor(t);
	while (match(TIMES,t) || match(SLASH,t))
	{
		factor(t);
	}
	return t;
}

//因子
//<因子>::=<标识符>|<标识符>'['<表达式>']'|<无符号整数>|'('<表达式>')'|<函数调用语句>
AST_node factor(AST_node parent)
{
	AST_node t = makeNode(FACTOR, parent);
	LexType type = (LexType)0;
	switch (symbol)
	{
	case NUM: match(NUM, t); type = INT; break;
	case IDENT:
		match(IDENT,t);
		if (match(LBRACKET,t))
		{
			//<标识符>'['<表达式>']'
			express(t);
			if (!match(RBRACKET,t))
			{
				error("Missing right bracket");
				recovery(7, TIMES, SLASH, MINUS, PLUS, END, SEMICOLON, THEN);
			}
		}
		else if (symbol == LPARENT)
		{
			//函数调用语句
			//<函数调用语句>::=<标识符>[<实在参数表>]
			arg_list(t);
		}
		break;
	case LPARENT:
		match(LPARENT,t);
		express(t);
		if (!match(RPARENT,t))
		{
			error("Missing right parenthesis");
			recovery(7, TIMES, SLASH, MINUS, PLUS, END, SEMICOLON, THEN);
		}
		break;
	default:
		error("Not a factor");
		recovery(8, TIMES, SLASH, MINUS, PLUS, END, SEMICOLON, THEN, RPARENT);
	}
	return t;
}