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

/*
	引用时的地址计算函数
	cnt:表示需要回溯静态链的次数
*/
void calcOffset(tableItem *item, std::string &pos, int cnt)
{
	if (cnt == 0)
	{
		//访问的是局部变量
		char _s[5], *s;
		s = itoa(item->offset, _s, 10);
		pos = *(new std::string(s));
		emitASM(new std::string("mov"), new std::string("eax"), new std::string("ebp"));
		emitASM(new std::string("sub"), new std::string("eax"), &pos);
		pos = "eax";
		return;
	}
	else
	{
		//访问的是非局部变量，需要通过静态链回溯
		std::string reg = "eax";
		emitASM(new std::string("mov"), &reg, new std::string("[ebp+8]"));
		cnt--;
		while (cnt > 0)
		{
			emitASM(new std::string("mov"), &reg, new std::string("[" + reg + "+8]"));
			cnt--;
		}
		char _s[5], *s;
		s = itoa(item->offset, _s, 10);
		pos = *(new std::string(s));
		emitASM(new std::string("sub"), &reg, &pos);
		pos = reg;
	}
}

/*
	实现了静态链时的地址计算函数
	cnt:表示需要回溯静态链的次数
*/
void calcOffset(tableItem *item, std::string &pos, int cnt, int &reguse)
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
		case EAX:reg = "eax"; reguse++; break;
		case EBX:reg = "ebx"; reguse++; break;
		case ECX:reg = "ecx"; reguse++; break;
		case EDX:reg = "edx"; reguse = EAX; break;
		default:
			break;
		}
		emitASM(new std::string("mov"), &reg, new std::string("[ebp+8]"));
		cnt--;
		while (cnt > 0)
		{
			emitASM(new std::string("mov"), &reg, new std::string("[" + reg + "+8]"));
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

/*
	将target中的内容移至寄存器中
	target:变量的地址
*/
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
			tableItem *item = tableFind(*table, res, parent);
			if(op == "function")
			{
				item->offset = offset;
				offset += 4;
			}
			fprintf(offsetTable, "\n%s:%d\n", res.c_str(),item->offset);
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
					if(item->type == REFERENCE)
					{
						//op1是引用
						emitASM(new std::string("mov"), &op1, new std::string("[" + op1 + "]"));
					}
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
				//op2是临时变量或立即数
				if (op2.c_str()[0] >= '0' && op2.c_str()[0] <= '9')
				{
					//op2是个立即数
					
				}
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
				std::string name = op2;
				calcOffset(item, op2, level - item->level, reguse);
				if(item->type == REFERENCE)
				{
					//op2是引用
					regs[reguse] = name;
					moveToRegs(op2, regs, reguse);
					emitASM(new std::string("mov"), &op2, new std::string("[" + op2 + "]"));
				}else
				{
					op2 = "DWORD PTR " + op2;
				}
			}
			if(op == "+")
			{
				if (op1.c_str()[0] >= '0' && op1.c_str()[0] <= '9')
				{
					//op1是个立即数
					if(op2.c_str()[0] >= '0' && op2.c_str()[0] <= '9')
					{
						//op2也是个立即数
						moveToRegs(op1, regs, reguse);
						emitASM(new std::string("add"), new std::string(op1), new std::string(op2));
					}else
					{
						emitASM(new std::string("add"), new std::string(op2), new std::string(op1));
					}
				}
				else
				{
					emitASM(new std::string("add"), new std::string(op1), new std::string(op2));
				}
			}
			else if (op == "-")
			{
				if (op1.c_str()[0] >= '0' && op1.c_str()[0] <= '9')
				{
					emitASM(new std::string("sub"), new std::string(op2), new std::string(op1));
				}
				else
				{
					emitASM(new std::string("sub"), new std::string(op1), new std::string(op2));
				}

			}
			else if (op == "*" || op == "/")
			{
				//imull S
				//mull S
				//有符号 / 无符号乘法：R[%edx]:R[%eax] <-S * R[%eax]
				//idivl S
				//divl S
				//有符号 / 无符号除法：R[%eax] <-R[%edx]:R[%eax] / S;
				emitASM(new std::string("push"), new std::string("eax"), NULL);
				emitASM(new std::string("push"), new std::string("ebx"), NULL);
				emitASM(new std::string("push"), new std::string("ecx"), NULL);
				emitASM(new std::string("push"), new std::string("edx"), NULL);
				if (op2.c_str()[0] >= '0' && op2.c_str()[0] <= '9' || op2 != "ebx" && op2 != "ecx")
				{
					//将op2移至ecx
					if (op1 == "ecx")
					{
						emitASM(new std::string("mov"), new std::string("eax"), &op1);
					}
					emitASM(new std::string("mov"), new std::string("ecx"), &op2);
					op2 = "ecx";
				}
				if (op1 != "eax")
				{
					emitASM(new std::string("mov"), new std::string("eax"), new std::string(op1));
				}

				if (op == "*")
				{
					emitASM(new std::string("imul"), new std::string(op2), NULL);
				}
				else
				{
					emitASM(new std::string("mov"), new std::string("edx"), new std::string("0"));
					emitASM(new std::string("idiv"), new std::string(op2), NULL);
				}
			}
			if (op == "+" || op == "-")
			{
				item = tableFind(*table, res, parent);
				if (item == NULL)
				{
					//res是临时变量
					if (op1.c_str()[0] > '9' || op1.c_str()[0] < '0')
					{
						if (op1 == "eax")
						{
							regs[EAX] = res;
							//避免结果被下一次计算结果覆盖
							if(reguse == EAX)
							{
								reguse++;
							}
						}else if (op1 == "ebx")
						{
							regs[EBX] = res;
							if (reguse == EBX)
							{
								reguse++;
							}
						}
						else if (op1 == "ecx")
						{
							regs[ECX] = res;
							if (reguse == ECX)
							{
								reguse++;
							}
						}
						else if (op1 == "edx")
						{
							regs[EDX] = res;
							if (reguse == EDX)
							{
								reguse = EAX;
							}
						}
						
					}
					else
					{
						if (op2 == "eax")
						{
							regs[EAX] = res;
							if (reguse == EAX)
							{
								reguse++;
							}
						}
						else if (op2 == "ebx")
						{
							regs[EBX] = res;
							if (reguse == EBX)
							{
								reguse++;
							}
						}
						else if (op2 == "ecx")
						{
							regs[ECX] = res;
							if (reguse == ECX)
							{
								reguse++;
							}
						}
						else if (op2 == "edx")
						{
							regs[EDX] = res;
							if (reguse == EDX)
							{
								reguse = EAX;
							}
						}

					}
				}
				else
				{
					//res不是临时变量
					calcOffset(item, res, level - item->level, reguse);
					emitASM(new std::string("mov"), new std::string(res), new std::string(op1));
				}
			}
			else if(op == "*" || "/")
			{
				item = tableFind(*table, res, parent);
				if (item == NULL)
				{
					//res是临时变量
					switch (reguse)
					{
					case EAX:
						//将eax的旧值留在栈中
						regs[EAX] = res;
						if (reguse == EAX)
						{
							reguse++;
						}
						emitASM(new std::string("pop"), new std::string("edx"), NULL);
						emitASM(new std::string("pop"), new std::string("ecx"), NULL);
						emitASM(new std::string("pop"), new std::string("ebx"), NULL);
						break;
					case EBX:
						emitASM(new std::string("mov"), new std::string("ebx"), new std::string("eax"));
						regs[EBX] = res;
						if (reguse == EBX)
						{
							reguse++;
						}
						emitASM(new std::string("pop"), new std::string("edx"), NULL);
						emitASM(new std::string("pop"), new std::string("ecx"), NULL);
						emitASM(new std::string("pop"), new std::string("eax"), NULL);					//将ebx的旧值存入eax以便覆盖掉
						emitASM(new std::string("pop"), new std::string("eax"), NULL);
						break;
					case ECX:
						emitASM(new std::string("mov"), new std::string("ecx"), new std::string("eax"));
						regs[ECX] = res;
						if (reguse == ECX)
						{
							reguse++;
						}
						emitASM(new std::string("pop"), new std::string("edx"), NULL);
						emitASM(new std::string("pop"), new std::string("ebx"), NULL);					//将ecx的旧值存入ebx以便覆盖掉
						emitASM(new std::string("pop"), new std::string("ebx"), NULL);
						emitASM(new std::string("pop"), new std::string("eax"), NULL);
						break;
					case EDX:
						emitASM(new std::string("mov"), new std::string("edx"), new std::string("eax"));
						regs[EDX] = res;
						if (reguse == EDX)
						{
							reguse = EAX;
						}
						emitASM(new std::string("pop"), new std::string("ecx"), NULL);					//将edx的旧值存入ecx以便覆盖掉
						emitASM(new std::string("pop"), new std::string("ecx"), NULL);
						emitASM(new std::string("pop"), new std::string("ebx"), NULL);
						emitASM(new std::string("pop"), new std::string("eax"), NULL);
						break;
					default:
						break;
					}
				}
				else
				{
					//res不是临时变量
					calcOffset(item, res, level - item->level, reguse);
					emitASM(new std::string("mov"), new std::string(res), new std::string("eax"));
					emitASM(new std::string("pop"), new std::string("edx"), NULL);
					emitASM(new std::string("pop"), new std::string("ecx"), NULL);
					emitASM(new std::string("pop"), new std::string("ebx"), NULL);
					emitASM(new std::string("pop"), new std::string("eax"), NULL);
				}
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
				if(item->type == REFERENCE)
				{
					//当前是引用
					moveToRegs(res, regs, reguse);
					res = "[" + res + "]";
				}
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
					if(item == NULL)
					{
						FILE *fstdout = fopen("CON", "w");
						fprintf(fstdout, "Can not find the temp %s\n", op1.c_str());
						return 1;
					}
					calcOffset(item, op1, level - item->level, reguse);
					moveToRegs(op1, regs, reguse);
					if (item->type == REFERENCE)
					{
						//op1是引用
						emitASM(new std::string("mov"), &op1, new std::string("[" + op1 + "]"));
					}
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
				tableItem *item = tableFind(*table, op1, parent);
				int _offset = item->offset + atoi(op2.c_str()) * 4;	//获得数组元素的偏移 
				s = itoa(-_offset, _s, 10);
				op2 = *new std::string(s);
			}
			else
			{
				//op2是变量
				tableItem *item = tableFind(*table, op2, parent);
				calcOffset(item, op2, level - item->level, reguse);
				moveToRegs(op2, regs, reguse);
				if(item->type == REFERENCE)
				{
					//op2是引用
					emitASM(new std::string("mov"), &op2, new std::string("[" + op2 + "]"));
				}
				emitASM(new std::string("shl"), &op2, new std::string("2"));
				char _s[5], *s;
				item = tableFind(*table, op1, parent);
				s = itoa(item->offset, _s, 10);
				emitASM(new std::string("add"), &op2, new std::string(s));
				emitASM(new std::string("neg"), &op2, NULL);
			}
			tableItem *item = tableFind(*table, op1, parent);
			if (level == item->level)
				{
					//数组为本层的局部变量
					op1 = "[ebp+" + op2 + "]";
				}
			else
				{
					//数组为非局部变量
					int cnt = level - item->level;
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
					emitASM(new std::string("mov"), &reg, new std::string("[ebp+8]"));
					cnt--;
					while (cnt > 0)
					{
						emitASM(new std::string("mov"), &reg, new std::string("[" + reg + "+8]"));
						cnt--;
					}
					op1 = "[" + reg + "+" + op2 + "]";
				}
			moveToRegs(op1, regs, reguse);
			item = tableFind(*table, res, parent);
			if (item == NULL)
			{
				//res是临时变量
				regs[(reguse+3)%4] = res;
			}
			else
			{
				calcOffset(item, res, level - item->level, reguse);
				if(item->type == REFERENCE)
				{
					//res是引用
					moveToRegs(res, regs, reguse);
					res = "[" + res + "]";
				}
				emitASM(new std::string("mov"), &res, &op1);
			}
		}
		else if (op == "cmp")
		{
			if (res.c_str()[0] < '0' || res.c_str()[0] > '9')
			{
				//res不是立即数
				if (!findInConst(res, constPool) && !findInRegs(res, regs))
				{
					//res不在寄存器中且res不在常量池中
					tableItem *item = tableFind(*table, res, parent);
					calcOffset(item, res, level - item->level, reguse);
					moveToRegs(res, regs, reguse);
					if(item->type == REFERENCE)
					{
						//res是引用
						res = "[" + res + "]";
					}
				}
			}
			if (op1.c_str()[0] < '0' || op1.c_str()[0] > '9')
			{
				//op1不是立即数
				if (!findInRegs(op1, regs) && !findInConst(op1,constPool))
				{
					//op1不在寄存器中且op1不在常量池中
					tableItem *item = tableFind(*table, op1, parent);
					calcOffset(item, op1, level - item->level, reguse);
					if(item->type == REFERENCE)
					{
						//op1是引用
						moveToRegs(op1, regs, reguse);
						op1 = "[" + op1 + "]";
					}
				}
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
				if(op1 != "")
				{
					//传引用
					tableItem *item = tableFind(*table, res, parent);
					if(item->type == REFERENCE)
					{
						//res本身就是引用
						calcOffset(item, res, level - item->level, reguse);
					}else
					{
						//计算res的地址
						calcOffset(item, res, level - item->level);
					}
				}else
				{
					//传值
					if (!findInRegs(res, regs) && !findInConst(res,constPool))
					{
						//res不在寄存器中
						tableItem *item = tableFind(*table, res, parent);
						calcOffset(item, res, level - item->level, reguse);
					}					
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
			if (!saved)
			{
				emitASM(new std::string("push"), new std::string("eax"), NULL);
				emitASM(new std::string("push"), new std::string("ebx"), NULL);
				emitASM(new std::string("push"), new std::string("ecx"), NULL);
				emitASM(new std::string("push"), new std::string("edx"), NULL);
				saved = true;
			}

			tableItem *item = tableFind(*table, op1, parent);
			if (item != NULL)
			{
				if (item->level > level-1)
				{
					//上层调用下层
					printf(";静态链\n");
					emitASM(new std::string("push"), new std::string("ebp"), NULL);
				}
				else if (item->level == level-1)
				{
					//同层调用
					printf(";静态链\n");
					emitASM(new std::string("push"), new std::string("[ebp+8]"), NULL);
				}
				else
				{
					//下层调用上层
					printf(";静态链\n");
					int cnt = item->level - level;
					std::string SL = "";
					switch (reguse)
					{
					case EAX:
						emitASM(new std::string("mov"), new std::string("eax"), new std::string("ebp"));
						SL = "eax";
						reguse++;
						break;
					case EBX:
						emitASM(new std::string("mov"), new std::string("ebx"), new std::string("ebp"));
						SL = "ebx";
						reguse++;
						break;
					case ECX:
						emitASM(new std::string("mov"), new std::string("ecx"), new std::string("ebp"));
						SL = "ecx";
						reguse++;
						break;
					case EDX:
						emitASM(new std::string("mov"), new std::string("edx"), new std::string("ebp"));
						SL = "edx";
						reguse = EAX;
						break;
					default:
						break;
					}
					while (cnt > 0)
					{
						emitASM(new std::string("mov"), &SL, new std::string("[" + SL + "+8]"));
					}
					emitASM(new std::string("push"), &SL, NULL);
				}
			}
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
			if(res == "")
			{
				//过程调用才pop出eax，否则eax是返回值
				emitASM(new std::string("pop"), new std::string("eax"), NULL);
			}
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
			if (item->type == REFERENCE)
			{
				//res是引用
				moveToRegs(res, regs, reguse);
				res = "[" + res + "]";
				reguse = (reguse + 3) % 4;
			}
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
			case INT:emitASM(new std::string("push"), new std::string("offset _int_in"), NULL); break;
			case CHAR:emitASM(new std::string("push"), new std::string("offset _char_in"), NULL); break;
			default:
				break; 
			}
			emitASM(new std::string("call"), new std::string("crt_scanf"), NULL);
		}
		else if (op == "write")
		{
			int cnt = 0;

			//因为printf可能会修改寄存器的值，所以在call之前先保存现场，call之后恢复现场
			emitASM(new std::string("push"), new std::string("eax"), NULL);
			emitASM(new std::string("push"), new std::string("ebx"), NULL);
			emitASM(new std::string("push"), new std::string("ecx"), NULL);
			emitASM(new std::string("push"), new std::string("edx"), NULL);

			//write,option,addr
			if (res == "0")
			{
				//option为0时表示输出字符串，addr为要输出的字符串
				std::string *lab = genStringLable();
				fprintf(data, "%s  db  \"%s\",10,0\n", lab->c_str(), op1.c_str());
				emitASM(new std::string("push"), new std::string("offset " + *lab), NULL);
				cnt++;
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
						if(item->type == REFERENCE)
						{
							//op1是引用
							op1 = "[" + op1 + "]";
						}
						reguse = (reguse + 3) % 4;
					}
				}
				emitASM(new std::string("push"), &op1, NULL);
				emitASM(new std::string("push"), new std::string("offset _char"), NULL);
				cnt += 2;
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
						if(item->type == REFERENCE)
						{
							op1 = "[" + op1 + "]";
						}
						reguse = (reguse + 3) % 4;
					}
				}
				emitASM(new std::string("push"), &op1, NULL);
				emitASM(new std::string("push"), new std::string("offset _int"), NULL);
				cnt += 2;
			}

			emitASM(new std::string("call"), new std::string("crt_printf"), NULL);
			//将printf的参数弹栈
			while (cnt > 0)
			{
				emitASM(new std::string("pop"), new std::string("edx"), NULL);
				cnt--;
			}

			//恢复现场
			emitASM(new std::string("pop"), new std::string("edx"), NULL);
			emitASM(new std::string("pop"), new std::string("ecx"), NULL);
			emitASM(new std::string("pop"), new std::string("ebx"), NULL);					//将ebx的旧值存入eax以便覆盖掉
			emitASM(new std::string("pop"), new std::string("eax"), NULL);

		}
		else if (op == "return")
		{
			tableItem *item = tableFind(*table, res, parent);
			if(item != NULL && item->type == FUN)
			{
				if(!findInRegs(res,regs))
				{
					calcOffset(item, res, level - item->level, reguse);
				}
				if(res != "eax")
					emitASM(new std::string("mov"), new std::string("eax"), new std::string(res));
			}
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
		else if (op == "inc" || op == "dec")
		{
			//自增自减运算
			if (!findInConst(res, constPool) && !findInRegs(res, regs))
			{
				//res在内存中
				tableItem *item = tableFind(*table, res, parent);
				calcOffset(item, res, level - item->level, reguse);
				res = "DWORD PTR " + res;
			}
			emitASM(&op, &res, NULL);
		}
		else if (op != "")
		{
			if (res != "" && !findInConst(res, constPool) && !findInRegs(res, regs))
			{
				tableItem *item = tableFind(*table, res, parent);
				if (item != NULL)
				{
					calcOffset(item, res, level - item->level, reguse);
				}
			}
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