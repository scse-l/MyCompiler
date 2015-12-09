/*
	根据四元式和符号表信息生成汇编代码
*/
#include "global.h"
#include "asm.h"
#include <map>
#include <cstdio>

#pragma warning(disable:4996)

void getIR(char *Q_ins, std::string &op, std::string &res, std::string &op1, std::string &op2)
{
	int i = 0;
	for (; Q_ins[i] != ','; i++)
	{
		op.append(sizeof(char), Q_ins[i]);
	}
	for (i++; Q_ins[i] != ',' && Q_ins[i] != '\n'; i++)
	{
		res.append(sizeof(char), Q_ins[i]);
	}
	for (i++; Q_ins[i] != ',' && Q_ins[i] != '\n'; i++)
	{
		op1.append(sizeof(char), Q_ins[i]);
	}
	for (i++; Q_ins[i] != '\n'; i++)
	{
		op2.append(sizeof(char), Q_ins[i]);
	}
	return;
}

/*
	将语法树中语法类型为fundef和prodef的节点存起来，以便后续使用相关的符号表
*/
void storeNode(AST_node t, std::vector<AST_node> &prosAndFuns)
{
	if (t == NULL)
	{
		return;
	}

	if (t->ast_type == FUNDEF || t->ast_type == PRODEF)
	{
		prosAndFuns.push_back(t);
		return;
	}

	for (std::vector<AST_node>::iterator i = t->children->begin();
	i != t->children->end(); i++)
	{
		storeNode(*i, prosAndFuns);
	}
	return;
}

void calcOffset(tableItem *item, std::string &pos)
{
	char _s[5], *s;
	s = itoa(item->offset, _s, 10);
	pos = *(new std::string(s));
	pos = "[ebp+" + pos + "]";
	return;
}

int findInRegs(std::string target, std::string regs[])
{
	int i = 0;
	for (i = 0; i < 4; i++)
	{
		if (regs[i] == target)
		{
			return i;
		}
	}
	return i;
}

enum Reg
{
	EAX = 0, EBX, ECX, EDX
};

