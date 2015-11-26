#include "syntax.h"
#include "program.h"
#include "support.h"

extern int symbol;

//语法分析程序
int syntax(AST root)
{
	//<程序>::=<分程序>.
	while (symbol != EOF)
	{
		program(root);
		if (!match(PERIOD, root))
		{
			error("Missing Period!Program is not completed");
			recovery(1, EOF);
		}
	}
	return 0;
}
