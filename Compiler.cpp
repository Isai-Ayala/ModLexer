#include <iostream>
#include <fstream>
#include <cstring>
#include <cstddef>
#include <string>
#include <stack>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include "Lexer.h"
#include "Compiler.h"
#include "conversion.h"
#include <sstream>


enum COMMANDS {HALT, PRTCR, PRTC, PRTI, PRTF, PRTD, PRTS, PRTAC, PRTAI, PRTAF, PRTAD, PRTAS, PUSHC, PUSHI, PUSHF, PUSHD, PUSHS, PUSHAC, PUSHAI, PUSHAF, PUSHAD, PUSHAS, PUSHKC, PUSHKI, PUSHKF, PUSHKD, PUSHKS, POPC, POPI, POPF, POPD, POPS, POPX, POPAC, POPAI, POPAF, POPAD, POPAS, RDC, RDI, RDF, RDD, RDS, RDAC, RDAI, RDAF, RDAD, RDAS, JMP, JMPEQ, JMPNE, JMPGT, JMPGE, JMPLT, JMPLE, STX, STKX, INC, DEC, ADD, SUB, MUL, DIV, MOD, CMP, PRTM, MOVY, POPY};

bool isFor = false;
std::string tempToken;
std::ofstream fout;
char fileBuf[10000];
int actualDir = 0;
bool sError = false;
char tempchar;
int tempint;
char gType;

struct var
{
	unsigned char type;
	int dir;
	std::string name;
	bool isArray;
	int size;
	int i;
	float f;
	char c;
	double d;
	std::string s;
	int stringSize;
};

struct tag
{
	std::string name;
	int dir;
};

std::vector<tag> tags;
std::vector<var> vars; 
lexer *lex;

std::vector<std::string> tokens;

int dataLength = 0;
using namespace std;


int main(int argc,char *argv[])
{
	if(init(argc, argv))
		return 1;

	if(parsing())
		return 1;

	fileHeader();

	std::cout << "end of program!" << std::endl;
 
	return 0;
}  //int main(int argc, unsigned char *argv[])

int instruction()
{
	if(expect("while") || expect("for"))
	{
		int ltype = 0;
		if(expect("for"))
			ltype = 1;
		if(!nextToken())
		{
			sError = true;
			return 1;
		}
		tempToken = lex->getToken();
		if(!expect("("))
		{
			sError = true;
			std::cout << "no opening parenthesis at beginning of loop statement at line " << lex->lineCount << " column " << lex->colCount << std::endl;
			return 1;
		}
		if(!nextToken())
		{
			sError = true;
			return 1;
		}
		tempToken = lex->getToken();
		if(!ltype)
		{
			if(whileStatement())
			{
				sError = true;
				return 1;
			}
		}
		else
		{
			if(forStatement())
			{
				sError = true;
				return 1;
			}
		}
		return 0;
	}

	if(isName() == 1)
	{
		if(assignment())
		{
			std::cout << "no valid instruction found in file" << std::endl;
			sError = true;
			return 1;
		}
		return 0;
	}

	if(expect("print") || expect("println"))
	{
		bool ln = false;
		if(expect("println"))
			ln = true;
		if(print())
		{
			sError = true;
			return 1;
		}
		if(ln)
			fileBuf[actualDir++] = (unsigned char)PRTCR;
		return 0;
	}

	if(expect("read"))
	{
		if(read())
		{
			sError = true;
			return 1;
		}
		return 0;
	}

	if(expect("if"))
	{
		if(!nextToken())
		{
			sError = true;
			return 1;
		}
		tempToken = lex->getToken();
		if(!expect("("))
		{
			sError = true;
			std::cout << "no opening parenthesis at beginning of if statement at line " << lex->lineCount << " column " << lex->colCount << std::endl;
			return 1;
		}
		if(!nextToken())
		{
			sError = true;
			return 1;
		}
		tempToken = lex->getToken();
		if(conditionalStatement())
		{
			sError = true;
			return 1;
		}
		return 0;
	}
	std::cout << "no valid token found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
	return 1;
}  //int instruction()

int parsing()
{
	if(lex -> nextToken() && lex -> getToken().compare("BEGIN") == 0)
	{
		if(lex->nextToken())
		{
			tempToken = lex->getToken();
			if(declaration())
				return 1;
			while(!expect("END"))
			{
				if(codeblock())
					return 1;
			}
			fileBuf[actualDir++] = (unsigned char)HALT;
			return 0;
		}
		else
		{
			std::cout << "end of program not found";
			return 1;
		}
	}
	else
	{
		std::cout << "beginning of program not found";
		return 1;
	}
}  //int parsing()

int codeblock()
{
	int checkError = actualDir;
	while(instruction() != 0)
	{
		if(sError)
			return 1;
		if(!nextToken())
		{
			std::cout << "no token found at line " << lex->lineCount << "   column " << lex->colCount << std::endl;
			return 1;
		}
		tempToken = lex->getToken();
	}
	if(checkError == actualDir)
	{
		std::cout << "invalid token found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return 1;
	}
	return 0;
}  //int codeblock()

int forAssignment()
{
	if(isName() != 1)
	{
		std::cout << "no variable assignment at beginning of for statement at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return -1;
	}
	if(assignment())
	{
		std::cout << "no valid instruction found in file" << std::endl;
		return -1;
	}
}  //int forAssignment()

