#include "express.h"
#include "support.h"
#include "global.h"

extern int symbol;

//表达式
//<表达式>::=[+|-]<项>{<加法运算符><项>}
int express()
{
	printf("----------------EXPRESSION BEGINS--------------\n");
	match(MINUS);
	match(PLUS);
	term();
	while (match(MINUS) || match(PLUS))
	{
		term();
	}
	printf("----------------EXPRESSION END--------------\n");
	return 0;
}

//项
//<项>::=<因子>{<乘法运算符><因子>}
int term()
{
	factor();
	while (match(TIMES) || match(SLASH))
	{
		factor();
	}
	return 0;
}

//因子
//<因子>::=<标识符>|<标识符>'['<表达式>']'|<无符号整数>|'('<表达式>')'|<函数调用语句>
int factor()
{
	switch (symbol)
	{
	case NUM: match(NUM); break;
	case IDENT:
		match(IDENT);
		if (match(LBRACKET))
		{
			//<标识符>'['<表达式>']'
			express();
			if (!match(RBRACKET))
			{
				error("Missing right bracket");
				recovery(7, TIMES, SLASH, MINUS, PLUS, END, SEMICOLON, THEN);
			}
		}
		else if (match(LPARENT))
		{
			//函数调用语句
			//<函数调用语句>::=<标识符>[<实在参数表>]
			express();
			while (match(COMMA))
			{
				express();
			}
			if (!match(RPARENT))
			{
				error("Missing Right Parenthesis");
				recovery(7, TIMES, SLASH, MINUS, PLUS, END, SEMICOLON, THEN);
			}
		}
		break;
	case LPARENT:
		match(LPARENT);
		express();
		if (!match(RPARENT))
		{
			error("Missing right parenthesis");
			recovery(7, TIMES, SLASH, MINUS, PLUS, END, SEMICOLON, THEN);
		}
		break;
	default:
		error("Not a factor");
		recovery(8, TIMES, SLASH, MINUS, PLUS, END, SEMICOLON, THEN, RPARENT);
	}
	return 0;
}