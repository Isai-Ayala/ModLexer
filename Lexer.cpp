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


bool error = false;
bool stringmode = false;
bool charmode = false;

using namespace std;

std::ifstream ifs;
std::string line = "";
std::string token;
lexer::lexer(int argc, char *argv[])
{
	lineCount = 0;
	colCount = 0;
	if(argc > 1)
		ifs.open(argv[1]);
	else
		ifs.open("CK.soy");
	if(!ifs.is_open())
	{
		std::cout << "error opening soy file!\n";
		error = true;
	}
}  //int main(int argc, char *argv[])

bool lexer::getError()
{
	return error;
}

bool lexer::nextToken()
{
	if( line.compare("") && colCount < line.length())
		return true;
	while(getline(ifs, line))
	{
		colCount = 0;
		lineCount++;
		if( line.compare("") && colCount < line.length())
				return true;
	}
	return false;
}  //bool nextToken()

std::string lexer::getToken()
{
	line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());

	token = "";
	if(stringmode && line[colCount] != '"')
	{
		while(line[colCount] != '"' && colCount < line.length())
			token += line[colCount++];
		if(colCount < line.length())
			return token;
		return "error";
	}
	while(line[colCount] == ' ' && colCount < line.length())
	{
		colCount++;
		if(colCount >= line.length())
		{
			if(!nextToken())
				return "";
		}
	}

	if(charmode && line[colCount] != '\'')
	{
		token += line[colCount++];
		if(line[colCount] != '\'')
			return "error";
		return token;
	}

	if((line[colCount] >= 'a' && line[colCount] <= 'z') || (line[colCount] >= 'A' && line[colCount] <= 'Z'))
	{
		token += line[colCount++];
		while((line[colCount] >= '0' && line[colCount] <= '9') || (line[colCount] >= 'a' && line[colCount] <= 'z') || (line[colCount] >= 'A' && line[colCount] <= 'Z'))
		{
			token += line[colCount++];
		}
		return token;
	}

	if(line[colCount] >= '0' && line[colCount] <= '9')
	{
		while(line[colCount] >= '0' && line[colCount] <= '9')
		{
			token += line[colCount++];
		}
		return token;
	}

	if(line[colCount] == '.' || line[colCount] == ';' || line[colCount] == '(' || line[colCount] == ')' || line[colCount] == '{' 
		|| line[colCount] == '}' || line[colCount] == '[' || line[colCount] == ']' || line[colCount] == '/' || line[colCount] == '*' 
		|| line[colCount] == '%' || line[colCount] == ',')
	{
		token += line[colCount++];
		return token;

	}

	switch(line[colCount++])
	{
		case '|':
			if(line[colCount] != '|')
				return "error";
			colCount++;
			return "||";
		break;
		case '&':
			if(line[colCount] != '&')
				return "error";
			colCount++;
			return "&&";
		break;
		case '\'':
			charmode = !charmode;
			return "'";
		break;
		case '"': 
			stringmode = !stringmode;
			return "\"";
		break;
		case '+':
			if(line[colCount] == '+')
			{
				colCount++;
				return "++";
			}
			return "+";
		break;
		case '-':
			if(line[colCount] == '-')
			{
				colCount++;
				//if(colcount > 2 && (line[colcount - 3] == '-' || (colCount >= line.length() && line[colCount] == '-')))
				return "--";
			}
			return "-";
		break;
		case '=':
			if(line[colCount] == '=')
			{
				colCount++;
				return "==";
			}
			return "=";
		break;
		case '!':
			if(line[colCount] == '=')
			{
				colCount++;
				return "!=";
			}
			return "!";
		break;
		case '>':
			if(line[colCount] == '=')
			{
				colCount++;
				return ">=";
			}
			return ">";
		break;
		case '<':
			if(line[colCount] == '=')
			{
				colCount++;
				return "<=";
			}
			return "<";
		break;
	default: return "error";
	break;
	}
	std::cerr << "next token is " << token << "\n";
	return token;
}  //std::string getToken()
