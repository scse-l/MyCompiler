#include "express.h"
#include "support.h"
#include "global.h"

extern int symbol;

//���ʽ
//<���ʽ>::=[+|-]<��>{<�ӷ������><��>}
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

//��
//<��>::=<����>{<�˷������><����>}
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

//����
//<����>::=<��ʶ��>|<��ʶ��>'['<���ʽ>']'|<�޷�������>|'('<���ʽ>')'|<�����������>
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
			//<��ʶ��>'['<���ʽ>']'
			express(t);
			if (!match(RBRACKET,t))
			{
				error("Missing right bracket");
				recovery(7, TIMES, SLASH, MINUS, PLUS, END, SEMICOLON, THEN);
			}
		}
		else if (match(LPARENT,t))
		{
			//�����������
			//<�����������>::=<��ʶ��>[<ʵ�ڲ�����>]
			express(t);
			while (match(COMMA,t))
			{
				express(t);
			}
			if (!match(RPARENT,t))
			{
				error("Missing Right Parenthesis");
				recovery(7, TIMES, SLASH, MINUS, PLUS, END, SEMICOLON, THEN);
			}
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