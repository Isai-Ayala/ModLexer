#include <iostream>
#include <tuple>

struct Transition{
  std::tuple<int,int> trans;
  int destination;

public:
  Transition(int,int,int);
  int getState() { return std::get<0>(trans);}
  int getStep() { return std::get<1>(trans);}
  int getDest() { return destination;}
  void setValues(int state, int step, int dest) {trans = std::make_tuple(state,step); destination = dest;}
};

Transition::Transition(int state, int step, int dest) {
  trans = std::make_tuple (state,step);
  destination = dest;
}


class Parser {
  enum STATES {A1,A2,A3,A4};
  enum TOKENS {ID,SEMI,PAREN_INI,PAREN_END,CB_INI,CB_END,SB_INI,SB_END,ADD,SUB,MUL,DIV,MOD,INT,CHAR,FLOAT,DOUBLE,STRING,IF,ELSE,FOR,WHILE,AND,OR,NOT,EQUALS,NEQUALS,BIGGEREQ,LESSEQ,BIGGER,LESS,ASSIGN,NUMBER,DECIMAL};
  std::vector<Transition> transitionFunction(100);
  int currentState;
  std::string error;
public:
  int getCurrent() { return currentState;}
  std::string getError() {return error;}
  int moveState(int token) {
    int index = 0;
    while(index++<transitionFunction.length()){
      if(transitionFunction[index].getState() == currentState)
        if(transitionFunction[index].getStep() == token){
          currentState = transitionFunction[index].getDest();
          return currentState;
        }
    }
    error = "Invalid token.";
    return -1;
  }

};

Parser::Parser() {
  currentState = A1;
  transitionFunction[0].setValues(A1,INT,A2);
  transitionFunction[1].setValues(A2,ID,A3);
  transitionFunction[2].setValues(A3,COMMA,A2);
  transitionFunction[3].setValues(A3,SEMI,A1);
}
