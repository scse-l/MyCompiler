#include "express.h"
#include "support.h"
#include "global.h"

extern int symbol;

//���ʽ
//<���ʽ>::=[+|-]<��>{<�ӷ������><��>}
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

//��
//<��>::=<����>{<�˷������><����>}
int term()
{
	factor();
	while (match(TIMES) || match(SLASH))
	{
		factor();
	}
	return 0;
}

//����
//<����>::=<��ʶ��>|<��ʶ��>'['<���ʽ>']'|<�޷�������>|'('<���ʽ>')'|<�����������>
int factor()
{
	switch (symbol)
	{
	case NUM: match(NUM); break;
	case IDENT:
		match(IDENT);
		if (match(LBRACKET))
		{
			//<��ʶ��>'['<���ʽ>']'
			express();
			if (!match(RBRACKET))
			{
				error("Missing right bracket");
				recovery(7, TIMES, SLASH, MINUS, PLUS, END, SEMICOLON, THEN);
			}
		}
		else if (match(LPARENT))
		{
			//�����������
			//<�����������>::=<��ʶ��>[<ʵ�ڲ�����>]
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