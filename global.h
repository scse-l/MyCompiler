#include <iostream>
#include <string>
//��ǰ���Է��㣬��map������ű�
#include <map>

#ifndef GLOBAL_H
#define GLOBAL_H

//�ṹ��ö������
typedef enum MyEnum
{
	NUM = 256, IDENT, PLUS, MINUS, TIMES, SLASH, BEGIN, NUL, ERR
} symEnum;

typedef struct table_t
{
	std::string name;
	int type;
	int val;
} table;

//��������
int getsym();
#endif