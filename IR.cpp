#include "IR.h"
#include "table.h"
#include "global.h"

#pragma warning(disable:4996)
/*
	为当前语句生成四元式
*/
std::string* IREmit(AST_node root)
{
	std::string *res = NULL;

	if (root->ast_type == ASSIGNSTAT)
	{
		//赋值语句
		res = assignStatEmit(root);
	}
	else if (root->ast_type == IFSTAT)
	{
		//条件语句
		res = ifStatEmit(root);
	}
	else if (root->ast_type == DOSTAT)
	{
		//当循环语句
		res = doStatEmit(root);
	}
	else if (root->ast_type == FORSTAT)
	{
		//for循环语句
		res = forStatEmit(root);
	}
	else if (root->ast_type == WRITESTAT)
	{
		//写语句
		writeStatEmit(root);
	}
	else if (root->ast_type == READSTAT)
	{
		//读语句
		readStatEmit(root);
	}
	else if (root->ast_type == CALL)
	{
		//过程调用语句
		callStatEmit(root);
	}
	else if (root->ast_type == CONSTDEF)
	{
		//常量声明
		constDefEmit(root);
	}
	else if (root->ast_type == VARDEF)
	{
		//变量声明
		varDefEmit(root);
	}
	else if(root->ast_type == PROGRAM || root->ast_type == STATS ||
			root->ast_type == PRODECL || root->ast_type == FUNDECL)
	{
		for (std::vector<AST_node>::iterator i = root->children->begin();
		i != root->children->end(); i++
			)
		{
			res = IREmit(*i);
		}
	}
	return res;
}

//常量声明的四元式生成
std::string* constDefEmit(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::string *res = NULL, *op1 = NULL, *op2 = NULL, op;

	if ((*i)->lex_symbol == IDENT)
	{
		res = (*i)->val.ident;
		op = "const";
		if ((*i)->tableItem->attribute == CHAR)
		{
			op1 = (*i)->val.ident;
			op2 = new std::string("char");
		}
		else
		{
			char _s[5], *s = itoa((*i)->val.value, _s, 10);
			op1 = new std::string(s);
			op2 = new std::string("int");
		}
	}
	i += 2;
	emit(op, res, op1, op2);
	return res;
}

//变量声明的四元式生成
std::string* varDefEmit(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::vector<std::string*> names;

	std::string *res = NULL, *op1 = NULL, *op2 = NULL, op;

	for (; i != t->children->end(); i++)
	{
		if ((*i)->lex_symbol == IDENT)
		{
			names.push_back((*i)->val.ident);
			op = "var";
			if ((*i)->tableItem->attribute == CHAR)
			{
				op1 = (*i)->val.ident;
				op2 = new std::string("char");
			}
			else
			{
				char _s[5], *s = itoa((*i)->val.value, _s, 10);
				op1 = new std::string(s);
				op2 = new std::string("int");
			}
		}

	}
	i += 2;
	emit(op, res, op1, op2);
	return res;
}