int forBoolExpression()
{
	int ans = logicOperation();
	if(ans < 0)
		return 1;
	if(!expect(";"))
	{
		std::cout << "no \";\" token found at the end of boolean expression of for statement at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return -1;
	}
	if(!nextToken())
		return -1;
	tempToken = lex->getToken();


	fileBuf[actualDir++] = (unsigned char)PUSHKI;
	for(int i = 3; i >= 0; i--)
		fileBuf[actualDir++] = (unsigned char)(1>>(8*i));
	fileBuf[actualDir++] = (unsigned char)CMP;
	return 0;
}  //int forBoolExpression()

int forStatement()
{
	isFor = true;
	int dirFinFor, dirInicioCondFinal, dirFinCondFinal;
	tag InicioBoolExp, InicioPostCond, FinPostCond, FinFor;
	stringstream ss;

	if(forAssignment())
		return -1;
	//first expression

	ss << tags.size();
	InicioBoolExp.name = ss.str();
	InicioBoolExp.dir = actualDir;
	if(forBoolExpression())
		return -1;


	ss << tags.size();
	FinFor.name = ss.str();
	fileBuf[actualDir++] = (unsigned char)JMPLT;
	dirFinFor = actualDir;
	actualDir += 2;
	//second expression


	ss << tags.size();
	FinPostCond.name = ss.str();
	fileBuf[actualDir++] = (unsigned char)JMP;
	dirFinCondFinal = actualDir;
	actualDir += 2;

	ss << tags.size();
	InicioPostCond.name = ss.str();
	InicioPostCond.dir = actualDir;

	if(forAssignment())
		return -1;
	if(!expect(")"))
	{
		std::cout << "no closing parenthesis found at the end of for statement at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return -1;
	}
	//third expression


	fileBuf[actualDir++] = (unsigned char)JMP;
	fileBuf[actualDir++] = (unsigned char)(InicioBoolExp.dir>>8);
	fileBuf[actualDir++] = (unsigned char)InicioBoolExp.dir;
	FinPostCond.dir = actualDir;
	fileBuf[dirFinCondFinal++] = (unsigned char)(FinPostCond.dir>>8);
	fileBuf[dirFinCondFinal] = (unsigned char)FinPostCond.dir; 
	if(!nextToken())
		return -1;
	tempToken = lex->getToken();

	if(expect("{"))
	{
		if(!nextToken())
			return -1;
		tempToken = lex->getToken();
		
		while(!expect("}"))
		{
			if(instruction())
				return -1;
		}
		if(!nextToken())
			return -1;
		tempToken = lex->getToken();
	}
	else
	{
		if(instruction())
			return -1;
	}
	fileBuf[actualDir++] = (unsigned char)JMP;
	fileBuf[actualDir++] = (unsigned char)(InicioPostCond.dir>>8);
	fileBuf[actualDir++] = (unsigned char)InicioPostCond.dir;
	FinFor.dir = actualDir;
	fileBuf[dirFinFor++] = (unsigned char)(FinFor.dir>>8);
	fileBuf[dirFinFor] = (unsigned char)FinFor.dir;

	tags.push_back(InicioBoolExp);
	tags.push_back(InicioPostCond);
	tags.push_back(FinPostCond);
	tags.push_back(FinFor);
	isFor = false;
	return 0;
}  //int forStatement()

int whileStatement()
{
	tag tempTag, tempTag2;
	stringstream ss;
	ss << tags.size();
	tempTag.name = ss.str();
	tempTag.dir = actualDir;
	tags.push_back(tempTag);
	int ans = logicOperation();
	if(!expect(")"))
	{
		std::cout << "no closing parenthesis for while statement at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return 1;
	}
	if(!nextToken())
		return -1;
	tempToken = lex->getToken();
	if(ans < 0)
		return 1;
	fileBuf[actualDir++] = (unsigned char)PUSHKI;
	for(int i = 3; i >= 0; i--)
		fileBuf[actualDir++] = (unsigned char)(1>>(8*i));
	fileBuf[actualDir++] = (unsigned char)CMP;
	ss << tags.size();
	tempTag2.name = ss.str();
	ans = tags.size();
	tags.push_back(tempTag2);
	fileBuf[actualDir++] = (unsigned char)JMPLT;
	int tempdir = actualDir;
	actualDir += 2;
	if(expect("{"))
	{
		if(!nextToken())
			return -1;
		tempToken = lex->getToken();
		
		while(!expect("}"))
		{
			int checkError = actualDir;
			if(instruction())
				return -1;
			if(checkError == actualDir)
			{
				std::cout << "invalid token at line " << lex->lineCount << " column " << lex->colCount << std::endl << tempToken << std::endl;
				return -1;
			}
		}
		if(!nextToken())
			return -1;
		tempToken = lex->getToken();
	}
	else
	{
		if(instruction())
			return -1;
	}
	fileBuf[actualDir++] = (unsigned char)JMP;
	fileBuf[actualDir++] = (unsigned char)(tempTag.dir>>8);
	fileBuf[actualDir++] = (unsigned char)tempTag.dir;
	tags[ans].dir = actualDir;

	fileBuf[tempdir++] = (unsigned char)(tags[ans].dir>>8);
	fileBuf[tempdir] = (unsigned char)(tags[ans].dir);
	return 0;
}  //int whileStatement()

