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


enum COMMANDS {HALT, PRTCR, PRTC, PRTI, PRTF, PRTD, PRTS, PRTAC, PRTAI, PRTAF, PRTAD, PRTAS, PUSHC, PUSHI, PUSHF, PUSHD, PUSHS, PUSHAC, PUSHAI, PUSHAF, PUSHAD, PUSHAS, PUSHKC, PUSHKI, PUSHKF, PUSHKD, PUSHKS, POPC, POPI, POPF, POPD, POPS, POPX, POPAC, POPAI, POPAF, POPAD, POPAS, RDC, RDI, RDF, RDD, RDS, RDAC, RDAI, RDAF, RDAD, RDAS, JMP, JMPEQ, JMPNE, JMPGT, JMPGE, JMPLT, JMPLE, STX, STKX, INC, DEC, ADD, SUB, MUL, DIV, MOD, CMP, PRTM, MOVY, POPY};

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
	char type;
	int dir;
	std::string name;
	bool isArray;
	int size;
	int i;
	float f;
	char c;
	double d;
	std::string s;
};

std::vector<var> vars; 
lexer *lex;

std::vector<std::string> tokens;

int dataLength = 0;
using namespace std;


int main(int argc, char *argv[])
{
	if(init(argc, argv))
		return 1;

	if(parsing())
	{
//		return 1;
	}

	fileHeader();

	std::cout << "end of program!" << std::endl;
 
	return 0;
}  //int main(int argc, char *argv[])

int instruction()
{
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
	if(tempToken == "print" || tempToken == "println")
	{
		bool ln = false;
		if(tempToken == "println")
			ln = true;
		if(print())
			return 1;
		if(ln)
			fileBuf[actualDir++] = (char)PRTCR;
		return 0;
	}
	return 0;
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

int num()
{
	if(expect("("))
	{
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
		if(operation())
			return 1;
		if(expect(")"))
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
		switchstring = "-";
		tempToken = lex->getToken();
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
	tempToken = tempToken2;
	return 0;
}  //int num()

int factor()
{
	int mulop;
	if(num())
		return 1;
	while(tempToken == "*" || tempToken == "/" || tempToken == "%%")
	{
		if(tempToken == "*")
			mulop = 61;
		else if(tempToken == "/")
			mulop = 62;
		else
			mulop = 63;
		if(!nextToken())
		{
			return 1;
		}
		if(!num())
			return 1;
		if(!nextToken())
			tempToken = lex->getToken();
		fileBuf[actualDir++] = (char)mulop;
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
		if(!factor())
			return 1;
		if(!nextToken())
			return 1;
		tempToken = lex->getToken();
		fileBuf[actualDir++] = (char)addop;
	}
	return 0;
}  //int operation()

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
	if(!lex->nextToken() || lex->getToken() != "=" || !lex->nextToken())
	{
		std::cout << "no \"=\" sign found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return 1;
	}
	tempToken = lex->getToken();
	if(operation())
		return 1;
	std::cout << "popvar assy instruction" << std::endl;
	if(vars[tempIndex].isArray)
		fileBuf[actualDir++] = (char)MOVY;
	if(pop(tempIndex))
		return 1;
	fileBuf[actualDir++] = tempint;
	tempint = vars[tempIndex].dir;
	fileBuf[actualDir++] = (char)tempint>>8;
	fileBuf[actualDir++] = (char)tempint;
	if(tempToken != ";")
	{
		std::cout << "invalid token found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return 1;
	}
	std::cout << "ping\n";
	if(!nextToken())
		return 1;
	tempToken = lex->getToken();
	return 0;
}  //int assignment()


int getArrIndex(int t) //t == 1 assigning side of equation, t == 2 assigned side of equation
{
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
			fileBuf[actualDir++] = (char)PUSHAI;
			fileBuf[actualDir++] = (char)tempdir>>8;
			fileBuf[actualDir++] = (char)tempdir;
			if(t == 1)
				fileBuf[actualDir++] = (char)POPX;
			else
				fileBuf[actualDir++] = (char)POPY;
		}
		else
		{
			fileBuf[actualDir++] = (char)PUSHI;
			fileBuf[actualDir++] = (char)tempdir>>8;
			fileBuf[actualDir++] = (char)tempdir;
			if(t == 1)
				fileBuf[actualDir++] = (char)POPX;
			else
				fileBuf[actualDir++] = (char)POPY;
		}
	}
	else
	{
		for(int i = 0; i < tempToken.length(); i++)
		{
			arrIndex *= 10;
			arrIndex += (int)(tempToken[i]-'0');
		}
		fileBuf[actualDir++] = (char)PUSHKI;
		for(int i = 3; i >= 0; i--)
			fileBuf[actualDir++] = (char)arrIndex>>(8*i);
		if(t == 1)
			fileBuf[actualDir++] = (char)POPX;
		else
			fileBuf[actualDir++] = (char)POPY;
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

int codeblock()
{
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
	return 0;
}  //int codeblock()

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
				fileBuf[actualDir++] = (char)PUSHAI;
			}
			else
				fileBuf[actualDir++] = (char)PUSHI;
			tempdir = vars[varindex].dir;
		}
		break;
		case 'c':
		{
			if(vars[varindex].isArray)
			{
				getArrIndex(1);
				fileBuf[actualDir++] = (char)PUSHAC;
			}
			else
				fileBuf[actualDir++] = (char)PUSHC;
			tempdir = vars[varindex].dir;
		}
		break;
		case 'f':
		{
			if(vars[varindex].isArray)
			{
				getArrIndex(1);
				fileBuf[actualDir++] = (char)PUSHAF;
			}
			else
				fileBuf[actualDir++] = (char)PUSHF;
			tempdir = vars[varindex].dir;
		}
		break;
		case 'd':
		{
			if(vars[varindex].isArray)
			{
				getArrIndex(1);
				fileBuf[actualDir++] = (char)PUSHAD;
			}
			else
				fileBuf[actualDir++] = (char)PUSHD;
			tempdir = vars[varindex].dir;
		}
		break;
	}
	fileBuf[actualDir++] = (char)tempdir>>8;
	fileBuf[actualDir++] = (char)tempdir;
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
	char buf;
	switch(gType)
	{
		case 'i': 
		{
			if(tempToken.find(".") == std::string::npos)
			{
				fileBuf[actualDir++] = (char)PUSHKI;
				int pi = 0;
				while(temptokenindex < tempToken.length())
				{
					pi *= 10;
					pi += ((int)(tempToken[temptokenindex++] - '0'));
				}
				for(int i = 3; i >= 0; i--)
					fileBuf[actualDir++] = (char)(pi>>(8*i));
				break;
			}
			std::cout << "not a type int at line " << lex->lineCount << " column " << lex->colCount << std::cout;
			return 1;
		}
		case 'c':
		{
			if(isNumber())
			{
				fileBuf[actualDir++] = (char)PUSHKC;
				int pc = 0;
				while(temptokenindex < tempToken.length())
				{
					pc *= 10; 
					pc += (int)(tempToken[temptokenindex++] - '0');
				}
				fileBuf[actualDir++] = (char)pc;
				break;
			}
			std::cout << "not a type char at line " << lex->lineCount << " column " << lex->colCount << std::cout;
			return 1;
		}
		case 'f':
		{
			fileBuf[actualDir++] = (char)PUSHKF;
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
				fileBuf[actualDir++] = (char)((*(int*)&pf)>>(8*i));
		}
		break;
		case 'd':
		{
			fileBuf[actualDir++] = (char)PUSHKD;
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
	}
	return 0;
}  //void pushnumassignment()

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
		std::cout << "prints assy instruction\n";
		if(expect("\""))
		{
			fileBuf[actualDir++] = (char)((int)PRTM);
			if(!nextToken())
				return 1;
			tempToken = lex->getToken();
			fileBuf[actualDir++] = (char)tempToken.length();
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
		std::cout << "invalid token found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
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
				fileBuf[actualDir++] = (char)PRTAI;
			}
			else
				fileBuf[actualDir++] = (char)PRTI;	
			break;
		}
		case 'c':
		{
			if(vars[varindex].isArray)
			{
				arrIndex = getArrIndex(1);
				if(arrIndex < 0)
					return 1;
				fileBuf[actualDir++] = (char)PRTAC;
			}
			else
				fileBuf[actualDir++] = (char)PRTC;
			break;
		}
		case 'f':
		{
			if(vars[varindex].isArray)
			{
				arrIndex = getArrIndex(1);
				if(arrIndex < 0)
					return 1;
				fileBuf[actualDir++] = (char)PRTAF;
			}
			else
				fileBuf[actualDir++] = (char)PRTF;
			break;
		}
		case 'd':
		if(vars[varindex].isArray)
		{
			arrIndex = getArrIndex(1);
			if(arrIndex < 0)
				return 1;
			fileBuf[actualDir++] = (char)PRTAD;
		}
		else
			fileBuf[actualDir++] = (char)PRTD;
		break;
	}
	tempdir = vars[varindex].dir;
	fileBuf[actualDir++] = (char)tempdir>>8;
	fileBuf[actualDir++] = (char)tempdir;
	return 0;
}  //void printVar(int varindex)

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

