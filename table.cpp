/*
	符号表相关操作的实现
*/
#include "table.h"
#include "support.h"
#include "global.h"

/*
	查找操作：若在所给符号表中找到则返回指向该符号表项的指针，否则返回NULL
	t:待查找的符号表
	name:欲查找的符号
	level:欲查找符号所属层数
*/
tableItem* tableFind(Table &t, std::string name, AST_node parent)
{
	Table::iterator res = t.find(name);
	tableItem *ret = NULL;

	if (res == t.end())
	{
		if (parent == NULL || parent->symTable == NULL)
		{
			return NULL;
		}
		return tableFind(*(parent->symTable), name, parent->parent);
	}
	itemList* list = &(res->second);
	ret = &(list->at(0));
	return ret;
}

/*
	插入操作：将所给的符号及其属性插入到符号表中
	t:待插入的符号表
	name:待插入的符号
	type:待插入符号类型：const、var、procedure、function
	attribute:待插入符号属性：integer、char、array,对于函数则记录返回值类型
	level:待插入符号所属层数
	addr:待插入符号的附加信息地址：对于integer和char记录其值,对于array、function、procedure记录其属性结构所在的地址
*/
tableItem* tableInsert(Table &t, std::string name, LexType type, LexType attribute, int level, const void *addr, int lineNo)
{
	Table::iterator res = t.find(name);
	itemList* List = NULL;

	if (res != t.end())
	{
		//符号表中已有同名符号
		List = &(res->second);
		for (itemList::iterator i = List->begin(); i != List->end(); i++)
		{
			if (i->level == level)
			{
				//当前层已有同名符号
				error(lineNo, "Dulplicate Declaration!");
				return NULL;
			}
		}
		//当前层没有同名符号
		tableItem* item = (tableItem*)malloc(sizeof(tableItem));
		item->type = type;
		item->attribute = attribute;
		item->level = level;
		item->addr = (void *)addr;
		item->offset = 0;
		List->push_back(*item);
		return item;
	}
	else
	{
		//当前符号表中没有同名符号
		List = new itemList;
		tableItem* item = (tableItem*)malloc(sizeof(tableItem));
		item->type = type;
		item->attribute = attribute;
		item->level = level;
		item->addr = (void *)addr;
		item->offset = 0;
		List->push_back(*item);
		t.insert(std::pair<std::string, itemList>::pair(name, *List));
		return item;
	}
	return NULL;
}

void printTable(Table &t)
{
	std::map<std::string, itemList>::iterator i = t.begin();

	for (; i != t.end(); i++)
	{
		if (i->second.empty())
		{
			continue;
		}
		for (itemList::iterator j = i->second.begin(); j != i->second.end(); j++)
		{
			std::cout << i->first << ", ";
			printItem(*j);
		}
	}
	return;
}

void printItem(tableItem &i)
{
	int cnt = 0;
	switch (i.type)
	{
	case CONST:
		std::cout << "const, ";
		switch (i.attribute)
		{
		case INT:
			std::cout << "int, " << *((int *)(i.addr)) << ", " << i.level << std::endl;
			break;
		case CHAR:
			std::cout << "char, " << *((char *)i.addr) << ", " << i.level << std::endl;
			break;
		default:
			break;
		}
		break;
	case VAR:
		std::cout << "var, ";
		if (i.attribute == INT)
		{
			std::cout << "int, ," << i.level << std::endl;
		}
		else if (i.attribute == CHAR)
		{
			std::cout << "char, ," << i.level << std::endl;
		}
		else
		{
			std::cout << "array, ," << i.level << std::endl;
		}
		break;
	case PRO:
		cnt = ((procedureTemplet*)i.addr)->args;
		std::cout << "procedure,  , " << "," << i.level << "," << cnt << ", ";
		for (int n = 0; n < cnt; n++)
		{
			std::cout << ((procedureTemplet*)i.addr)->types->at(n) << ", ";
		}
		std::cout<<", "<<std::endl;
		break;
	case FUN:
		cnt = ((functionTemplet*)i.addr)->args;
		std::cout << "function, ";
		if (i.attribute == INT)
		{
			std::cout << "int,," << i.level << ",";
		}
		else if (i.attribute == CHAR)
		{
			std::cout << "char,," << i.level << ",";
		}
		std::cout << cnt << ",";
		for (int n = 0; n < cnt; n++)
		{
			std::cout << ((functionTemplet*)i.addr)->types->at(n) << ", ";
		}
		std::cout << std::endl;
		break;
	case ARGS:
		std::cout << "arg, ";
		if (i.attribute == INT)
		{
			std::cout << "int, ," << i.level << std::endl;
		}
		else if (i.attribute == CHAR)
		{
			std::cout << "char, ," << i.level << std::endl;
		}
		break;

	default:
		std::cout << std::endl;
		break;
	}
	return;
}

void tableClear(Table &t, int lev)
{
	Table::iterator i = t.begin();

	for (; i != t.end(); i++)
	{
		itemList* List = &(i->second);
		for (itemList::iterator j = List->begin(); j != List->end(); j++)
		{
			if (j->level == lev)
			{
				List->erase(j);
				if (List->empty())
				{
					break;
				}
				else
				{
					j = List->begin();
				}
			}
		}
	}
	return;
}