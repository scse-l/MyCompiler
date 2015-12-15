/*
	根据四元式和符号表信息生成汇编代码
*/
#include "global.h"
#include "asm.h"
#include <map>
#include <cstdio>

#pragma warning(disable:4996)

enum Reg
{
	EAX = 0, EBX, ECX, EDX
};

void getIR(char *Q_ins, std::string &op, std::string &res, std::string &op1, std::string &op2)
{
	int i = 0;
	if (Q_ins[0] == '\n')
		return;
	for (; Q_ins[i] != ',' && Q_ins[i] != '\n'; i++)
	{
		op.append(sizeof(char), Q_ins[i]);
	}
	if (Q_ins[i] == '\n')
		return;
	for (i++; Q_ins[i] != ',' && Q_ins[i] != '\n'; i++)
	{
		res.append(sizeof(char), Q_ins[i]);
	}
	if (Q_ins[i] == '\n')
		return;
	for (i++; Q_ins[i] != ',' && Q_ins[i] != '\n'; i++)
	{
		op1.append(sizeof(char), Q_ins[i]);
	}
	if (Q_ins[i] == '\n')
		return;
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
	pos = "[ebp-" + pos + "]";
	return;
}

/*
	实现了静态链时的地址计算函数
	cnt:表示需要回溯静态链的次数
*/
void calcOffset(tableItem *item, std::string &pos, int cnt, int reguse)
{
	if (cnt == 0)
	{
		//访问的是局部变量
		char _s[5], *s;
		s = itoa(item->offset, _s, 10);
		pos = *(new std::string(s));
		pos = "[ebp-" + pos + "]";
		return;
	}
	else
	{
		//访问的是非局部变量，需要通过静态链回溯
		std::string reg;
		switch (reguse)
		{
		case EAX:reg = "eax"; break;
		case EBX:reg = "ebx"; break;
		case ECX:reg = "ecx"; break;
		case EDX:reg = "edx"; break;
		default:
			break;
		}
		emitASM(new std::string("mov"), &reg, new std::string("[ebp]"));
		cnt--;
		while (cnt > 0)
		{
			emitASM(new std::string("mov"), &reg, new std::string("[" + reg + "]"));
			cnt--;
		}
		char _s[5], *s;
		s = itoa(item->offset, _s, 10);
		pos = *(new std::string(s));
		pos = "["+ reg + "-" + pos + "]";
		return;
	}
}

/*
	生成字符串的标号
*/
std::string *genStringLable()
{
	static int StringLab = 0;
	char _s[5], *s;
	s = itoa(StringLab, _s, 10);
	std::string *ret = new std::string(s);
	*ret = "_string" + *ret;
	StringLab++;
	return ret;
}

/*
	在寄存器中查找target，若target在寄存器中，则将target置为寄存器名字
*/
bool findInRegs(std::string &target, std::string regs[])
{
	int i = 0;
	for (i = 0; i < 4; i++)
	{
		if (regs[i] == target)
		{
			switch (i)
			{
			case EAX:target = "eax"; return true;
			case EBX:target = "ebx"; return true;
			case ECX:target = "ecx"; return true;
			case EDX:target = "edx"; return true;
			default:
				break;
			}
		}
	}
	return false;
}

/*
	在常量池中查找target，若target在寄存器中，则将target置为寄存器名字
*/
bool findInConst(std::string &target, std::map<std::string,int> &constPool)
{
	if(constPool.find(target) == constPool.end())
		return false;
	else
	{
		char s[5];
		target = *new std::string(itoa(constPool.find(target)->second,s,10));
		return true;
	}
}

void moveToRegs(std::string &target, std::string regs[], int &reguse)
{
	if (reguse == EAX)
	{
		emitASM(new std::string("mov"), new std::string("eax"), new std::string(target));
		regs[reguse] = target;
		target = "eax";
	}
	else if (reguse == EBX)
	{
		emitASM(new std::string("mov"), new std::string("ebx"), new std::string(target));
		regs[reguse] = target;
		target = "ebx";
	}
	else if (reguse == ECX)
	{
		emitASM(new std::string("mov"), new std::string("ecx"), new std::string(target));
		regs[reguse] = target;
		target = "ecx";
	}
	else if (reguse == EDX)
	{
		emitASM(new std::string("mov"), new std::string("edx"), new std::string(target));
		regs[reguse] = target;
		target = "edx";
	}
	reguse = (reguse + 1) % 4;
	return;
}

