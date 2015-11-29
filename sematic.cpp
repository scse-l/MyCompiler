#include "global.h"
#include "sematic.h"
#include "support.h"

#pragma warning(disable:4996)

extern Table symTable;

//语义分析程序
int sematic(AST root)
{

	//填查符号表
	tableCheck(root, 0);
	//类型检查
	typeCheck(root);

	return 0;
}

//类型检查函数
int typeCheck(AST root)
{
	std::vector<AST_node>::iterator i = root->children->begin();

	for (; i != root->children->end(); i++)
	{
		if ((*i)->ast_type == EXPRESSION)
		{
			//当前节点是表达式
			expCheck(root);
		}
		else if ((*i)->ast_type == CALL)
		{
			//当前节点是过程调用语句;
			proCallCheck(*i);
		}
		else if ((*i)->ast_type == READSTAT)
		{
			//当前语句是读语句
			readStatCheck(*i);
		}
		else if ((*i)->ast_type == PROGRAM)
		{
			typeCheck(*i);
		}

	}
	return 0;
}

int proCallCheck(AST_node t)
{
	procedureTemplet* addr = NULL;
	int n = 0;
	//当前语句是过程调用语句
	//<过程调用语句>::=<标识符>['('<实在参数>{,<实在参数>}')']
	std::vector<AST_node>::iterator i = t->children->begin();
	for (; i != t->children->end(); i++)
	{
		if ((*i)->lex_symbol == IDENT)
		{
			addr = ((procedureTemplet*)(*i)->tableItem->addr);
		}
		else if ((*i)->ast_type == ARGLIST)
		{
			n = argsCheck(*i, addr);
		}
	}
	if (n < addr->args)
	{
		char _s[5];
		char *s = itoa(addr->args, _s, 10);
		std::string msg = s;
		msg = "Too Few Args:Should be " + msg + ", But ";
		s = itoa(n, _s, 10);
		msg = msg + s + " given";
		error(t->lineNo, msg);
	}
	else if (n > addr->args)
	{
		char _s[5];
		char *s = itoa(addr->args, _s, 10);
		std::string msg = s;
		msg = "Too Many Args:Should be " + msg + ", But ";
		s = itoa(n, _s, 10);
		msg = msg + s + " given";
		error(t->lineNo, msg);
	}
	return 0;
}

/*
	比较实参与形参的类型和数量是否一致
	返回实参的个数
*/
int argsCheck(AST_node t, const void *addr)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::vector<int>::iterator type = ((procedureTemplet*)addr)->types->begin();
	int n = 0;

	for (; i != t->children->end() && 
			type != ((procedureTemplet*)addr)->types->end(); i++)
	{
		if ((*i)->ast_type == EXPRESSION)
		{
			if (expCheck(*i) != *type)
			{
				error((*i)->lineNo, "Wrong Arg Type");
			}
			n++;
			type++;
		}
	}
	if (i != t->children->end())
	{
		for (; i != t->children->end(); i++)
		{
			if ((*i)->ast_type == EXPRESSION)
			{
				n++;
			}
		}
	}
	return n;
}

int readStatCheck(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin();

	for (; i != t->children->end(); i++)
	{
		if ((*i)->lex_symbol == IDENT)
		{
			if ((*i)->tableItem->type != VAR || (*i)->tableItem->attribute == ARRAY)
			{
				error((*i)->lineNo, "Can not use a const or an array as arg");
			}
		}
	}
	return 0;
}

/*
	表达式的类型检查函数
*/
LexType expCheck(AST_node expression)
{
	std::vector<AST_node>::iterator i = expression->children->begin();
	LexType l = (LexType)0;

	for (; i != expression->children->end(); i++)
	{
		if ((*i)->ast_type == TERM)
		{
			l = termCheck(*i);
			if (expression->lex_symbol != INT)
			{
				expression->lex_symbol = l;
			}
		}
	}
	return l;
}

/*
	项的类型检查函数
*/
LexType termCheck(AST_node term)
{
	std::vector<AST_node>::iterator i = term->children->begin();
	LexType l = (LexType)0;

	for (; i != term->children->end(); i++)
	{
		if ((*i)->ast_type == FACTOR)
		{
			l = factorCheck(*i);
			if (term->lex_symbol != INT)
			{
				term->lex_symbol = l;
			}
		}
	}
	term->lex_symbol = l;
	return l;
}

