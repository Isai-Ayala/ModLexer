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

struct var
{
	char type;
	int dir;
};

std::vector<var> vars(); 
lexer *lex;

int dataLength = 0;


int main(int argc, char *argv[])
{
	lex = new lexer(argc, argv);
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

int declaration()
{
	var tempVar;
	while(expect("int") || expect("double") || expect("float") || expect("string") || expect("char"))
	{
		if(tempToken.compare("int") == 0)
		{
			var.type = 'i';
			var.dir = dataLength;
			dataLength += 4; //HOOOOOOOOOOOOOOOOOOOOOOOOLD

		}

	}
	return 0;
}  //int declaration()

int parsing()
{
	tempToken = lex->getToken();
	if(declaration())
		return 1;
	while(expect("END"))
	{

		if(!lex->nextToken())
		{
			std::cout << "end of file not found";
			return 1;
		}
		tempToken = lex->getToken();
	}
	return 0;
}  //int parsing()

bool expect(std::string token)
{
	if(tempToken.compare(token))
		return false;
	return true;
}  //bool expect(std::string token)

/*
void tokenNames()
{
	tokens[0] = ";";
	tokens[1] = "(";
	tokens[2] = ")";
	tokens[3] = "{";
	tokens[4] = "}";
	tokens[5] = "[";
	tokens[6] = "]";
	tokens[7] = "+";
	tokens[8] = "-";
	tokens[9] = "*";
	tokens[10] = "/";
	tokens[12] = "%%";
	tokens[13] = "int";
	tokens[14] = "char";
	tokens[15] = "float";
	tokens[16] = "double";
	tokens[17] = "string";
	tokens[18] = "if";
	tokens[19] = "else";
	tokens[20] = "for";
	tokens[21] = "while";
	tokens[22] = "&&";
	tokens[23] = "||";
	tokens[24] = "!";
	tokens[25] = "==";
	tokens[26] = "!=";
	tokens[27] = ">=";
	tokens[28] = "<=";
	tokens[29] = ">";
	tokens[30] = "<";
	tokens[31] = "=";
}  //void tokenNames()*/