FILE *offsetTable = fopen("offsetTable.txt", "w");
FILE *data = fopen("data.txt", "w");
int insNo = 0;

/*
	根据语法树中的符号表信息和四元式生成相应的汇编代码
*/
int asmMaker(AST_node cur, AST_node parent, int level)
{
	std::vector<AST_node> prosAndFuns;					//用于存放函数和过程的节点以便使用符号表
	Table *table = cur->symTable;
	std::vector<AST_node>::iterator i = cur->children->begin();
	for (; i != cur->children->end() && (*i)->ast_type != PROGRAM; i++)
	{
		//跳过不是program的节点
	}
	if(i != cur->children->end())
		storeNode(*i, prosAndFuns);

	char Q_ins[100];
	int offset = 8;
	std::string *ins = NULL;
	std::string op, res, op1, op2;
	std::map<std::string, int> constPool;		//用于存放常量
	std::map<std::string, std::vector<int>> varPosRecord;		//用于记录变量当前所在的位置
	std::string regs[4];						//用于记录当前寄存器中保存的临时变量名
	int reguse = 0;
	bool saved = false;							//用于记录函数过程调用时寄存器是否被保存

	while (fgets(Q_ins, 1000, stdin) != NULL)
	{
		getIR(Q_ins, op, res, op1, op2);
		insNo++;
//		printf("-------------%s %s %s %s\n", op.c_str(), res.c_str(), op1.c_str(), op2.c_str());
		if (op == "int" || op == "char")
		{
			//int或者char类型的声明
			if (op1 != "")
			{
				//常量声明
				if (op == "int")
				{
					//整型声明
					constPool.insert(std::pair<std::string, int>(res, atoi(op1.c_str())));
				}
				else 
				{
					constPool.insert(std::pair<std::string, int>(res, op1.c_str()[0]));
				}
			}
			else
			{
				//变量声明
				tableItem* item = tableFind(*table, res, parent);
				std::vector<int> *pos = new std::vector<int>();
				pos->push_back(offset);
				item->offset = offset;
				offset += 4;
				//std::cout << res << ":" << item->offset;
				fprintf(offsetTable, "%s:%d\n", res.c_str(), item->offset);
				varPosRecord.insert(std::pair<std::string, std::vector<int>>(res, *pos));
			}
		}
		else if (op == "arraydef")
		{
			//数组声明
			std::vector<int> *pos = new std::vector<int>();
			pos->push_back(offset);
			tableFind(*table, res, parent)->offset = offset;
			fprintf(offsetTable, "%s:%d\n", res.c_str(), offset);
			offset +=  (4 * atoi(op2.c_str()));
			varPosRecord.insert(std::pair<std::string, std::vector<int>>(res, *pos));
		}
		else if (op == "procedure" || op == "function")
		{
			fprintf(offsetTable, "\n%s:\n", res.c_str());
			for (int i = 0; i < prosAndFuns.size(); i++)
			{
				if (*(prosAndFuns[i]->children->at(0)->children->at(1)->val.ident) == res)
				{
					asmMaker(prosAndFuns[i], cur, level+1);
					break;
				}
			}
		}
		else if (op == "+" || op == "-" || op == "*" || op == "/")
		{
			//算术运算
			//res = op1 op op2;
			//若op1不在寄存器中,则将其移至寄存器中
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
				else if (!findInConst(op1, constPool))
				{
					//op1是变量
					pos = new std::string();
					calcOffset(item, *pos, level - item->level, reguse);
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
			else if(!findInConst(op2,constPool))
			{
				//op2是变量
				calcOffset(item, op2, level - item->level, reguse);
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
				calcOffset(item, res, level - item->level, reguse);
				emitASM(new std::string("mov"), new std::string(res), new std::string(op1));
			}
		}
		else if (op == ":=")
		{
			//根据op2来取得res的表示
			if (op2 == "")
			{
				//res := op1
				tableItem *item = tableFind(*table, res, parent);
				calcOffset(item, res, level - item->level, reguse);
			}
			else
			{
				//:=,res,op1, op2，其中res是数组名，含义：res[op2] := op1
				if (op2.c_str()[0] >= '0' && op2.c_str()[0] <= '9')
				{
					//op2是个立即数
					char _s[5],*s;
					int _offset = tableFind(*table, res, parent)->offset + atoi(op2.c_str()) * 4;	//获得数组元素的偏移
					s = itoa(_offset, _s, 10);
					res = *new std::string(s);
					res = "[ebp-" + res + "]";
				}
				else if(!findInConst(op2,constPool))
				{
					//op2是变量
					tableItem *item = tableFind(*table, op2, parent);
					calcOffset(item, op2, level - item->level, reguse);
					moveToRegs(op2, regs, reguse);
					emitASM(new std::string("shl"), &op2, new std::string("2"));
					char _s[5], *s;
					s = itoa(tableFind(*table, res, parent)->offset, _s, 10);
					emitASM(new std::string("add"), &op2, new std::string(s));
					emitASM(new std::string("neg"), &op2, NULL);
					res = "[ebp+" + op2 + "]";
				}
			}
			//取得op1的表示
			if (!findInRegs(op1, regs))
			{
				//op1不在寄存器中
				if (op1.c_str()[0] >= '0' && op1.c_str()[0] <= '9' || findInConst(op1,constPool))
				{
					//op1是立即数
					res = "DWORD PTR " + res;
				}
				else
				{
					//op1是变量
					tableItem *item = tableFind(*table, op1, parent);
					calcOffset(item, op1, level - item->level, reguse);
					moveToRegs(op1, regs, reguse);
				}
			}

			emitASM(new std::string("mov"), &res, &op1);
		}
		else if (op == "array")
		{
			//访问数组元素
			//res := op1[op2]
			if (op2.c_str()[0] >= '0' && op2.c_str()[0] <= '9' || findInConst(op2, constPool))
			{
				//op2是个立即数或者常量
				char _s[5], *s;
				int _offset = tableFind(*table, op1, parent)->offset + atoi(op2.c_str()) * 4;	//获得数组元素的偏移
				s = itoa(_offset, _s, 10);
				op1 = *new std::string(s);
				op1 = "[ebp-" + op1 + "]";
			}
			else
			{
				//op2是变量
				tableItem *item = tableFind(*table, op2, parent);
				calcOffset(item, op2, level - item->level, reguse);
				moveToRegs(op2, regs, reguse);
				emitASM(new std::string("shl"), &op2, new std::string("2"));
				char _s[5], *s;
				s = itoa(tableFind(*table, op1, parent)->offset, _s, 10);
				emitASM(new std::string("add"), &op2, new std::string(s));
				emitASM(new std::string("neg"), &op2, NULL);
				op1 = "[ebp+" + op2 + "]";
			}
			moveToRegs(op1, regs, reguse);
			tableItem *item = tableFind(*table, res, parent);
			if (item == NULL)
			{
				//res是临时变量
				regs[(reguse+3)%4] = res;
			}
			else
			{
				calcOffset(item, res, level - item->level, reguse);
				emitASM(new std::string("mov"), &res, &op1);
			}
		}
		else if (op == "cmp")
		{
			if (!findInRegs(op1, regs))
			{
				//op1不在寄存器中
				moveToRegs(op1, regs, reguse);
			}
			emitASM(new std::string("cmp"), &res, &op1);
		}
		else if (op == "param")
		{
			if (!saved)
			{
				emitASM(new std::string("push"), new std::string("eax"), NULL);
				emitASM(new std::string("push"), new std::string("ebx"), NULL);
				emitASM(new std::string("push"), new std::string("ecx"), NULL);
				emitASM(new std::string("push"), new std::string("edx"), NULL);
				saved = true;
			}
			if (res.c_str()[0] >= '0' && res.c_str()[0] <= '9' || findInConst(res,constPool))
			{
				//res是个立即数

			}
			else
			{
				//res是个标识符
				if (!findInRegs(res, regs) && !findInConst(res,constPool))
				{
					//res不在寄存器中
					tableItem *item = tableFind(*table, res, parent);
					calcOffset(item, res, level - item->level, reguse);
				}
			}
			emitASM(new std::string("push"), &res, NULL);
		}
		else if (op == "call")
		{
			//call,retVal,name,args,
			//retVal = name(arg1, arg2...)
			//其中retVal为存放返回值的变量地址(过程调用则为NULL)
			//name为过程(函数)名，args为参数个数
			op1 = "__" + op1;
			emitASM(new std::string("call"), &op1, NULL);
			if (res != "")
			{
				//函数调用
				tableItem *item = tableFind(*table, res, parent);
				if (item == NULL)
				{
					//res是个临时变量
					regs[EAX] = res;
				}
				else
				{
					//res不是临时变量
					calcOffset(item, res, level - item->level, reguse);
					emitASM(new std::string("mov"), &res, new std::string("eax"));
				}
			}
			int _offset = 4 * atoi(op2.c_str());
			char _s[5], *s;
			s = itoa(_offset, _s, 10);
			emitASM(new std::string("add"), new std::string("esp"), new std::string(s));
			emitASM(new std::string("pop"), new std::string("edx"), NULL);
			emitASM(new std::string("pop"), new std::string("ecx"), NULL);
			emitASM(new std::string("pop"), new std::string("ebx"), NULL);
			emitASM(new std::string("pop"), new std::string("eax"), NULL);
		}
		else if (op == "goto")
		{
			emitASM(new std::string("jmp"), &res, NULL);
		}
		else if (op == "program")
		{
			if (res == "main")
			{
				printf("start:\n");
			}
			//记录参数偏移地址，将活动记录压栈
			char _s[5];
			printf("__%s:\n", res.c_str());
			emitASM(new std::string("push"), new std::string("ebp"), NULL);
			emitASM(new std::string("mov"), new std::string("ebp"), new std::string("esp"));
			emitASM(new std::string("sub"), new std::string("esp"), new std::string(itoa(offset+100, _s, 10)));
		}
		else if (op == "read")
		{
			//read,addr
			//含义：将读入数据存到addr中
			tableItem *item = tableFind(*table, res, parent);
			calcOffset(item, res, level - item->level, reguse);
			switch (reguse)
			{
			case EAX:emitASM(new std::string("lea"), new std::string("eax"), &res); res = "eax"; break;
			case EBX:emitASM(new std::string("lea"), new std::string("ebx"), &res); res = "ebx"; break;
			case ECX:emitASM(new std::string("lea"), new std::string("ecx"), &res); res = "ecx"; break;
			case EDX:emitASM(new std::string("lea"), new std::string("edx"), &res); res = "edx"; break;
			default:
				break;
			}
			emitASM(new std::string("push"), &res, NULL);
			switch (item->attribute)
			{
			case INT:emitASM(new std::string("push"), new std::string("offset _int"), NULL); break;
			case CHAR:emitASM(new std::string("push"), new std::string("offset _char"), NULL); break;
			default:
				break; 
			}
			emitASM(new std::string("call"), new std::string("crt_scanf"), NULL);
		}
		else if (op == "write")
		{
			//write,option,addr
			if (res == "0")
			{
				//option为0时表示输出字符串，addr为要输出的字符串
				std::string *lab = genStringLable();
				fprintf(data, "%s  db  \"%s\",10,0\n", lab->c_str(), op1.c_str());
				emitASM(new std::string("push"), new std::string("offset " + *lab), NULL);
			}
			else if (res == "1")
			{
				//option为1时表示输出字符，addr为要输出的字符所在的地址
				if (op1.c_str()[0] >= '0' && op1.c_str()[0] <= '9' || !findInConst(op1, constPool))
				{
					//op1是变量
					if (!findInRegs(op1, regs))
					{
						//op1不在寄存器中
						tableItem *item = tableFind(*table, op1, parent);
						calcOffset(item, op1, level - item->level, reguse);
						moveToRegs(op1, regs, reguse);
						reguse = (reguse + 3) % 4;
					}
				}
				emitASM(new std::string("push"), &op1, NULL);
				emitASM(new std::string("push"), new std::string("offset _char"), NULL);
			}
			else
			{
				//option为2时表示输出整型数，addr为要输出的值所在的地址
				if (op1.c_str()[0] >= '0' && op1.c_str()[0] <= '9' || !findInConst(op1, constPool))
				{
					//op1是变量
					if (!findInRegs(op1, regs))
					{
						//op1不在寄存器中
						tableItem *item = tableFind(*table, op1, parent);
						calcOffset(item, op1, level - item->level, reguse);
						moveToRegs(op1, regs, reguse);
						reguse = (reguse + 3) % 4;
					}
				}
				emitASM(new std::string("push"), &op1, NULL);
				emitASM(new std::string("push"), new std::string("offset _int"), NULL);
			}
			emitASM(new std::string("call"), new std::string("crt_printf"), NULL);
		}
		else if (op == "return")
		{
			emitASM(new std::string("mov"), new std::string("esp"), new std::string("ebp"));
			emitASM(new std::string("pop"), new std::string("ebp"), NULL);
			emitASM(new std::string("ret"), NULL, NULL);
			printf("\n");
			if (parent == NULL)
			{
				fclose(data);
				fclose(offsetTable);
			}
			return 0;
		}
		else if (op != "")
		{
			emitASM(&op, &res, NULL);
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
		printf(",\t%s", op2->c_str());
	}
	printf("\n");
	return ;
}