//赋值语句的四元式生成
//<赋值语句>::=<标识符>:=<表达式>|<函数标识符>:=<表达式>|<标识符>'['<表达式>']':=<表达式>
std::string* assignStatEmit(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::string *res = NULL, *op1 = NULL, *op2 = NULL;

	for (; i != t->children->end(); i++)
	{
		if ((*i)->lex_symbol == IDENT)
		{
			res = (*i)->val.ident;
		}
		else if ((*i)->lex_symbol == LBRACKET)
		{
			//对数组元素的赋值
			i++;
			op2 = expEmit(*i);
		}
		else if ((*i)->ast_type == EXPRESSION)
		{
			op1 = expEmit(*i);
		}
	}
	emit(":=", res, op1, op2);
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

//当循环的四元式生成
//<当循环语句>::=do<语句>while<条件>
std::string* doStatEmit(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::string *res = NULL, *lable = makeNewLable();

	for (; i != t->children->end(); i++)
	{
		if ((*i)->ast_type == STATS)
		{
			putLable(lable);
			res = IREmit(*i);
		}
		else if ((*i)->ast_type == CONDITION)
		{
			res = conditionEmit(*i);
			emit("if", lable, res, NULL);
		}
	}
	return res;
}

//for循环的四元式生成
//<for循环语句>::=for <标识符>:=<表达式>（downto|to）<表达式>do<语句>//步长为1
std::string* forStatEmit(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::string *conditionLable = makeNewLable(), *endLable = makeNewLable();
	std::string *res = NULL, *op1 = NULL, *op2 = NULL, stepOp, relop;

	for (; i != t->children->end(); i++)
	{
		if ((*i)->lex_symbol == IDENT)
		{
			res = (*i)->val.ident;
			i += 2;
			op1 = emit(":=", res, expEmit(*i),NULL);
		}
		else if ((*i)->lex_symbol == DOWNTO)
		{
			relop = ">";
			stepOp = "-";
		}
		else if ((*i)->lex_symbol == TO)
		{
			relop = "<";
			stepOp = "+";
		}
		else if((*i)->ast_type == EXPRESSION)
		{
			op2 = expEmit(*i);
		}
		else if ((*i)->ast_type == STATS)
		{
			putLable(conditionLable);
			res = makeTempReg();
			emit(relop,res,op1,op2);
			emit("ifFalse", endLable, res, NULL);
			IREmit(*i);
			*op2 = "1";
			emit(stepOp, op1, op2, NULL);
			emit("goto", conditionLable, NULL, NULL);
			putLable(endLable);
		}
	}
	return NULL;
}

//写语句
//<写语句>::=write'('<字符串>,<表达式>')'|write'('<字符串>')'|write'('<表达式>')'
std::string* writeStatEmit(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::string *s = NULL,op = "write";

	for (; i != t->children->end(); i++)
	{
		if ((*i)->lex_symbol == STRING)
		{
			s = new std::string("0");
			emit(op, s, (*i)->val.ident, NULL);
		}
		else if ((*i)->ast_type == EXPRESSION)
		{
			std::string *addr = expEmit(*i);
			if ((*i)->lex_symbol == CHAR)
			{
				s = new std::string("1");
			}
			else
			{
				s = new std::string("2");
			}
			emit(op, s, addr, NULL);
		}
	}
	return NULL;
}

//读语句
//<读语句>::=read'('<标识符>{,<标识符>}')'
std::string* readStatEmit(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::string op = "read";

	for (; i != t->children->end(); i++)
	{
		if ((*i)->lex_symbol == IDENT)
		{
			emit(op, (*i)->val.ident, NULL, NULL);
		}
	}
	return NULL;
}

//过程调用语句
//<过程调用语句>::=<标识符>[<实在参数表>]
//<实在参数表>::='('<实在参数>{,<实在参数>}')
std::string* callStatEmit(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::vector<std::string*> args;

	std::string *name = new std::string(*(*i)->val.ident);
	i++;
	if (i == t->children->end())
	{
		//无参数调用
		std::string *n = new std::string("0");
		return emit("call", NULL, name, n);
	}
	//记录参数
	for (std::vector<AST_node>::iterator j = (*i)->children->begin();
	j != (*i)->children->end(); j++)
	{
		if ((*j)->ast_type == EXPRESSION)
			args.push_back(expEmit(*j));
	}
	//从右至左将参数压栈
	for (std::vector<std::string*>::reverse_iterator j = args.rbegin();
	j != args.rend(); j++)
	{
		emit("param", *j, NULL, NULL);
	}
	std::string *n = NULL;
	char _s[5], *s = NULL;
	s = itoa(args.size(), _s, 10);
	n = new std::string(s);

	return emit("call", NULL, name, n);
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
	std::vector<std::string*> args;

	if ((*i)->lex_symbol == IDENT)
	{
		if ((*i)->tableItem->type == FUN)
		{
			//该因子是函数调用语句
			//<函数调用语句>::=<标识符>[<实在参数表>]
			//<实在参数表>::='('<实在参数>{,<实在参数>}')
			std::string *name = new std::string(*(*i)->val.ident);
			i++;
			if (i == t->children->end())
			{
				//无参数调用
				std::string *n = new std::string("0");
				return emit("call", makeTempReg(), name, n);
			}
			//记录参数
			for (std::vector<AST_node>::iterator j = (*i)->children->begin();
				j != (*i)->children->end(); j++)
			{
				if((*j)->ast_type == EXPRESSION)
					args.push_back(expEmit(*j));
			}
			//从右至左将参数压栈
			for (std::vector<std::string*>::reverse_iterator j = args.rbegin();
					j != args.rend(); j++)
			{
				emit("param", *j, NULL, NULL);
			}
			std::string *n = NULL;
			char _s[5], *s = NULL;
			s = itoa(args.size(), _s, 10);
			n = new std::string(s);
			
			return emit("call", makeTempReg(), name, n);
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

/*
	生成新的标号
*/
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

/*
	放置标号
*/
void putLable(std::string *lable)
{
	printf("%s: \n", lable->c_str());
	return;
}