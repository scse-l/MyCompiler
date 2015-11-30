#include "IR.h"
#include "table.h"
#include "global.h"

#pragma warning(disable:4996)
/*
	生成四元式
*/
std::string* IREmit(AST_node root)
{
	std::vector<AST_node>::iterator i = root->children->begin();
	std::string *res = NULL;

	for (; i != root->children->end(); i++)
	{
		if ((*i)->ast_type == ASSIGNSTAT)
		{
			//赋值语句
			res = assignStatEmit(*i);
		}
		else if ((*i)->ast_type == IFSTAT)
		{
			//条件语句
			res = ifStatEmit(*i);
		}
		else if((*i)->ast_type == PROGRAM || 
				(*i)->ast_type == STATS ||
				(*i)->ast_type == PRODECL || (*i)->ast_type == FUNDECL)
		{
			res = IREmit(*i);
		}
	}
	return res;
}

//赋值语句的四元式生成
std::string* assignStatEmit(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::string *res = NULL, *op1 = NULL;

	for (; i != t->children->end(); i++)
	{
		if ((*i)->lex_symbol == IDENT)
		{
			res = (*i)->val.ident;
		}
		else if ((*i)->ast_type == EXPRESSION)
		{
			op1 = expEmit(*i);
		}
	}
	emit("=", res, op1, NULL);
	return res;
}

//条件语句的四元式生成
//<条件语句>::=if<条件>then<语句>|if<条件>then<语句>else<语句>
std::string* ifStatEmit(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin()+1;
	std::string *res = NULL, *lable = makeNewLable(), *end = NULL;

	for (; i != t->children->end(); i++)
	{
		if ((*i)->ast_type == CONDITION)
		{
			//当前节点是条件
			res = conditionEmit(*i);
			emit("ifFalse", lable, res, NULL);
		}
		else if ((*i)->lex_symbol == THEN)
		{
			i++;
			res = IREmit(*i);
		}
		else if ((*i)->lex_symbol == ELSE)
		{
			i++;
			end = makeNewLable();
			emit("goto", end, NULL, NULL);
			putLable(lable);
			res = IREmit(*i);
			putLable(end);
		}
	}
	if (end == NULL)
	{
		putLable(lable);
	}

	return res;
}

/*
	为条件生成四元式
	t:语法树类型为CONDITION的节点
	<条件>::=<表达式><关系运算符><表达式>
*/
std::string* conditionEmit(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::string op, *op1, *op2;
	
	op1 = expEmit(*i);
	i++;
	if ((*i)->lex_symbol == LESS)
	{
		op = "<";
	}
	else if ((*i)->lex_symbol == LEQ)
	{
		op = "<=";
	}
	else if ((*i)->lex_symbol == NEQ)
	{
		op = "<>";
	}
	else if ((*i)->lex_symbol == GREATER)
	{
		op = ">";
	}
	else if ((*i)->lex_symbol == GEQ)
	{
		op = ">=";
	}
	else
	{
		op = "=";
	}
	i++;
	op2 = expEmit(*i);

	return emit(op, makeTempReg(), op1, op2);
}

/*
	为表达式生成四元式
	t:语法树类型为EXPRESSION的节点
		<表达式>::=[+|-]<项>{<加法运算符><项>}
*/
std::string* expEmit(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::string op = "", *op1 = NULL, *op2 = NULL, *res = NULL;

	if ((*i)->ast_type == TERMINAL)
	{
		//该节点为正负号
		if ((*i)->lex_symbol == MINUS)
		{
			op = "-";
		}
		else
		{
			op = "+";
		}
		i++;
	}
	op1 = termEmit(*i);
	for (i++; i != t->children->end();i++)
	{
		if ((*i)->lex_symbol == MINUS)
		{
			op = "-";
		}
		else if ((*i)->lex_symbol == PLUS)
		{
			op = "+";
		}
		else
		{
			op2 = termEmit(*i);
			res = makeTempReg();
			emit(op, res, op1, op2);
			op1 = res;
			op2 = NULL;
			op = "";
		}
	}
	return op1;
}

/*
	为项生成四元式
	t:语法树类型为FACTOR的节点
	返回值为储存当前项的值的寄存器名
	<项>::=<因子>{<乘法运算符><因子>}
*/
std::string* termEmit(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::string op = "", *op1 = NULL, *op2 = NULL, *res = NULL;

	op1 = factorEmit(*i);
	for (i++; i != t->children->end(); i++)
	{
		if ((*i)->lex_symbol == TIMES)
		{
			op = "*";
		}
		else if ((*i)->lex_symbol == SLASH)
		{
			op = "/";
		}
		else
		{
			op2 = factorEmit(*i);
			res = makeTempReg();
			emit(op, res, op1, op2);
			op1 = res;
			op2 = NULL;
			op = "";
		}
	}
	return op1;
}

/*
	为因子生成四元式
	t:语法树类型为TERM的节点
	返回值为储存当前因子值的寄存器名
	<因子>::=<标识符>|<标识符>'['<表达式>']'|<无符号整数>|'('<表达式>')'|<函数调用语句>
*/
std::string* factorEmit(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin();

	if ((*i)->lex_symbol == IDENT)
	{
		if ((*i)->tableItem->type == FUN)
		{
			//该因子是函数调用语句
			return emit("call", makeTempReg(), NULL, NULL);
		}
		else if ((*i)->tableItem->attribute == ARRAY)
		{
			//该因子是数组元素
			return emit("array", makeTempReg(), (*i)->val.ident, expEmit(*(i + 2)));
		}
		else
		{
			return (*i)->val.ident;
		}

	}
	else if ((*i)->lex_symbol == NUM)
	{
		//该因子是无符号整数
		char _s[5];
		char *s = itoa((*i)->val.value, _s, 10);
		return new std::string(s);
	}
	else
	{
		//该因子是表达式
		return expEmit(*(i + 1));
	}
	return NULL;
}

/*
	生成新的临时变量名
*/
std::string *makeTempReg()
{
	static int regCount = 0;
	regCount++;
	char _message[10];
	char* message = itoa(regCount, _message, 10);
	std::string *s = new std::string(message);
	*s = "temp" + *s;

	return new std::string(*s);
}

/*
	生成四元式指令
*/
std::string* emit(std::string op, std::string *res, std::string *op1, std::string *op2)
{
	printf("%s,%s,%s,%s\n",	 op.c_str(), 
							(res == NULL) ? " " : res->c_str(),
							(op1 == NULL) ? " " : op1->c_str(),
							(op2 == NULL) ? " " : op2->c_str());
	return res;
}
/*
	生成含有标号的四元式指令
*/
std::string* emit(std::string *lable, std::string op, std::string *res, std::string *op1, std::string *op2)
{
	printf("%s: %s,%s,%s,%s\n", lable->c_str(),
								op.c_str(),
								(res == NULL) ? " " : res->c_str(),
								(op1 == NULL) ? " " : op1->c_str(),
								(op2 == NULL) ? " " : op2->c_str());
	return res;
}

std::string *makeNewLable()
{
	static int labCount = 0;
	labCount++;
	char _message[10];
	char* message = itoa(labCount, _message, 10);
	std::string *s = new std::string(message);
	*s = "Lable" + *s;

	return new std::string(*s);
}

void putLable(std::string *lable)
{
	printf("%s: ", lable->c_str());
	return;
}