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
tableItem* tableFind(Table &t, std::string name, int level)
{
	Table::iterator res = t.find(name);
	if (res == t.end())
	{
		return NULL;
	}
	itemList list = res->second;
	for (int i = 0; i != list.size(); i++)
	{
		if (list.at(i).level == level)
		{
			return &list.at(i);
		}
	}
	return NULL;
}

/*
插入操作：将所给的符号及其属性插入到符号表中
t:待插入的符号表
name:待插入的符号
type,attribute,level,addr:待插入符号的属性
*/
tableItem* tableInsert(Table &t, std::string name, int type, int attribute, int level, const void *addr, int lineNo)
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
		std::cout << i->first << ", ";
		for (itemList::iterator j = i->second.begin(); j != i->second.end(); j++)
		{
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
		case NUM:
			std::cout << "num, " << *((int *)(i.addr)) << ", " << i.level << std::endl;
			break;
		case CH:
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
		printf("\n");
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
		printf("\n");
		break;
	default:
		printf("\n");
		break;
	}
	return;
}