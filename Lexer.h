#ifndef lexer_H
#define lexer_H
#include <string>

class lexer
{

public:
	lexer(int argc, char *argv[]);
	int mainSwitch();
	void tokenNames();
	bool nextToken();
	std::string getToken();

	bool getError();
	int lineCount;
	int colCount;

};

#endif