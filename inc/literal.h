#ifndef LITERAL_H
#define LITERAL_H

#include <string>
#include <iostream>

using namespace std;

class Literal
{
private:
  string valueStr;
  unsigned int valueInt;
  int size;
  int location;
  Symbol* sym;
  int index;

public:
  Literal(string valStr, unsigned int valInt, int sz, int loc, Symbol* sym, int index)
  {
    this->valueStr = valStr;
    this->valueInt = valInt;
    this->size = sz;
    this->location = loc;
    this->sym = sym;
    this->index = index;
  }

  Literal(string valStr, unsigned int valInt, int sz, int loc)
  {
    this->valueStr = valStr;
    this->valueInt = valInt;
    this->size = sz;
    this->location = loc;
    this->sym = nullptr;
    this->index = -1;
  }

  // Getters & Setters
  string getLiteralValueString() { return this->valueStr; }
  void setLiteralValueString(string val) { this->valueStr = val; }

  int getLiteralValueInt() { return this->valueInt; }
  void setLiteralValueInt(unsigned int val) { this->valueInt = val; }

  int getLiteralSize() { return this->size; }
  void setLiteralSize(int sz) { this->size = sz;}

  int getLiteralLocation() { return this->location; }
  void setLiteralLocation(int loc) { this->location = loc; }

  Symbol* getLiteralSymbol() { return this->sym; }
  void setLiteralSymbol(Symbol* s) { this->sym = s; }

  int getLiteralIndex() { return this->index; }
  void setLiteralIndex(int i) { this->index = i; }
};

#endif