void pushvar(int varindex)
{
	int tempdir;
	switch(vars[varindex].type)
	{
		case 'i': 
		{
			if(vars[varindex].isArray)
			{
				getArrIndex(1);
				fileBuf[actualDir++] = (unsigned char)PUSHAI;
			}
			else
				fileBuf[actualDir++] = (unsigned char)PUSHI;
			tempdir = vars[varindex].dir;
		}
		break;
		case 'c':
		{
			if(vars[varindex].isArray)
			{
				getArrIndex(1);
				fileBuf[actualDir++] = (unsigned char)PUSHAC;
			}
			else
				fileBuf[actualDir++] = (unsigned char)PUSHC;
			tempdir = vars[varindex].dir;
		}
		break;
		case 'f':
		{
			if(vars[varindex].isArray)
			{
				getArrIndex(1);
				fileBuf[actualDir++] = (unsigned char)PUSHAF;
			}
			else
				fileBuf[actualDir++] = (unsigned char)PUSHF;
			tempdir = vars[varindex].dir;
		}
		break;
		case 'd':
		{
			if(vars[varindex].isArray)
			{
				getArrIndex(1);
				fileBuf[actualDir++] = (unsigned char)PUSHAD;
			}
			else
				fileBuf[actualDir++] = (unsigned char)PUSHD;
			tempdir = vars[varindex].dir;
		}
		break;
	}
	fileBuf[actualDir++] = (unsigned char)(tempdir>>8);
	fileBuf[actualDir++] = (unsigned char)tempdir;
}  //void pushvar()

int pushnumassignment()
{
	int temptokenindex = 0;
	int sign = 1;
	if(tempToken[0] == '-')
	{
		sign = -1;
		temptokenindex++;
	}
	unsigned char buf;
	switch(gType)
	{
		case 'i': 
		{
			if(tempToken.find(".") == std::string::npos)
			{
				fileBuf[actualDir++] = (unsigned char)PUSHKI;
				int pi = 0;
				while(temptokenindex < tempToken.length())
				{
					pi *= 10;
					pi += ((int)(tempToken[temptokenindex++] - '0'));
				}
				for(int i = 3; i >= 0; i--)
					fileBuf[actualDir++] = (unsigned char)(pi>>(8*i));
				break;
			}
			std::cout << "not a type int at line " << lex->lineCount << " column " << lex->colCount << std::cout;
			return 1;
		}
		case 'c':
		{
			if(isNumber())
			{
				fileBuf[actualDir++] = (unsigned char)PUSHKC;
				int pc = 0;
				while(temptokenindex < tempToken.length())
				{
					pc *= 10; 
					pc += (int)(tempToken[temptokenindex++] - '0');
				}
				fileBuf[actualDir++] = (unsigned char)pc;
				break;
			}
			std::cout << "not a type unsigned char at line " << lex->lineCount << " column " << lex->colCount << std::cout;
			return 1;
		}
		case 'f':
		{
			fileBuf[actualDir++] = (unsigned char)PUSHKF;
			float pf = 0.0f;
			int decspaces = 0;
			while(temptokenindex < tempToken.length())
			{
				if(tempToken[temptokenindex] == '.' && decspaces == 0)
				{
					temptokenindex++;
					decspaces++;
					continue;
				}
				if(decspaces)
					decspaces++;
				pf *= 10.0f;
				pf += (float)(tempToken[temptokenindex++] - '0');
			}
			decspaces--;
			while(decspaces > 0)
			{
				pf /= 10.0f;
				decspaces--;
			}
			for(int i = 3; i >= 0; i--)
				fileBuf[actualDir++] = (unsigned char)((*(int*)&pf)>>(8*i));
		}
		break;
		case 'd':
		{
			fileBuf[actualDir++] = (unsigned char)PUSHKD;
			double pd = 0;
			int decspaces = 0;
			while(temptokenindex < tempToken.length())
			{
				if(tempToken[temptokenindex] == '.' && decspaces == 0)
				{
					decspaces++;
					temptokenindex++;
					continue;
				}
				if(decspaces)
					decspaces++;
				pd *= 10;
				pd += (double)(tempToken[temptokenindex++] - '0');
			}
			decspaces--;
			while(decspaces > 0)
			{
				pd /= 10;
				decspaces--;
			}
			temptokenindex = 7;
			strncpy(&fileBuf[actualDir],doubletochar(pd),8);
			actualDir += 8;
		}
		break;
		case 'x':
		{
			if(tempToken.find(".") == std::string::npos)
				gType = 'i';
			else
				gType = 'd';
			pushnumassignment();
		}
	}
	return 0;
}  //void pushnumassignment()

int read()
{	
	if(!nextToken())
		return 1;
	tempToken = lex->getToken();
	if(!expect("("))
		std::cout << "no valid opening parenthesis for read found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
	do
	{
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
		//std::cout << "reads assy instruction\n";
		if(isName() != 1)
		{
			std::cout << "no valid id found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
			return 1;
		}
		if(readVar(varDir()))
			return 1;
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
	}while(tempToken == ",");
	if(tempToken != ")")
	{
		std::cout << "no closing parenthesis for read at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return 1;
	}
	if(!nextToken())
		return 1;
	tempToken = lex->getToken();
	if(!expect(";"))
	{
		std::cout << "invalid read end \";\" found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return 1;
	}
	if(!nextToken())
		return 1;
	tempToken = lex->getToken();
	return 0;
}  //int read()

