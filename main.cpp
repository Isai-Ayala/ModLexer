#include <iostream>
#include <fstream>
#include <cstring>
#include <cstddef>
#include <string>
#include <stack>
#include <iostream>
#include <cstdlib>
#include "main.h"

int lineCount = 0;
int colCount = 0;
int main(int argc, char *argv[])
{
	std::string line;
	std::ifstream ifs;
	if(argc > 1)
		ifs.open(argv[1]);
	else
		ifs.open("CK.soy");
	if(ifs.is_open())
	{
		while(getline(ifs, line))
		{
			std::cout << line << std::endl; 
		}
		ifs.close();
	}
	return 0;
}  //int main(int argc, char *argv[])