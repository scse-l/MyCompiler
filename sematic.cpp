#include "global.h"
#include "sematic.h"
#include "support.h"

#pragma warning(disable:4996)

extern unsigned int errorCount;

//语义分析程序
int sematic(AST root)
{

	//填查符号表
	tableCheck(*(root->symTable), root, 0);

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
		if ((*i)->ast_type == CALL)
		{
			//当前节点是过程调用语句;
			proCallCheck(*i);
		}
		else if ((*i)->ast_type == READSTAT)
		{
			//当前语句是读语句
			readStatCheck(*i);
		}
		else if ((*i)->ast_type == ASSIGNSTAT)
		{
			//当前语句是赋值语句
			assignStatCheck(*i);
		}
		else if ((*i)->ast_type == CONDITION)
		{
			//当前语句是if语句
			conditionCheck(*i);
		}
		else if ((*i)->ast_type == FORSTAT)
		{
			//当前语句是for语句
			forStatCheck(*i);
		}
		else if ((*i)->ast_type == WRITESTAT)
		{
			//当前语句是写语句
			writeStatCheck(*i);
		}
		else
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
			if((*i)->tableItem == NULL)
		//	if (tableFind(*(t->symTable),*((*i)->val.ident),t->parent) == NULL)
			{
				//之前写的是什么意思呢?
				return 1;
			}
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
	std::vector<LexType>::iterator type = ((procedureTemplet*)addr)->types->begin();
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
			if ((*i)->tableItem == NULL)
			{
				return 1;
			}
			if ((*i)->tableItem->type != VAR && (*i)->tableItem->type != REFERENCE
				|| (*i)->tableItem->attribute == ARRAY)
			{
				error((*i)->lineNo, "Can not use a const or an array as arg");
			}
		}
	}
	return 0;
}

int assignStatCheck(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	LexType type;
	if ((*i)->tableItem == NULL)
	{
		return 1;
	}
	type = (*i)->tableItem->attribute;
	if ((*i)->tableItem->type == CONST || (*i)->tableItem->type == PRO)
	{
		error((*i)->lineNo, "Const or Procedure can not be assigned");
		return 1;
	}
	if (type == ARRAY)
	{
		type = ((arrayTemplet*)(*i)->tableItem->addr)->type;
	}
	i = t->children->end() - 1;
	expCheck(*i);
	if (type != (*i)->lex_symbol)
	{
		error((*i)->lineNo, "Type not match");
	}
	return 0;
}

int conditionCheck(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	LexType type = expCheck(*i);

	if (type != expCheck(*(i + 2)))
	{
		error((*i)->lineNo, "Type not match");
	}
	return 0;
}

/*
	对for语句进行类型检查
	<for循环语句>::=for <标识符>:=<表达式>（downto|to）<表达式>do<语句>//步长为1
*/
int forStatCheck(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin()+1;
	LexType type;
	if ((*i)->tableItem == NULL)
	{
		error((*i)->lineNo, "Undefined Identifier");
	}
	type = (*i)->tableItem->attribute;
	i = i + 2;
	if (type != expCheck(*i))
	{
		error((*i)->lineNo, "Type not match");
	}
	i = i + 2;
	if (type != expCheck(*i))
	{
		error((*i)->lineNo, "Type not match");
	}
	i = i + 2;
	typeCheck(*i);
	return 0;
}

