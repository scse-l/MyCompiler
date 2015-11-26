#include "global.h"
#include "sematic.h"
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

	return 0;
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
				(*i)->tableItem = tableInsert(symTable, name, CONST, NUM, level, addr, (*i)->lineNo);
			}
			else if ((*i)->lex_symbol == CH)
			{
				addr = (char *)malloc(sizeof(char));
				char c = (*i)->val.ident->c_str()[0];
				*((char *)addr) = c;
				(*i)->tableItem = tableInsert(symTable, name, CONST, CH, level, addr, (*i)->lineNo);
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
							tableItem* item = tableInsert(symTable, names[j], VAR, attribute, level, addr, (*i)->lineNo);
						}
					}
					else
					{
						for (unsigned int j = 0; j < names.size(); j++)
						{
							addr = (char *)malloc(sizeof(char));
							*((char *)addr) = 't';
							tableItem* item = tableInsert(symTable, names[j], VAR, attribute, level, addr, (*i)->lineNo);
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
对参数表进行分析
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
	对参数段进行分析
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