/*
	根据语法树中的符号表信息和四元式生成相应的汇编代码
*/
int asmMaker(AST_node cur, AST_node parent)
{
	std::vector<AST_node> prosAndFuns;					//用于存放函数和过程的节点以便使用符号表
	Table *table = cur->symTable;

	//当前节点是根节点
	storeNode(cur, prosAndFuns);

	char Q_ins[100];
	int offset = 4;
	std::string *ins = NULL;
	std::string op, res, op1, op2;
	std::map<std::string, int> constPool;		//用于存放常量
	std::map<std::string, std::vector<int>> varPosRecord;		//用于记录变量当前所在的位置
	std::string regs[4];						//用于记录当前寄存器中保存的临时变量名
	int reguse = 0;

	while (fgets(Q_ins, 1000, stdin) != NULL)
	{
		getIR(Q_ins, op, res, op1, op2);
		if (op == "int" || op == "char")
		{
			//int或者char类型的声明
			if (op1 != "")
			{
				//常量声明
				constPool.insert(std::pair<std::string, int>(res, atoi(op1.c_str())));
			}
			else
			{
				//变量声明
				tableItem* item = tableFind(*table, res, parent);
				std::vector<int> *pos = new std::vector<int>();
				pos->push_back(offset);
				item->offset = offset;
				offset += (op == "int") ? 4 : 2;
				varPosRecord.insert(std::pair<std::string, std::vector<int>>(res, *pos));
			}
		}
		else if (op == "arraydef")
		{
			//数组声明
			std::vector<int> *pos = new std::vector<int>();
			pos->push_back(offset);
			offset += (op1 == "int") ? (4 * atoi(op2.c_str())) : (2 * atoi(op2.c_str()));
			varPosRecord.insert(std::pair<std::string, std::vector<int>>(res, *pos));
		}
		else if (op == "procedure")
		{
			printf("-----------------procedure %s begins\n", res.c_str());
			//记录参数偏移地址，将活动记录压栈
			for (int i = 0; i < prosAndFuns.size(); i++)
			{
				if (*(prosAndFuns[i]->children->at(0)->children->at(1)->val.ident) == res)
				{
					asmMaker(prosAndFuns[i], prosAndFuns[i]->parent);
					break;
				}
			}
		}
		else if (op == "function")
		{
			for (int i = 0; i < prosAndFuns.size(); i++)
			{
				if (*(prosAndFuns[i]->children->at(0)->children->at(1)->val.ident) == res)
				{
					asmMaker(prosAndFuns[i], prosAndFuns[i]->parent);
					break;
				}
			}

		}
		else if (op == "+" || op == "-" || op == "*" || op == "/")
		{
			//算术运算
			//res = op1 + op2;
			tableItem* item = tableFind(*table, op1, parent);
			if ('0' <= op1.c_str()[0] && op1.c_str()[0] <= '9' || item != NULL)
			{
				//不是临时变量
				std::string *pos = NULL;
				if ('0' <= op1.c_str()[0] && op1.c_str()[0] <= '9')
				{
					//op1是立即数
					pos = new std::string(op1);
				}
				else
				{
					pos = new std::string();
					calcOffset(item, *pos);
				}

				if (reguse == EAX)
				{
					emitASM(new std::string("mov"), new std::string("eax"), new std::string(*pos));
					regs[reguse] = op1;
					op1 = "eax";
				}else if (reguse == EBX)
				{
					emitASM(new std::string("mov"), new std::string("ebx"), new std::string(*pos));
					regs[reguse] = op1;
					op1 = "ebx";
				}
				else if (reguse == ECX)
				{
					emitASM(new std::string("mov"), new std::string("ecx"), new std::string(*pos));
					regs[reguse] = op1;
					op1 = "ecx";
				}
				else if (reguse == EDX)
				{
					emitASM(new std::string("mov"), new std::string("edx"), new std::string(*pos));
					regs[reguse] = op1;
					op1 = "edx";
				}
				else
				{
					printf("Can not find the temp value %s\n", op1.c_str());
				}
				reguse = (reguse + 1) % 4;
			}
			else
			{
				//op1是临时变量
				int j = 0;
				for (; j < 4; j++)
				{
					if (regs[j] == op1)
						break;
				}
				switch (j)
				{
				case EAX:op1 = "eax"; break;
				case EBX:op1 = "ebx"; break;
				case ECX:op1 = "ecx"; break;
				case EDX:op1 = "edx"; break;
				default:
					break;
				}
			}
			item = tableFind(*table, op2, parent);
			if (item == NULL)
			{
				//op2是临时变量
				int j = 0;
				for (; j < 4; j++)
				{
					if (regs[j] == op2)
						break;
				}
				switch (j)
				{
				case EAX:op2 = "eax"; break;
				case EBX:op2 = "ebx"; break;
				case ECX:op2 = "ecx"; break;
				case EDX:op2 = "edx"; break;
				default:
					break;
				}
			}
			if(op == "+")
			{
				emitASM(new std::string("add"), new std::string(op1), new std::string(op2));
			}
			else if (op == "-")
			{
				emitASM(new std::string("sub"), new std::string(op1), new std::string(op2));
			}
			else if (op == "*")
			{
				emitASM(new std::string("mul"), new std::string(op1), new std::string(op2));
			}
			else if (op == "/")
			{
				emitASM(new std::string("div"), new std::string(op1), new std::string(op2));
			}
			item = tableFind(*table, res, parent);
			if (item == NULL)
			{
				//res是临时变量
				if (op1 == "eax")
				{
					regs[EAX] = res;
				}else if (op1 == "ebx")
				{
					regs[EBX] = res;
				}
				else if (op1 == "ecx")
				{
					regs[ECX] = res;
				}
				else if (op1 == "edx")
				{
					regs[EDX] = res;
				}
			}
			else
			{
				//res不是临时变量
				calcOffset(item, res);
				emitASM(new std::string("mov"), new std::string(res), new std::string(op1));
			}
		}
		else if (op == ":=")
		{
			if (op2 == "")
			{
				//res := op1
				calcOffset(tableFind(*table, res, parent), res);
				int index = findInRegs(op1, regs);
				if (index < 4)
				{
					//op1在寄存器中
					switch (index)
					{
					case EAX:emitASM(new std::string("mov"), &res, new std::string("eax")); break;
					case EBX:emitASM(new std::string("mov"), &res, new std::string("ebx")); break;
					case ECX:emitASM(new std::string("mov"), &res, new std::string("ecx")); break;
					case EDX:emitASM(new std::string("mov"), &res, new std::string("edx")); break;
					default:
						break;
					}
				}
				else
				{
					//op1不在寄存器中
					if (op1.c_str()[0] >= '0' && op1.c_str()[0] <= '9')
					{
						//op1是立即数
					}
					else
					{
						calcOffset(tableFind(*table, op1, parent), op1);
					}
					emitASM(new std::string("mov"), &res, &op1);
				}
			}
			else
			{
				//:=,res,op1, op2，其中res是数组名，含义：res[op2] := op1
				 
			}
		}
		else if (op == "array")
		{
			//访问数组元素

		}
		else if (op == "return")
		{
			printf("----------------------ends\n");
			return 0;
		}
		op.clear();
		res.clear();
		op1.clear();
		op2.clear();
	}
		
	return 0;
}

void emitASM(std::string *ins, std::string *op1, std::string *op2)
{
	printf("%s", ins->c_str());
	if (op1 != NULL)
	{
		printf("\t%s", op1->c_str());
	}
	if (op2 != NULL)
	{
		printf("\t%s", op2->c_str());
	}
	printf("\n");
	return ;
}