int readVar(int varindex)
{
	int arrIndex;
	int tempdir; 
	switch(vars[varindex].type)
	{
		case 'i':
		{
			if(vars[varindex].isArray)
			{
				arrIndex = getArrIndex(1);
				if(arrIndex < 0)
					return 1;
				fileBuf[actualDir++] = (unsigned char)RDAI;
			}
			else
				fileBuf[actualDir++] = (unsigned char)RDI;	
			break;
		}
		case 'c':
		{
			if(vars[varindex].isArray)
			{
				arrIndex = getArrIndex(1);
				if(arrIndex < 0)
					return 1;
				fileBuf[actualDir++] = (unsigned char)RDAC;
			}
			else
				fileBuf[actualDir++] = (unsigned char)RDC;
			break;
		}
		case 'f':
		{
			if(vars[varindex].isArray)
			{
				arrIndex = getArrIndex(1);
				if(arrIndex < 0)
					return 1;
				fileBuf[actualDir++] = (unsigned char)RDAF;
			}
			else
				fileBuf[actualDir++] = (unsigned char)RDF;
			break;
		}
		case 'd':
		{
			if(vars[varindex].isArray)
			{
				arrIndex = getArrIndex(1);
				if(arrIndex < 0)
					return 1;
				fileBuf[actualDir++] = (unsigned char)RDAD;
			}
			else
				fileBuf[actualDir++] = (unsigned char)RDD;
			break;
		}
		case 's':
		{
			if(vars[varindex].isArray)
			{
				arrIndex = getArrIndex(1);
				if(arrIndex < 0)
					return 1;
				fileBuf[actualDir++] = (unsigned char)RDAS;
			}
			else
				fileBuf[actualDir++] = (unsigned char)RDS;
			break;
		}
	}
	tempdir = vars[varindex].dir;
	fileBuf[actualDir++] = (unsigned char)(tempdir>>8);
	fileBuf[actualDir++] = (unsigned char)tempdir;
	return 0;
}  //void readVar(int varindex)

