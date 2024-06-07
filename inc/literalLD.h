#ifndef LITERAL_LD_H
#define LITERAL_LD_H

#include <string>
#include <iostream>

using namespace std;

class LiteralLD
{
private:
  string valueStr;
  int valueInt;
  int size;
  int location;

public:
  LiteralLD(string valStr, int valInt, int sz, int loc)
  {
    this->valueStr = valStr;
    this->valueInt = valInt;
    this->size = sz;
    this->location = loc;
  }

  // Getters & Setters
  string getLiteralValueString() { return this->valueStr; }
  void setLiteralValueString(string val) { this->valueStr = val; }

  int getLiteralValueInt() { return this->valueInt; }
  void setLiteralValueInt(int val) { this->valueInt = val; }

  int getLiteralSize() { return this->size; }
  void setLiteralSize(int sz) { this->size = sz;}

  int getLiteralLocation() { return this->location; }
  void setLiteralLocation(int loc) { this->location = loc; }
};

#endif