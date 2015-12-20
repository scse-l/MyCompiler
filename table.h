#pragma once
/*
	符号表相关定义
*/
#include "lex.h"
#include <map>
#include <vector>
#include <string>

typedef std::vector<struct table_t> itemList;
typedef std::map<std::string, itemList> Table;

//定义符号表项
typedef struct table_t
{
	LexType type;				//标识符类型：const、var、procedure、function、reference
	LexType attribute;			//标识符属性：integer、char、array,对于函数则记录返回值类型							
	int level;					//标识符所属层数
	void *addr;					//地址：对于integer和char记录其值,对于array、function、procedure记录其属性结构所在的地址
	int offset;					//相对地址的偏移量，常量不需要
	Table *table;				//记录当前标识符所在的符号表，如果当前标识符是函数或者过程，则记录它们的子符号表
} tableItem;



/*
	插入操作：将所给的符号及其属性插入到符号表中
	t:待插入的符号表
	name:待插入的符号
	type:待插入符号类型：const、var、procedure、function
	attribute:待插入符号属性：integer、char、array,对于函数则记录返回值类型
	level:待插入符号所属层数
	addr:待插入符号的附加信息地址：对于integer和char记录其值,对于array、function、procedure记录其属性结构所在的地址
*/
tableItem* tableInsert(Table &t, std::string name, LexType type, LexType attribute,
						int level, const void *addr, int lineNo);

void printTable(Table &t);
void printItem(tableItem &i);
void tableClear(Table &t, int lev);