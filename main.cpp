#include <iostream>
#include "global.h"

#pragma warning(disable:4996)

/*
 * ȫ�ֱ�������
*/
//��map����tableʱ�ķ��ű�Ͳ��ҽ������
std::map<std::string, int> symTable;
std::map<std::string, int>::iterator res;

unsigned int numMax = 10;  //�����������󳤶�
char ch;     //���ļ������һ���ַ�
int symbol;  //һ��token������
long long value;  //һ��token��ֵ:���token��NUM,��val����ֵ;���token��IDENT,��val�����ڷ��ű��е�λ��
std::string ident;

int main()
{
	freopen("in.txt", "r", stdin);
	freopen("out.txt", "w", stdout);
	while (symbol != EOF)
	{
		symbol = getsym();
		if (symbol == IDENT)
		{
			printf("Type:Identifier\tName:%s\n", ident.c_str());
		}
		else if(symbol == NUM)
		{
			printf("Type:Number\tVallue:%d\n", value);
		}
		else if (symbol == ERR)
		{
		}
		else
		{
			printf("--------------------------\nWrong Type:%s\n------------------------\n",ident.c_str());
		}
	}
	printf("File Complete!\n");
	return 0;
}