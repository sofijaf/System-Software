#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <iostream>
#include <string>
#include <vector>
#include "symbol.h"
#include <iomanip>

using namespace std;

class SymbolTable
{
private:
  vector<Symbol*> symbolTable;
public:
  SymbolTable()
  {
    symbolTable = vector<Symbol*>();
  }

  void addSymboltoSymbolTable(Symbol* s)
  {
    symbolTable.push_back(s);
  }

  Symbol* findSymbolByName(string name)
  {
    for(auto i = symbolTable.begin(); i != symbolTable.end(); ++i)
    {
      if((*i)->getSymbolName() == name)
      {
        return (*i)->getSymbol();
      }
    }
    return nullptr;
  }

// PRIVREMENO
void printSymbolTable(std::ostream& os) {
  for(int i=0; i<symbolTable.size(); i++) {
    os << std::internal << std::setw(5) << symbolTable.at(i)->getSymbolID();
    os << std::internal << std::setw(20) << symbolTable.at(i)->getSymbolName();
    os << std::internal << std::setw(20) << symbolTable.at(i)->getSymbolSection();
    if (symbolTable.at(i)->getSymbolBinding() == 1)
      os << std::internal << std::setw(10) << "GLOB";
    else if(symbolTable.at(i)->getSymbolBinding() == 0)
      os << std::internal << std::setw(10) << "LOC";
    os << std::internal << std::setw(10) << symbolTable.at(i)->getSymbolValue();
    os << std::internal << std::setw(10) << symbolTable.at(i)->getSymbolType();
    // cout<<"Value:";
    // outfile<<" "<<decimalToHex(std::to_string(symbolTable.at(i).getSymbolValue()));
    os <<std::endl;
  }
}
};

#endif