int writeStatCheck(AST_node t)
{
	std::vector<AST_node>::iterator i = t->children->begin();

	for (; i != t->children->end(); i++)
	{
		if ((*i)->ast_type == EXPRESSION)
		{
			expCheck(*i);
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
	if (term->lex_symbol != INT)
	{
		term->lex_symbol = l;
	}
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
				return (LexType)1;
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
				functionTemplet *addr = (functionTemplet *)(*i)->tableItem->addr;
				//比较形参与实参的类型和数量是否相等
				int n = 0;
				if(i + 1 != factor->children->end())
				{
					n = argsCheck(*(i + 1), addr);
				}
//				if(n != 0)
				{
					if (n < addr->args)
					{
						char _s[5];
						char *s = itoa(addr->args, _s, 10);
						std::string msg = s;
						msg = "Too Few Args:Should be " + msg + ", But ";
						s = itoa(n, _s, 10);
						msg = msg + s + " given";
						error(factor->lineNo, msg);
					}
					else if (n > addr->args)
					{
						char _s[5];
						char *s = itoa(addr->args, _s, 10);
						std::string msg = s;
						msg = "Too Many Args:Should be " + msg + ", But ";
						s = itoa(n, _s, 10);
						msg = msg + s + " given";
						error(factor->lineNo, msg);
					}
				}
				l = (LexType)(*i)->tableItem->attribute;
			}
			else if ((*i)->tableItem->attribute == ARRAY)
			{
				//当前标识符类型是数组
				if ((i + 1) == factor->children->end())
				{
					error((*i)->lineNo, "Wrong Factor: Can not be an array");
					return (LexType)0;
				}
				if ((*(i + 1))->lex_symbol == LBRACKET)
				{
					//<标识符>'['<表达式>']'
					//数组元素引用
					l = ((arrayTemplet*)(*i)->tableItem->addr)->type;
				}
			}
			else
			{
				l = (LexType)(*i)->tableItem->attribute;
			}
		}
		else if ((*i)->ast_type == EXPRESSION)
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


/*
	符号表填查函数
	symTable: 当前节点所属的符号表
	root: 要进行tableCheck的节点
	level：当前节点所属的层数
*/
int tableCheck(Table &symTable, AST root, int level)
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
				(*(i-2))->tableItem = tableInsert(symTable, name, CONST, INT, level, addr, (*i)->lineNo);
				(*(i - 2))->tableItem->table = &symTable;
			}
			else if ((*i)->lex_symbol == CH)
			{
				addr = (char *)malloc(sizeof(char));
				char c = (*i)->val.ident->c_str()[0];
				*((char *)addr) = c;
				(*(i-2))->tableItem = tableInsert(symTable, name, CONST, CHAR, level, addr, (*i)->lineNo);
				(*(i - 2))->tableItem->table = &symTable;
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
					int space = 0;
					if ((*i)->lex_symbol == INT)
					{
						space = ((arrayTemplet *)addr)->type * 4;
					}
					else
					{
						space = ((arrayTemplet *)addr)->type * 4;
					}
					for (unsigned int j = 0; j < names.size(); j++)
					{
						tableItem* item = tableInsert(symTable, names[j],VAR, ARRAY, level, addr, (*i)->lineNo);
						item->table = &symTable;
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
							item->table = &symTable;
						}
					}
					else
					{
						for (unsigned int j = 0; j < names.size(); j++)
						{
							addr = (char *)malloc(sizeof(char));
							tableItem* item = tableInsert(symTable, names[j], VAR, CHAR, level, addr, (*i)->lineNo);
							item->table = &symTable;
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
		addr->totalSpace = 100;
		//当前节点是过程首部
		//<过程首部>::=procedure<标识符>[<形式参数表>];
		for (; i != root->children->end(); i++)
		{
			if ((*i)->lex_symbol == IDENT)
			{
				(*i)->tableItem = tableInsert(symTable, *((*i)->val.ident), PRO, (LexType)0, level, addr, (*i)->lineNo);
				(*i)->tableItem->table = root->symTable;
			}
			else if ((*i)->ast_type == ARGLIST)
			{
				addr->types = argsTypes(*(root->symTable), *i, level+1);
				addr->args = addr->types->size();
			}
		}
		return 0;
	}
	else if (root->ast_type == FUNHEAD)
	{
		functionTemplet *addr = (functionTemplet *)malloc(sizeof(functionTemplet));
		std::string name;
		AST_node node = NULL;
		addr->args = 0;
		addr->types = NULL;
		addr->totalSpace = 1000;
		//当前节点是函数首部
		//<函数首部>::=function<标识符>[<形式参数表>]:<基本类型>;
		for (; i != root->children->end(); i++)
		{
			if ((*i)->lex_symbol == IDENT)
			{
				name = *((*i)->val.ident);
				node = *i;
			}
			else if ((*i)->ast_type == ARGLIST)
			{
				addr->types = argsTypes(*(root->symTable), *i, level+1);
				addr->args = addr->types->size();
			}
			else if ((*i)->lex_symbol == INT || (*i)->lex_symbol == CHAR)
			{
				node->tableItem = tableInsert(symTable, name, FUN, (*i)->lex_symbol, level, addr, (*i)->lineNo);
				node->tableItem->table = root->symTable;
			}
		}
		return 0;
	}
	else if (root->ast_type == PROGRAM)
	{
		//遍历对每个子节点进行符号表填查
		for (; i != root->children->end(); i++)
		{
			tableCheck(*(root->symTable), *i, level + 1);
		}
		printTable(*(root->symTable));
		std::cout<<std::endl<<std::endl;
		return 0;
	}
	else if (root->ast_type == TERMINAL && root->lex_symbol == IDENT)
	{
		//当前节点是标识符
		tableItem* item = tableFind(symTable, *(root->val.ident), root->parent);
		if (item == NULL)
		{
			//符号表中没有当前标识符
			error(root->lineNo,"Undefined Identifier " + *(root->val.ident));
			//防止重复报错
			tableInsert(symTable, *(root->val.ident), (LexType)0, (LexType)0, level, NULL, root->lineNo)->table = &symTable;
			return 0;
		}
		root->tableItem = item;
	}
	else
	{
		//遍历对每个子节点进行符号表填查
		for (; i != root->children->end(); i++)
		{
			tableCheck(symTable, *i, level);
		}
		return 0;
	}

	return 0;
}

/*
	对形式参数表进行分析
	t:语法树类型为ARGLIST的节点
*/
std::vector<LexType>* argsTypes(Table &symTable, AST_node t, int level)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::vector<LexType>* types = new std::vector<LexType>;
	int offset = 12;

	//<形式参数表>:: = '('<形式参数段>{; <形式参数段>}')'
	for (; i != t->children->end(); i++)
	{
		if ((*i)->ast_type == ARGS)
		{
			//当前节点是个参数段
			args(symTable, *i, types, level, &offset);
		}
	}
	return types;
}

/*
	对形式参数段进行分析
	t:语法树类型为ARGS的节点
*/
void args(Table &symTable, AST_node t, std::vector <LexType> *types, int level, int *startoffset)
{
	std::vector<AST_node>::iterator i = t->children->begin();
	std::vector<std::string> names;
	int cnt = 0;
	bool ref = false;

	//<形式参数段>::=[var]<标识符>{,<标识符>}:<基本类型>
	for (; i != t->children->end(); i++)
	{
		if ((*i)->lex_symbol == VAR)
		{
			ref = true;
		}
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
	int space = 4;
	int n = 0;
	while (cnt > 0)
	{
		types->push_back((*i)->lex_symbol);
		tableItem* item;
		if (ref)
		{
			//传地址
			item = tableInsert(symTable, names[n], REFERENCE, (*i)->lex_symbol, level, NULL, (*i)->lineNo);
		}
		else
		{
			//传值
			item = tableInsert(symTable, names[n], VAR, (*i)->lex_symbol, level, NULL, (*i)->lineNo);
		}
		item->offset = -*startoffset;
		*startoffset += space;
		cnt--;
		n++;
	}

	return;
}