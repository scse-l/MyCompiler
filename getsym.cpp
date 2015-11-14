#include <iostream>
#include <string>
#include <map>
#include "global.h"

extern unsigned int numMax;
extern char ch;
extern long long value;
extern std::string ident;
extern std::map<std::string, int> symTable;
extern std::map<std::string, int>::iterator res;

int getsym()
{
	while (true)
	{
		ch = getchar();
		ident.clear();
		ident.append(sizeof(char), ch);
		if (isalpha(ch))
		{
			//��ĸ��ͷ
			ident.clear();          //�����һ��ident���µ�����
			ident.append(sizeof(char), ch);     //����ǰ�ַ������ident��
			while (isalnum(ch = getchar()))
			{
				//�����ǰ����ĸ�������֣�����Ӧ���ǵ�ǰ��ʶ���е�һ����
				ident.append(sizeof(char), ch);  //����ǰ�ַ������ident��
			}
			//��ʶ������
			ungetc(ch, stdin);
			res = symTable.find(ident);
			if (res == symTable.end())
			{
				//û���ҵ���ǰ��ʶ��
				symTable.insert(std::pair<std::string, int>(ident, IDENT));
			}
			else
			{
				if (symTable[ident] != IDENT)
				{
					printf("���Ͳ�ƥ�䣡\n");
				}
			}
			return IDENT;
		}
		else if (isdigit(ch))
		{
			//����
			ident.clear();
			ident.append(sizeof(char), ch);
			while (isdigit(ch = getchar()))
			{
				//������ֵ
				ident.append(sizeof(char), ch);
			}
			ungetc(ch, stdin);
			if (ident.length() > numMax)
			{
				//�������
				value = 0;
				printf("----------------------------\n��ǰ����(%s)���!\n", ident.c_str());
				return ERR;
			}
			else {
				//������ֵ
				value = 0;
				for (std::string::iterator itr = ident.begin(); itr != ident.end();itr++)
				{
					value = value * 10 + (*itr - '0');
				}
			}
			return NUM;
		}
		else if (isspace(ch))
		{
			//���������������пհ׷�
			ident.clear();
			while (isspace(ch))
			{
				ch = getchar();
			}
			ungetc(ch, stdin);
		}
		else if(ch == EOF)
		{
			return EOF;
		}
		else
		{
			//������һ����ʶ���������ֿ�ʼ��λ��
			while (!isalnum(ch) && ch != EOF)
			{
				ch = getchar();
			}
			if (ch == EOF)
			{
				return EOF;
			}
			ungetc(ch, stdin);
			return NUL;
		}
	}


}