/*
	因子的类型检查函数
	<因子>::=<标识符>|<标识符>'['<表达式>']'|<无符号整数>|'('<表达式>')'|<函数调用语句>
*/
LexType factorCheck(AST_node factor)
{
	std::vector<AST_node>::iterator i = factor->children->begin();
	LexType l = (LexType)0;

	for (; i != factor->children->end(); i++)
	{
		if ((*i)->lex_symbol == IDENT)
		{
			if ((*i)->tableItem == NULL)
			{
				//符号表中没有当前标识符
				error(factor->lineNo, "Undefined Identifier " + *(factor->val.ident));
				return (LexType)0;
			}
			if ((*i)->tableItem->type == PRO)
			{
				//当前标志符的类型是过程
				error((*i)->lineNo, "Wrong Factor: Can not be a procedure");
			}
			else if ((*i)->tableItem->type == FUN)
			{
				//<函数调用语句>
				//<函数调用语句>::=<标识符>['('<表达式>{,<表达式>}')']
				functionTemplet *addr = (functionTemplet*)(*i)->tableItem->addr;
				int n = 1;
				//比较形参与实参的类型和数量是否相等
				std::vector<int>::iterator type = addr->types->begin();
				for (; (i + n) != factor->children->end() && 
								type != addr->types->end(); n++, type++)
				{
					if ((*(i + n))->ast_type == EXPRESSION)
					{
						if (expCheck(*(i + n)) != *type)
						{
							error((*i)->lineNo, "Wrong args type");
						}
					}
				}
				if ((i + n) != factor->children->end())
				{
					error((*i)->lineNo, "Too many args");
				}
				else if ((type != addr->types->end()))
				{
					error((*i)->lineNo, "Too few args");
				}
				l = (LexType)(*i)->tableItem->attribute;
			}
			else if ((*i)->tableItem->attribute == ARRAY)
			{
				//当前标识符类型是数组
				if ((*(i + 1))->lex_symbol == LBRACKET)
				{
					//<标识符>'['<表达式>']'
					//数组元素引用
					l = ((arrayTemplet*)(*i)->tableItem->addr)->type;
				}
				else
				{
					error((*i)->lineNo, "Wrong Factor: Can not be an array");
				}
			}
			else
			{
				l = (LexType)(*i)->tableItem->attribute;
			}
		}
		else if ((*i)->lex_symbol == EXPRESSION)
		{
			expCheck(*i);
		}
		else if ((*i)->lex_symbol == NUM)
		{
			l = INT;
		}
		else if ((*i)->lex_symbol == CH)
		{
			l = CHAR;
		}
	}
	factor->lex_symbol = l;
	return l;
}


