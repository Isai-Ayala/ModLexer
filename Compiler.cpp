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

std::string tempToken;
std::ofstream fout;
char fileBuf[10000];
int actualDir = 0;
bool error = false;
char tempchar;
int tempint;

struct var
{
	char type;
	int dir;
	std::string name;
	bool isArray;
	int size;
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
		return 1;

	fileHeader();

	std::cout << "end of program!" << std::endl;
 
	return 0;
}  //int main(int argc, char *argv[])

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
				if(!lex->nextToken())
				{
					std::cout << "end of file not found ";
					return 1;
				}
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

int assignment()
{
	int tempIndex = varDir();
	if(tempDir < 0)
	{
		std::cout << "invalid id at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return 1;
	}
	if(isName != 1 || !lex->nextToken() || lex->getToken() != "=" || !lex->nextToken())
	{
		std::cout << "no valid id found at line " << lex->lineCount << " column " << lex->colCount << std::endl;
		return 1;
	}
	if(operation())
		return 1;
	std::cout << "popvar assy instruction" << std::endl;
	switch(vars[tempIndex].type)
	{
		case 'c':
			if(vars[tempIndex].isArray)
			{
				tempint = 33;
				break;
			}
			tempint = 27;
			break;
		case 'i': 
			if(vars[tempIndex].isArray)
			{
				tempint = 34;
				break;
			}
			tempint = 28;
			break;
		case 'f': 
			if(vars[tempIndex].isArray)
			{
				tempint = 35;
				break;
			}
			tempint = 29;
			break;
		case 'd': 
			if(vars[tempIndex].isArray)
			{
				tempint = 36;
				break;
			}
			tempint = 30;
			break;
		case 's':
			if(vars[tempIndex].isArray)
			{
				tempint = 37;
				break;
			}
			tempint = 31;
			break;
		default: std::cout << "weird variable failure at line " << lex->lineCount << " column " << lex->colCount << std::Endl;
			return 1;
	}
	fileBuf[actualDir++] = tempint;
	tempint = vars[tempIndex].dir;
	fileBuf[actualDir++] = tempint>>8;
	fileBuf[actualDir++] = tempint;
	return 0;
}  //int assignment()

int instruction()
{
	if(!assignment())
	{
		std::cout << "no valid instruction found in file" << std::endl;
		return 1;
	}
	return 0;
}  //int instruction()

int codeblock()
{
	while(instruction())
	{
		if(!lex->nextToken())
		{
			std::cout << "no token found at line " << lex->lineCount << "   column " << lex->colCount << std::endl;
			return 1;
		}
		tempToken = lex->getToken();
	}
	if(error)
		return 1;
	return 0;
}  //int codeblock()

/*
int num(){
  lex -> getToken();
  if(expect("(")){
    if(operation())
      return 1;
    lex -> getToken();
    if(!expect(")"))
      return 1;
  }
  else if(isName() != 1)
  {
	  if(isNumber()){
	    lex -> getToken();
	    //dunno
	  }
	  else
	      return 1;
  }
  else{
    lex -> getToken();
    //dunno
  }
  return 0;
} //int num()

int factor(){
  if(num())
    return 1;
  lex -> getToken();
  while(expect("*") || expect("/") || expect("%%")){
    if(num()) return 1;
    lex -> getToken();
  }
  return 0;
}

int operation(){
  if(factor())
    return 1;
  lex -> getToken();
  while(expect("+") || expect("-")){
    if(factor()) return 1;
    lex -> getToken();
  }
  return 0;
}

int assignment(std::string variable){
  lex -> getToken();
  if(expect("=")){
    if(operation())
      return 1;
    lex -> getToken();
    if(expect(";"))
      return 0;
    else
      return 1;
  }
  else
    return 1;
}

int code(){
  do {
    if(isName()==1){
        if(assignment(tempToken))
          return 1;
    }
  } while(lex -> nextToken());
  return 0;
}*/

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
			tempVar.type = 'i';
			tempVar.dir = dataLength;
			tempVar.isArray = false;
		}
		if(expect("float"))
		{
			tempSize = 4;
			tempVar.size = 4;
			tempVar.type = 'i';
			tempVar.dir = dataLength;
			tempVar.isArray = false;
		}
		if(expect("char"))
		{
			tempSize = 1;
			tempVar.size = 1;
			tempVar.type = 'i';
			tempVar.dir = dataLength;
			tempVar.isArray = false;
		}
		do
		{
			if(lex->nextToken())
				tempToken = lex->getToken();
			else 
			{
				std::cout << "missing id for variable at line: " << lex->lineCount << " column: " << lex->colCount << std::endl;
				return -1;
			}
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
				if(!lex->nextToken())
				{
					std::cout << "no suitable array size found in line: " << lex->lineCount << " colCount: " << lex->colCount << std::endl;
					return -1;
				}
				tempToken = lex->getToken();
				if(!isNumber())
				{
					std::cout << "invalid array size, must be an integer larger than 0, line: " << lex->lineCount << " column: " << lex->colCount << std::endl;
					return -1;
				}
				tempVar.size *= atoi(tempToken.c_str());
				if(!(lex->nextToken()) || !(lex->getToken() == "]") || !(lex->nextToken()))
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
}  //void fileHeader()

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