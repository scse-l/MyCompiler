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
int statement()
{
	if (symbol == IDENT)
	{
		//赋值语句或者过程调用语句
		//<赋值语句>::=<标识符>:=<表达式>|<函数标识符>:=<表达式>|<标识符>'['<表达式>']':=<表达式>
		//<过程调用语句>::=<标识符>[<实在参数表>]
		//通过标识符的属性来决定调用的语句分析函数？
		match(IDENT);
		if (symbol == BECOMES || symbol == LBRACKET)
		{
			//赋值语句
			stat_assign();
		}
		else
		{
			//过程调用语句
			stat_procedure();
		}
	}
	else if (symbol == IF)
	{
		//条件语句
		//<条件语句>::=if<条件>then<语句>|if<条件>then<语句>else<语句>
		stat_if();
	}
	else if (symbol == DO)
	{
		//当循环语句
		//<当循环语句>::=do<语句>while<条件>
		stat_do();
	}
	else if (symbol == FOR)
	{
		//for循环语句
		//<for循环语句>::=for<标识符>:=<表达式>（downto|to）<表达式>do<语句>//步长为1
		stat_for();
	}
	else if (symbol == BEGIN)
	{
		//复合语句
		//<复合语句>::=begin<语句>{;<语句>}end
		stat_list();
	}
	else if (symbol == READ)
	{
		//读语句
		//<读语句>::=read'('<标识符>{,<标识符>}')'
		stat_read();
	}
	else if (symbol == WRITE)
	{
		//写语句
		//<写语句>::=write'('<字符串>,<表达式>')'|write'('<字符串>')'|write'('<表达式>')'
		stat_write();
	}
	else
	{
		//空语句
	}
	return 0;
}

//复合语句
//<复合语句>::=begin<语句>{;<语句>}end
int stat_list()
{
	match(BEGIN);
	statement();
	while (match(SEMICOLON))
	{
		statement();
	}
	if (!match(END))
	{
		error("Missing Semicolon or \"end\"");
	}
	return 0;
}

//赋值语句
//<赋值语句>::=<标识符>:=<表达式>|<函数标识符>:=<表达式>|<标识符>'['<表达式>']':=<表达式>
int stat_assign()
{
	//由于区别赋值语句与过程调用语句，所以IDENT已经被匹配
	printf("---------------------Assignment--------------\n");	
	if (match(LBRACKET))
	{
		express();
		if (!match(RBRACKET))
		{
			error("Missing right bracket");
			recovery(1, BECOMES);
		}
	}
	match(BECOMES);
	express();
	printf("----------------------End of Assignment---------\n");
	return 0;
}

//过程调用语句
int stat_procedure()
{
	printf("-------------------Procedure Call-----------\n");
	recovery(2, END, SEMICOLON);
	printf("----------------End of Procedure Call---------\n");
	return 0;
}

//for语句
//<for循环语句>::=for<标识符>:=<表达式>（downto|to）<表达式>do<语句>
int stat_for()
{
	match(FOR);
	if (!match(IDENT))
	{
		error("Missing identifier for \"for-statement\"");
		recovery(1, BECOMES);
	}
	if (!match(BECOMES))
	{
		error("Missing assign symbol");
		recovery(5, PLUS, MINUS, IDENT, LPARENT, NUM);
	}
	express();
	if (!match(DOWNTO))
	{
		if (!match(TO))
		{
			error("Missing \"downto\" or \"to\"");
			recovery(5, IDENT, MINUS, PLUS, NUM, LPARENT);
		}
	}
	express();
	if (!match(DO))
	{
		error("Missing do clause");
		//因为可能是空语句，所以不需要错误恢复
		//recovery(IDENT, IF, FOR, BEGIN, READ, WRITE);
	}
	statement();
	return 0;
}

//当循环语句
//<当循环语句>::=do<语句>while<条件>
int stat_do()
{
	match(DO);
	statement();
	if (!match(WHILE))
	{
		error("Missing while clause");
		//错误恢复：跳至条件的first集
		recovery(5, PLUS, MINUS, IDENT, NUM, LPARENT);
	}
	condition();
	return 0;

}

//条件语句
//<条件语句>::=if<条件>then<语句>|if<条件>then<语句>else<语句>
int stat_if()
{
	match(IF);
	condition();
	if (!match(THEN))
	{
		error("Missing \"then\" clause");
		recovery(8, IDENT, IF, DO, BEGIN, READ, WRITE, FOR, SEMICOLON, ELSE);
	}
	statement();
	if (match(ELSE))
	{
		//有else分支
		statement();
	}
	return 0;
}

//条件
//<条件>::=<表达式><关系运算符><表达式>
int condition()
{
	express();
	switch (symbol)
	{
	case LEQ:case LESS:case EQL:
	case GEQ:case GREATER:case NEQ:symbol = lex(); break;
	default:
		error("Missing Relation Operator");
		//错误恢复：跳至<表达式>的first集
		recovery(5,PLUS,MINUS,IDENT,NUM,LPARENT);
		break;
	}
	express();
	return 0;
}

//写语句
//<写语句>::=write'('<字符串>,<表达式>')'|write'('<字符串>')'|write'('<表达式>')'
int stat_write()
{
	match(WRITE);
	if (!match(LPARENT))
	{
		error("Missing left parenthesis");
		recovery(6, STRING, MINUS, PLUS, IDENT, LPARENT, NUM);
	}
	if (match(STRING))
	{
		if (match(COMMA))
		{
			express();
		}
	}
	else
	{
		express();
	}
	if (!match(RPARENT))
	{
		error("Missing right parenthesis");
		recovery(9, SEMICOLON, END, IDENT, IF, DO, BEGIN, READ, WRITE, FOR);
	}
	return 0;
}

//读语句
//<读语句>::=read'('<标识符>{,<标识符>}')'
int stat_read()
{
	match(READ);
	if (!match(LPARENT))
	{
		error("Missing left parenthesis");
		recovery(1, IDENT);
	}
	if (!match(IDENT))
	{
		error("Wrong args");
		recovery(10, COMMA, RPARENT, IDENT, IF, DO, BEGIN, READ, WRITE, FOR, END);
	}
	while (match(COMMA))
	{
		if (!match(IDENT))
		{
			error("Wrong args");
			recovery(2, COMMA, RPARENT);
		}
	}
	if (!match(RPARENT))
	{
		error("Missing right parenthesis");
		recovery(2, END, SEMICOLON);
	}
	return 0;
}