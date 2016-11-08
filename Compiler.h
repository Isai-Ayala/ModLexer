#ifndef MAIN_H
#define MAIN_H

int parsing();
int declaration();
bool expect(std::string token);
int isName();
bool isNumber();
void tokenNames();
void fileHeader();
int codeblock();
int instruction();
int assignment();
int operation();
int factor();
int num();
int init(int argc, char *argv[]);
int vardir();


#endif