int declaration()
{
	var tempVar;
	int tempSize;
	while(expect("int") || expect("double") || expect("float") || expect("string") || expect("char"))
	{
		if(expect("int"))
		{
			tempSize = 4;
			tempVar.size = 4;
			tempVar.type = 'i';
			tempVar.dir = dataLength;
			tempVar.isArray = false;
		}
		if(expect("double"))
		{
			tempSize = 8;
			tempVar.size = 8;
			tempVar.type = 'd';
			tempVar.dir = dataLength;
			tempVar.isArray = false;
		}
		if(expect("float"))
		{
			tempSize = 4;
			tempVar.size = 4;
			tempVar.type = 'f';
			tempVar.dir = dataLength;
			tempVar.isArray = false;
		}
		if(expect("char"))
		{
			tempSize = 1;
			tempVar.size = 1;
			tempVar.type = 'c';
			tempVar.dir = dataLength;
			tempVar.isArray = false;
		}
		do
		{
			if(!nextToken())
				return 1;
				tempToken = lex->getToken();
			if(isName())
			{
				std::cout << "\"" << tempToken << "\" is already a reserved keyword, line: " << lex->lineCount << " column: " << lex->colCount << std::endl;
				return -1;
			}
			tempVar.name = tempToken;
			lex->nextToken();
			tempToken = lex->getToken();
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
	char tchar = dataLength>>8;
	fout << tchar;
	tchar = dataLength;
	fout << tchar;
	tchar = actualDir>>8;
	fout << tchar;
	tchar = actualDir;
	fout << tchar;


	for(int i = 0; i < actualDir; i++)
		fout << fileBuf[i];
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
	tokens.push_back("%%");
	tokens.push_back("int");
	tokens.push_back("char");
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
		fout.open("C:\\Users\\isai\\Desktop\\compilador\\ModVM\\CK.CHOP", ios::binary | ios::out | ios::trunc);


	if(lex->getError())
	{
		std::cout << "error opening file!\n";
		return 1;
	}
	return 0;
} //void init()