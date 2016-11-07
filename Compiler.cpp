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

	if(lex -> nextToken() && lex -> getToken().compare("BEGIN") == 0)
	{
		if(lex->nextToken())
		{
			if(parsing())
				return 1;
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

	std::cout << "end of program!" << std::endl;
 
	return 0;
}  //int main(int argc, char *argv[])

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
				if(lex->nextToken() && lex->getToken() == "]" && lex->nextToken())
				{
					tempToken = lex->getToken();
					vars.push_back(tempVar);
				}
				else
				{
					std::cout << "syntax error at line: " << lex->lineCount << "  column: " << lex->colCount << std::endl;
					return -1;
				}
			}
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
	std::cout << "data segment: " << dataLength << std::endl;
	return 0;
}  //int declaration()

bool isNumber()
{
	for(int i = 0; i < tempToken.length(); i++)
		if(tempToken[i] < '0' || tempToken[i] > '9')
			return false;
	return true;
}  //bool isNumber()

int parsing()
{
	tempToken = lex->getToken();
	if(declaration())
		return 1;
	while(!expect("END"))
	{

		if(!lex->nextToken())
		{
			std::cout << "end of file not found ";
			return 1;
		}
	}
	return 0;
}  //int parsing()

bool expect(std::string token)
{
	if(tempToken.compare(token))
		return false;
	return true;
}  //bool expect(std::string token)


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
