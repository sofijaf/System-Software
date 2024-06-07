#ifndef SYMBOL_LD_H
#define SYMBOL_LD_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class SymbolLD
{
private:
  int id;
  string name;
  string section;
  string bind;
  int value;
  string type;

public:
  SymbolLD(int id, string name, string section, string bind, int value, string type) : id(id), name(name), section(section), bind(bind), value(value), type(type) {}

  // Getters & Setters
  int getID() { return this->id; }
  void setID(int id) { this->id = id; }

  string getName() { return this->name; }
  void setName(string n) { this->name = n; }

  string getSection() { return this->section; }
  void setSection(string sec) { this->section = sec; }

  string getBind() { return this->bind; }
  void setBind(string b) { this->bind = b; }

  int getValue() { return this->value; }
  void setValue(int v) { this->value = v; }

  string getType() { return this->type; }
  void setType(string t) { this->type = t; }

  SymbolLD* getSymbol() { return this; }
};


#endif