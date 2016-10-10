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

std::ifstream ifs;
int main(int argc, char *argv[])
{
	std::string line;
	if(argc > 1)
		ifs.open(argv[1]);
	else
		ifs.open("CK.soy");
	if(ifs.is_open())
	{
		if(mainSwitch())
			return 1;
		ifs.close();
	}
	return 0;
}  //int main(int argc, char *argv[])

int mainSwitch()
{
	int scont; //string counter
	while(getline(ifs, line))
	{
		scont = 0;
		while(scont < line.length())
		{
			switch(line[scont])
			{
				case ' ': scont++;
					break;
				case '\n': scont++;
					break;
			}
			if(scont < 0)
				return -1;
			scont++;
		}
	}
	return 0;
}  //void mainSwitch(string line)