//符号表填查函数
int tableCheck(AST root, int level)
{
	std::vector<AST_node>::iterator i = root->children->begin();

	if (root->ast_type == CONSTDEF)
	{
		std::string name;
		void *addr = NULL;
		//接下是常量定义
		//<常量定义>::=<标识符>＝<常量>
		for (; i != root->children->end(); i++)
		{
			if ((*i)->lex_symbol == IDENT)
			{
				//当前节点是标识符
				name = *((*i)->val.ident);
			}
			else if ((*i)->lex_symbol == NUM)
			{
				addr = (int *)malloc(sizeof(int));
				*((int *)addr) = (*i)->val.value;
				(*i)->tableItem = tableInsert(symTable, name, CONST, INT, level, addr, (*i)->lineNo);
			}
			else if ((*i)->lex_symbol == CH)
			{
				addr = (char *)malloc(sizeof(char));
				char c = (*i)->val.ident->c_str()[0];
				*((char *)addr) = c;
				(*i)->tableItem = tableInsert(symTable, name, CONST, CHAR, level, addr, (*i)->lineNo);
			}
		}
		return 0;
	}
	else if (root->ast_type == VARDEF)
	{
		int attribute = 0;
		void *addr = NULL;
		std::vector<std::string> names;
		//<变量说明>::=<标识符>{,<标识符>}:<类型>
		for (; i != root->children->end(); i++)
		{
			if ((*i)->lex_symbol == IDENT)
			{
				names.push_back(*((*i)->val.ident));
			}
			else if ((*i)->lex_symbol == ARRAY)
			{
				//当前类型是数组
				//数组类型::=array'['<无符号整数>']'of <基本类型>
				attribute = ARRAY;
				addr = (arrayTemplet *)malloc(sizeof(arrayTemplet));
			}
			else if ((*i)->lex_symbol == NUM)
			{
				if (addr != NULL)
					((arrayTemplet*)addr)->length = (*i)->val.value;
			}
			else if ((*i)->lex_symbol == INT || (*i)->lex_symbol == CHAR)
			{
				if (addr != NULL)
				{
					//当前变量类型是数组
					((arrayTemplet*)addr)->type = (*i)->lex_symbol;
					for (unsigned int j = 0; j < names.size(); j++)
					{
						tableItem* item = tableInsert(symTable, names[j],VAR, ARRAY, level, addr, (*i)->lineNo);
					}
				}
				else
				{
					//当前变量类型是基本类型
					attribute = (*i)->lex_symbol;
					if ((*i)->lex_symbol == INT)
					{
						for (unsigned int j = 0; j < names.size(); j++)
						{
							addr = (int *)malloc(sizeof(int));
							*((int *)addr) = 0;
							tableItem* item = tableInsert(symTable, names[j], VAR, INT, level, addr, (*i)->lineNo);
						}
					}
					else
					{
						for (unsigned int j = 0; j < names.size(); j++)
						{
							addr = (char *)malloc(sizeof(char));
							*((char *)addr) = 't';
							tableItem* item = tableInsert(symTable, names[j], VAR, CHAR, level, addr, (*i)->lineNo);
						}
					}
				}
			}
		}
		return 0;
	}
	else if (root->ast_type == PROHEAD)
	{
		procedureTemplet *addr = (procedureTemplet *)malloc(sizeof(procedureTemplet));
		addr->types = NULL;
		addr->args = 0;
		//当前节点是过程首部
		//<过程首部>::=procedure<标识符>[<形式参数表>];
		for (; i != root->children->end(); i++)
		{
			if ((*i)->lex_symbol == IDENT)
			{
				tableInsert(symTable, *((*i)->val.ident), PRO, 0, level, addr, (*i)->lineNo);
			}
			else if ((*i)->ast_type == ARGLIST)
			{
				addr->types = argsTypes(*i, level+1);
				addr->args = addr->types->size();
			}
		}
		return 0;
	}
	else if (root->ast_type == FUNHEAD)
	{
		functionTemplet *addr = (functionTemplet *)malloc(sizeof(functionTemplet));
		std::string name;
		addr->args = 0;
		addr->types = NULL;
		//当前节点是函数首部
		//<函数首部>::=function<标识符>[<形式参数表>]:<基本类型>;
		for (; i != root->children->end(); i++)
		{
			if ((*i)->lex_symbol == IDENT)
			{
				name = *((*i)->val.ident);

			}
			else if ((*i)->ast_type == ARGLIST)
			{
				addr->types = argsTypes(*i, level+1);
				addr->args = addr->types->size();
			}
			else if ((*i)->lex_symbol == INT || (*i)->lex_symbol == CHAR)
			{
				tableInsert(symTable, name, FUN, (*i)->lex_symbol, level, addr, (*i)->lineNo);
			}
		}
		return 0;
	}
	else if (root->ast_type == PROGRAM)
	{
		//遍历对每个子节点进行符号表填查
		for (; i != root->children->end(); i++)
		{
			tableCheck(*i, level + 1);
		}
		printTable(symTable);
		std::cout<<std::endl<<std::endl;
		tableClear(symTable, level+1);
		return 0;
	}
	else if (root->ast_type == TERMINAL && root->lex_symbol == IDENT)
	{
		//当前节点是标识符
		tableItem* item = tableFind(symTable, *(root->val.ident), level);
		if (item == NULL)
		{
			//符号表中没有当前标识符
			error(root->lineNo,"Undefined Identifier " + *(root->val.ident));
			tableInsert(symTable, *(root->val.ident), 0, 0, level, NULL, root->lineNo);
			return 0;
		}
		root->tableItem = item;
	}
	else
	{
		//遍历对每个子节点进行符号表填查
		for (; i != root->children->end(); i++)
		{
			tableCheck(*i, level);
		}
		return 0;
	}

	return 0;
}

/*
	对形式参数表进行分析
	t:语法树类型为ARGLIST的节点
*/
std::vector<int>* argsTypes(AST_node t, int level)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::vector<int>* types = new std::vector<int>;

	//<形式参数表>:: = '('<形式参数段>{; <形式参数段>}')'
	for (; i != t->children->end(); i++)
	{
		if ((*i)->ast_type == ARGS)
		{
			//当前节点是个参数段
			args(*i, types, level);
		}
	}
	return types;
}

/*
	对形式参数段进行分析
	t:语法树类型为ARGS的节点
*/
void args(AST_node t, std::vector<int> *types, int level)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::vector<std::string> names;
	int cnt = 0;

	//<形式参数段>::=[var]<标识符>{,<标识符>}:<基本类型>
	for (; i != t->children->end(); i++)
	{
		if ((*i)->lex_symbol == IDENT)
		{
			cnt++;
			names.push_back(*(*i)->val.ident);
		}
		else if ((*i)->lex_symbol == INT || (*i)->lex_symbol == CHAR)
		{
			break;
		}
	}
	while (cnt > 0)
	{
		types->push_back((*i)->lex_symbol);
		tableInsert(symTable, names[cnt - 1], ARGS, (*i)->lex_symbol, level, NULL, (*i)->lineNo);
		cnt--;
	}

	return;
}