int print()
{	
	if(!nextToken())
		return 1;
	tempToken = lex->getToken();
	if(!expect("("))
		std::cout << "no valid token found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
	do
	{
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
		//std::cout << "prints assy instruction\n";
		if(expect("\""))
		{
			fileBuf[actualDir++] = (unsigned char)((int)PRTM);
			if(!nextToken())
				return 1;
			tempToken = lex->getToken();
			fileBuf[actualDir++] = (unsigned char)tempToken.length();
			for(int i = 0; i < tempToken.length(); i++)
				fileBuf[actualDir++] = tempToken[i];
			if(!nextToken())
				return 1;
			tempToken = lex->getToken();
			if(!expect("\""))
				std::cout << "no end of string found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		}
		else
		{
			if(isName() != 1)
			{
				std::cout << "no valid id found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
				return 1;
			}
			if(printVar(varDir()))
				return 1;
		}
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
	}while(tempToken == ",");
	if(tempToken != ")")
	{
		std::cout << "no closing parenthesis for print at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return 1;
	}
	if(!nextToken())
		return 1;
	tempToken = lex->getToken();
	if(!expect(";"))
	{
		std::cout << "invalid print end \";\" found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return 1;
	}
	if(!nextToken())
		return 1;
	tempToken = lex->getToken();
	return 0;
}  //int print()

int printVar(int varindex)
{
	int arrIndex;
	int tempdir; 
	switch(vars[varindex].type)
	{
		case 'i':
		{
			if(vars[varindex].isArray)
			{
				arrIndex = getArrIndex(1);
				if(arrIndex < 0)
					return 1;
				fileBuf[actualDir++] = (unsigned char)PRTAI;
			}
			else
				fileBuf[actualDir++] = (unsigned char)PRTI;	
			break;
		}
		case 'c':
		{
			if(vars[varindex].isArray)
			{
				arrIndex = getArrIndex(1);
				if(arrIndex < 0)
					return 1;
				fileBuf[actualDir++] = (unsigned char)PRTAC;
			}
			else
				fileBuf[actualDir++] = (unsigned char)PRTC;
			break;
		}
		case 'f':
		{
			if(vars[varindex].isArray)
			{
				arrIndex = getArrIndex(1);
				if(arrIndex < 0)
					return 1;
				fileBuf[actualDir++] = (unsigned char)PRTAF;
			}
			else
				fileBuf[actualDir++] = (unsigned char)PRTF;
			break;
		}
		case 'd':
		{
			if(vars[varindex].isArray)
			{
				arrIndex = getArrIndex(1);
				if(arrIndex < 0)
					return 1;
				fileBuf[actualDir++] = (unsigned char)PRTAD;
			}
			else
				fileBuf[actualDir++] = (unsigned char)PRTD;
			break;
		}
		case 's':
		{
			if(vars[varindex].isArray)
			{
				arrIndex = getArrIndex(1);
				if(arrIndex < 0)
					return 1;
				fileBuf[actualDir++] = (unsigned char)PRTAS;
			}
			else
				fileBuf[actualDir++] = (unsigned char)PRTS;
			break;
		}
	}
	tempdir = vars[varindex].dir;
	fileBuf[actualDir++] = (unsigned char)(tempdir>>8);
	fileBuf[actualDir++] = (unsigned char)tempdir;
	return 0;
}  //void printVar(int varindex)

int makeJumps(int cmpType, bool isNot) // > es 1, >= es 2, < es 3, <= es 4, == es 5, != es 6, -1 es error
{
	int ans;
	switch(cmpType)
	{
		case 1: fileBuf[actualDir++] = (unsigned char)JMPGT;
			break;
		case 2: fileBuf[actualDir++] = (unsigned char)JMPGE;
			break;
		case 3: fileBuf[actualDir++] = (unsigned char)JMPLT;
			break;
		case 4: fileBuf[actualDir++] = (unsigned char)JMPLE;
			break;
		case 5: fileBuf[actualDir++] = (unsigned char)JMPEQ;
			break;
		case 6: fileBuf[actualDir++] = (unsigned char)JMPNE;
			break;
		default: return -1;
			break;
	}
	tag tempTag;
	stringstream ss;
	ss << tags.size();
	tempTag.name = ss.str();
	tempTag.dir = actualDir + 10;
	tags.push_back(tempTag);
	fileBuf[actualDir++] = (unsigned char)(tempTag.dir>>8);
	fileBuf[actualDir++] = (unsigned char)tempTag.dir;
	fileBuf[actualDir++] = (unsigned char)PUSHKI;
	if(isNot)
		ans = 1;
	else
		ans = 0;
	for(int i = 3; i >= 0; i--)
		fileBuf[actualDir++] = (unsigned char)(ans>>(8*i));
	fileBuf[actualDir++] = (unsigned char)JMP;
	ss << tags.size();
	tempTag.name = ss.str();
	tempTag.dir = actualDir + 7;
	tags.push_back(tempTag);
	fileBuf[actualDir++] = (unsigned char)(tempTag.dir>>8);
	fileBuf[actualDir++] = (unsigned char)(tempTag.dir);
	fileBuf[actualDir++] = (unsigned char)PUSHKI;
	if(isNot)
		ans = 0;
	else
		ans = 1;
	for(int i = 3; i >= 0; i--)
		fileBuf[actualDir++] = (unsigned char)(ans>>(8*i));
	return 0;
}  //int makeJumps(int cmpType, bool isNot)

int logicOperation()
{
	int ans;
	int cmpType;
	bool isNot = false;
	gType = 'x';
	if(expect("!"))
	{
		isNot = !isNot;
		if(!nextToken())
			return -1;
		tempToken = lex->getToken();
	}
	if(expect("("))
	{
		if(!nextToken())
			return -1;
		tempToken = lex->getToken();
		ans = logicOperation();
		if(!expect(")"))
		{
			std::cout << "no closing parenthesis found for inner operation in if statement at line " << lex->lineCount << " column " << lex->colCount << std::endl;
			return -1;
		}
		if(!nextToken())
			return -1;
		tempToken = lex->getToken();
	}
	else
	{
		if(operation())
			return -1;
		cmpType = compareResult();
		if(!nextToken())
			return -1;
		tempToken = lex->getToken();
		if(operation())
			return -1;
		fileBuf[actualDir++] = (unsigned char)CMP;
		if(makeJumps(cmpType, isNot))
			return -1;
		ans = 1;
	}
	if(expect("||"))
	{
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
		ans += logicOperation();
		fileBuf[actualDir++] = (unsigned char)ADD;
		return ans;
	}
	else if(expect("&&"))
	{
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
		ans *= logicOperation();
		fileBuf[actualDir++] = (unsigned char)MUL;
		return ans;
	}
	return ans;
}  //int logicOperation()

int conditionalStatement()
{
	tag tempTag;
	stringstream ss;
	int ans = logicOperation();
	if(!expect(")"))
	{
		std::cout << "no closing parenthesis for if statement at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return 1;
	}
	if(!nextToken())
		return -1;
	tempToken = lex->getToken();
	if(ans < 0)
		return 1;
	fileBuf[actualDir++] = (unsigned char)PUSHKI;
	for(int i = 3; i >= 0; i--)
		fileBuf[actualDir++] = (unsigned char)(1>>(8*i));
	fileBuf[actualDir++] = (unsigned char)CMP;
	ss << tags.size();
	ans = tags.size();
	tempTag.name = ss.str();
	tags.push_back(tempTag);

	fileBuf[actualDir++] = (unsigned char)JMPLT;
	int tempdir = actualDir;
	actualDir += 2;
	if(expect("{"))
	{
		if(!nextToken())
			return -1;
		tempToken = lex->getToken();
		while(!expect("}"))
		{
			if(instruction())
				return -1;
		}
		if(!nextToken())
			return -1;
		tempToken = lex->getToken();
	}
	else
	{
		if(instruction())
			return -1;
	}
	tags[ans].dir = actualDir;
	if(expect("else"))
		tags[ans].dir+=3; 
	fileBuf[tempdir++] = (unsigned char)(tags[ans].dir>>8);
	fileBuf[tempdir] = (unsigned char)tags[ans].dir;
	if(expect("else"))
	{
		fileBuf[actualDir++] = (unsigned char)JMP;
		ss << tags.size();
		ans = tags.size();
		tempTag.name = ss.str();
		tags.push_back(tempTag);
		tempdir = actualDir;
		actualDir+=2;
		if(!nextToken())
			return -1;
		tempToken = lex->getToken();
		if(expect("{"))
		{
			if(!nextToken())
				return -1;
			tempToken = lex->getToken();
			while(!expect("}"))
			{
				if(instruction())
					return -1;
			}
			if(!nextToken())
				return -1;
			tempToken = lex->getToken();
		}
		else
		{
			if(instruction())
				return -1;
		}
		tags[ans].dir = actualDir;
		fileBuf[tempdir++] = (unsigned char)(tags[ans].dir>>8);
		fileBuf[tempdir] = (unsigned char)tags[ans].dir;
	}
	return 0;
}  //int conditionalStatement()

int pop(int tempIndex)
{
	switch(vars[tempIndex].type)
	{
		case 'c':
			if(vars[tempIndex].isArray)
			{
				tempint = POPAC;
				break;
			}
			tempint = POPC;
			break;
		case 'i': 
			if(vars[tempIndex].isArray)
			{
				tempint = POPAI;
				break;
			}
			tempint = POPI;
			break;
		case 'f': 
			if(vars[tempIndex].isArray)
			{
				tempint = POPAF;
				break;
			}
			tempint = POPF;
			break;
		case 'd': 
			if(vars[tempIndex].isArray)
			{
				tempint = POPAD;
				break;
			}
			tempint = POPD;
			break;
		case 's':
			if(vars[tempIndex].isArray)
			{
				tempint = POPAS;
				break;
			}
			tempint = POPS;
			break;
		default: std::cout << "weird variable failure at line " << lex->lineCount << " column " << lex->colCount << std::endl;
			return 1;
	}
	return 0;
}  //int pop(int tempIndex)

bool isReal()
{
	int i = 0;
	bool dec = false;
	if(tempToken[i] == '-')
		i++;
	while(i < tempToken.length())
	{
		if(tempToken[i] < '0' || tempToken[i] > '9')
		{
			if(tempToken[i] == '.')
				if(dec)
					return false;
				else
				{
					dec = true;
					break;
				}
			return false;
		}
		i++;
	}
	return true;
}  //bool isReal()

void emitBuffer()
{
	for(int i = 0; i < actualDir; i++)
		fout << fileBuf[i];
}  //void emitBuffer()

int varDir()
{
	for(int i = 0; i < vars.size(); i++)
		if(vars[i].name == tempToken)
			return i;
	return -1;
}  //int varDir()

int isName()
{
	for(int i = 0; i < tokens.size(); i++)
	{
		if(expect(tokens[i]))
			return -1;
	}

	for(int i = 0; i < vars.size(); i++)
		if(expect(vars[i].name))
			return 1;

	return 0;
}  //int isName()

bool isNumber()
{
	for(int i = 0; i < tempToken.length(); i++)
		if(tempToken[i] < '0' || tempToken[i] > '9')
			return false;
	return true;
}  //bool isNumber()

bool expect(std::string token)
{
	if(tempToken.compare(token))
		return false;
	return true;
}  //bool expect(std::string token)

int num()
{
	bool neg = false;
	if(expect("("))
	{
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
		if(operation())
			return 1;
		if(!expect(")"))
		{
			std::cout << "no closing parenthesis found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
			return 1;
		}
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
		return 0;
	}
	if(isName() == 1)
	{
		pushvar(varDir());
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
		return 0;
	}
	std::string tempToken2;
	std::string switchstring;
	if(tempToken == "-")
	{
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
		neg = true;
	}
	if(!isNumber())
	{
		std::cout << "invalid number format at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return 1;
	}
	switchstring.append(tempToken);
	if(!nextToken())
		return 1;
	tempToken = lex->getToken();
	if(tempToken == ".")
	{
		switchstring.append(".");
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
		if(!isNumber())
		{
			std::cout << "invalid number format at line " << lex->lineCount << " column " << lex->colCount << std::endl;
			return 1;
		}
		switchstring.append(tempToken);
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
	}
	tempToken2 = tempToken;
	tempToken = switchstring; 
	pushnumassignment(); 
	if(neg)
	{
		fileBuf[actualDir++] = (unsigned char)PUSHKI;
		for(int i= 3; i >= 0; i--)
			fileBuf[actualDir++] = (char)((-1)>>(8*i));
		fileBuf[actualDir++] = (unsigned char)MUL;
	}
	tempToken = tempToken2;
	return 0;
}  //int num()

int factor()
{
	int mulop;
	if(num())
		return 1;
	while(tempToken == "*" || tempToken == "/" || tempToken == "%")
	{
		if(tempToken == "*")
			mulop = 61;
		else if(tempToken == "/")
			mulop = 62;
		else
			mulop = 63;
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
		if(num())
			return 1;
		fileBuf[actualDir++] = (unsigned char)mulop;
	}
	return 0;
}  //int factor()

int operation()
{
	int addop;
	if(factor())
		return 1;
	while(tempToken == "+" || tempToken == "-")
	{
		if(tempToken == "+")
			addop = 59;
		else
			addop = 60;
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
		if(factor())
			return 1;
		fileBuf[actualDir++] = (unsigned char)addop;
	}
	return 0;
}  //int operation()

int stringAssignment()
{
	if(!expect("\""))
	{
		if(isName() != 1)
		{
			std::cout << "unrecognized string variable name at line " << lex->lineCount << " column " << lex->colCount << std::endl;
			return 1;
		}
		int stringIndex = varDir();
		if(vars[stringIndex].isArray)
		{
			getArrIndex(1);
			fileBuf[actualDir++] = (unsigned char)PUSHAS;
		}
		else
			fileBuf[actualDir++] = (unsigned char)PUSHS;
		fileBuf[actualDir++] = (unsigned char)(vars[stringIndex].dir>>8);
		fileBuf[actualDir++] = (unsigned char)vars[stringIndex].dir;
	}
	else
	{
		fileBuf[actualDir++] = (unsigned char)PUSHKS;
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
		fileBuf[actualDir++] = (unsigned char)tempToken.length();
		for(int i = 0; i < tempToken.length(); i++)
			fileBuf[actualDir++] = (unsigned char)tempToken[i];
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
		if(!expect("\""))
		{
			std::cout << "no closing quotes for string found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
			return 1;
		}
	}
	if(!nextToken())
		return 1;
	tempToken = lex->getToken();
	return 0;
}  //int stringAssignment()

int charAssignment()
{
	if(expect("'"))
	{
		if(!nextToken())
			return -1;
		tempToken = lex->getToken();
		fileBuf[actualDir++] = (unsigned char)PUSHKC;
		fileBuf[actualDir++] = (char)tempToken[0];
		if(!nextToken())
			return -1;
		tempToken = lex->getToken();
		if(!expect("'"))
		{
			std::cout << "no closing apostrophe for char variable\n";
			return -1;
		}
	}
	else
	{
		if(isName() != 1)
		{
			std::cout << "unrecognized variable id at line " << lex->lineCount << " column " << lex->colCount << std::endl;
			return 1;
		}
		int charIndex = varDir();
		if(vars[charIndex].isArray)
		{
			getArrIndex(1);
			fileBuf[actualDir++] = (unsigned char)PUSHAC;
		}
		else
			fileBuf[actualDir++] = (unsigned char)PUSHC;
		fileBuf[actualDir++] = (unsigned char)(vars[charIndex].dir>>8);
		fileBuf[actualDir++] = (unsigned char)(vars[charIndex].dir);
	}
	if(!nextToken())
		return 1;
	tempToken = lex->getToken();
	return 0;
}  //int charAssignment()

int assignment()
{
	int tempIndex = varDir();
	if(tempIndex < 0)
	{
		std::cout << "invalid id at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return 1;
	}
	gType = vars[tempIndex].type;
	if(vars[tempIndex].isArray)
		getArrIndex(2);
	if(!nextToken())
		return 1;
	tempToken = lex->getToken();
	if(!vars[tempIndex].isArray && gType == 'i' && (expect("++") || expect("--")))
	{
		if(expect("++"))
			fileBuf[actualDir++] = (unsigned char)INC;
		else
			fileBuf[actualDir++] = (unsigned char)DEC;
		fileBuf[actualDir++] = (unsigned char)(vars[tempIndex].dir>>8);
		fileBuf[actualDir++] = (unsigned char)vars[tempIndex].dir;
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
		if(!expect(";"))
		{
			std::cout << "no closing semicolon for assignment found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
			return 1;
		}
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
		return 0;
	}
	if(tempToken != "=" )
	{
		std::cout << "no \"=\" sign found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return 1;
	}
	if(!nextToken())
		return 1;
	tempToken = lex->getToken();
	if(gType == 's')
	{
		if(stringAssignment())
			return 1;
	}
	else
	{
		if(gType == 'c')
		{
			if(charAssignment())
				return 1;
		}
		else
		{
			if(operation())
				return 1;
		}
	}
	//std::cout << "popvar assy instruction " << std::endl;
	if(vars[tempIndex].isArray)
		fileBuf[actualDir++] = (unsigned char)MOVY;
	if(pop(tempIndex))
		return 1;
	fileBuf[actualDir++] = tempint;
	tempint = vars[tempIndex].dir;
	fileBuf[actualDir++] = (unsigned char)(tempint>>8);
	fileBuf[actualDir++] = (unsigned char)tempint;
	if(!expect(";"))
	{
		if(isFor)
			return 0;
		std::cout << "no ending semicolon found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return 1;
	}
	if(!nextToken())
		return 1;
	tempToken = lex->getToken();
	return 0;
}  //int assignment()

int getArrIndex(int t) //t == 1 assigning side of equation, t == 2 assigned side of equation
{
	int multiplier = 1, tempind = varDir();
	if(vars[tempind].isArray && vars[tempind].type == 's')
		multiplier *= vars[tempind].stringSize;
	if(!nextToken())
		return -1;
	if(lex->getToken() != "[")
	{
		std::cout << "no \"[\" token found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return -1;
	}
	if(!nextToken())
		return -1;
	tempToken = lex->getToken();
	int arrIndex = 0;
	if(!isNumber())
	{
		arrIndex = varDir();
		int tempdir = vars[arrIndex].dir;
		if(isName() != 1)
		{
			std::cout << "invalid array index found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
			return -1;
		}
		if(vars[arrIndex].type != 'i')
		{
			std::cout << "invalid array index at line " << lex->lineCount << " column " << lex->colCount << std::endl;
			return -1;
		}
		if(vars[arrIndex].isArray)
		{
			arrIndex = getArrIndex(1);
			if(arrIndex < 0)
				return -1;
			fileBuf[actualDir++] = (unsigned char)PUSHAI;
		}
		else
		{
			fileBuf[actualDir++] = (unsigned char)PUSHI;
		}
		fileBuf[actualDir++] = (unsigned char)(tempdir>>8);
		fileBuf[actualDir++] = (unsigned char)tempdir;
		fileBuf[actualDir++] = (unsigned char)PUSHKI;
		for(int i = 3; i >= 0; i--)
			fileBuf[actualDir++] = (unsigned char)(multiplier>>(8*i));
		fileBuf[actualDir++] = (unsigned char)MUL;
		if(t == 1)
			fileBuf[actualDir++] = (unsigned char)POPX;
		else
			fileBuf[actualDir++] = (unsigned char)POPY;
	}
	else
	{
		for(int i = 0; i < tempToken.length(); i++)
		{
			arrIndex *= 10;
			arrIndex += (int)(tempToken[i]-'0');
		}
		fileBuf[actualDir++] = (unsigned char)PUSHKI;
		for(int i = 3; i >= 0; i--)
			fileBuf[actualDir++] = (unsigned char)((arrIndex)>>(8*i));
		fileBuf[actualDir++] = (unsigned char)PUSHKI;
		for(int i = 3; i >= 0; i--)
			fileBuf[actualDir++] = (unsigned char)(multiplier>>(8*i));
		fileBuf[actualDir++] = (unsigned char)(MUL);
		if(t == 1)
			fileBuf[actualDir++] = (unsigned char)POPX;
		else
			fileBuf[actualDir++] = (unsigned char)POPY;
	}
	if(!nextToken())
		return -1;
	if(lex->getToken() != "]")
	{
		std::cout << "invalid token found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
 		return -1;
	}
	return 0;
}  //int getArrIndex()

int compareResult()  // > es 1, >= es 2, < es 3, <= es 4, == es 5, != es 6, -1 es error
{
	if(expect(">"))
		return 1;
	if(expect(">="))
		return 2;
	if(expect("<"))
		return 3;
	if(expect("<="))
		return 4;
	if(expect("=="))
		return 5;
	if(expect("!="))
		return 6;
	return -1;
}  //int compareResult(bool isNot)

int declaration()
{
	int tempSize;
	while(expect("int") || expect("double") || expect("float") || expect("string") || expect("char"))
	{
		std::string temptype = tempToken;
		do
		{
			var tempVar;
			if(temptype == "int")
			{
				tempSize = 4;
				tempVar.size = 4;
				tempVar.type = 'i';
				tempVar.dir = dataLength;
				tempVar.isArray = false;
			}
			if(temptype == "double")
			{
				tempSize = 8;
				tempVar.size = 8;
				tempVar.type = 'd';
				tempVar.dir = dataLength;
				tempVar.isArray = false;
			}
			if(temptype == "float")
			{
				tempSize = 4;
				tempVar.size = 4;
				tempVar.type = 'f';
				tempVar.dir = dataLength;
				tempVar.isArray = false;
			}
			if(temptype == "char")
			{
				tempSize = 1;
				tempVar.size = 1;
				tempVar.type = 'c';
				tempVar.dir = dataLength;
				tempVar.isArray = false;
			}
			if(!nextToken())
				return 1;
			tempToken = lex->getToken();
			if(isName())
			{
				std::cout << "\"" << tempToken << "\" is already a reserved keyword, line: " << lex->lineCount << " column: " << lex->colCount << std::endl;
				return -1;
			}
			tempVar.name = tempToken;
			if(!nextToken())
				return 1;
			tempToken = lex->getToken();
			if(temptype == "string")
			{
				tempVar.type = 's';
				tempVar.dir = dataLength;
				tempVar.isArray = false;
				if(!expect("["))
				{
					std::cout << "no size was declared for string variable\n";
					return 1;
				}
				if(!nextToken())
					return 1;
				tempToken = lex->getToken();
				tempSize = atoi(tempToken.c_str());
				if(tempSize < 0)
				{
					std::cout << "invalid size for string\n";
					return 1;
				}
				tempVar.size  = tempSize;
				tempVar.stringSize = tempSize;
				if(!nextToken())
					return 1;
				tempToken = lex->getToken();
				if(!expect("]"))
				{
					std::cout << "no closing bracket for string size\n";
					return 1;
				}
				if(!nextToken())
					return 1;
				tempToken = lex->getToken();
			}
			if(expect("["))
			{
				tempVar.isArray = true;
				if(!nextToken())
					return 1;
				tempToken = lex->getToken();
				if(!isNumber())
				{
					std::cout << "invalid array size, must be an integer larger than 0, line: " << lex->lineCount << " column: " << lex->colCount << std::endl;
					return -1;
				}
				tempVar.size *= atoi(tempToken.c_str());
				if(!nextToken())
					return 1;
				if(!(lex->getToken() == "]") || !(lex->nextToken()))
				{
					std::cout << "syntax error at line: " << lex->lineCount << "  column: " << lex->colCount << std::endl;
					return -1;
				}
				tempToken = lex->getToken();
			}
			vars.push_back(tempVar);
			dataLength += tempVar.size;
			tempVar.size = tempSize;
		}while(expect(","));
		if(expect(";") && lex->nextToken())
			tempToken = lex->getToken();
		else
		{
			std::cout << "no instruction ending found at line: " << lex->lineCount << " column: " << lex->colCount << std::endl;
			return -1;
		}
	}
	return 0;
}  //int declaration()

void fileHeader()
{
	fout << "(C)CHUNKUN";
	unsigned char tchar = (dataLength>>8);
	fout << tchar;
	tchar = dataLength;
	fout << tchar;
	tchar = ((actualDir+8)>>8);
	fout << tchar;
	tchar = (actualDir+8);
	fout << tchar;

	//callCode();

	for(int i = 0; i < actualDir; i++)
		fout << ( unsigned char)fileBuf[i];
}  //void fileHeader()

bool nextToken()
{
	if(!lex->nextToken())
	{
		std::cout << "no valid token found at line " << lex->lineCount << " column " << lex->lineCount << std::endl;
		return false;
	}
	return true;
}  //bool nextToken()

void tokenNames()
{
	tokens.push_back(";");
	tokens.push_back("(");
	tokens.push_back(")");
	tokens.push_back("{");
	tokens.push_back("}");
	tokens.push_back("[");
	tokens.push_back("]");
	tokens.push_back("+");
	tokens.push_back("-");
	tokens.push_back("/");
	tokens.push_back("*");
	tokens.push_back("%");
	tokens.push_back("int");
	tokens.push_back("unsigned char");
	tokens.push_back("float");
	tokens.push_back("double");
	tokens.push_back("string");
	tokens.push_back("if");
	tokens.push_back("else");
	tokens.push_back("for");
	tokens.push_back("while");
	tokens.push_back("&&");
	tokens.push_back("||");
	tokens.push_back("!");
	tokens.push_back("==");
	tokens.push_back("!=");
	tokens.push_back(">=");
	tokens.push_back("<=");
	tokens.push_back(">");
	tokens.push_back("<");
	tokens.push_back("=");
}  //void tokenNames()

int init(int argc, char *argv[])
{
	lex = new lexer(argc, argv);
	tokens.reserve(31);
	tokenNames();
	if(argc > 2)
		fout.open(argv[2], ios::binary | ios::out | ios::trunc);
	else
		fout.open("CK.CHOP", ios::binary | ios::out | ios::trunc);


	if(lex->getError())
	{
		std::cout << "error opening file!\n";
		return 1;
	}
	return 0;
} //void init()

void callCode()
{
	for(int i = 0; i < actualDir; i++)
	{
		if((i+1)%4 == 0)
			printf("   %x\n", i);
		printf("%x ", ( unsigned char)fileBuf[i]);
	}
	std::cout << std::endl << std::endl;
}  //void callColde()