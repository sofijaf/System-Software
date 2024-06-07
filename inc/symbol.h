#ifndef SYMBOL_H
#define SYMBOL_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Symbol
{
private:
  int id;
  string symbolName;
  int value;
  string type;           // SCTN - za sekcije, NOTYPE - za simbole
  int binding;            // 0 - local, 1 - global
  string symbolSection;
  // bool resolved;

  bool isExtern;
  bool isGlobal;
  // bool isDefined;
  // bool isSection;

public:
  static int rednibrojSimbola;

  Symbol() {};
  Symbol(string name, int val, string sect, bool isSctn, bool isGlob, bool isExt)
  {
    id = rednibrojSimbola++;
    symbolName = name;
    value = val;      // ili locCnt!!!!!!!!!! VIDI
    if(isSctn) type = "SCTN";
    else type = "NOTYPE";
    if(isGlob)  binding = 1;
    else        binding = 0;
    symbolSection = sect;
    isExtern = isExt;
    isGlobal = isGlob;
  }

  // Getters & Setters
  Symbol* getSymbol() { return this; }

  int getSymbolID() { return this->id; }

  string getSymbolName() { return this->symbolName; }
  void setSymbolName(string name) { this->symbolName = name; } 

  int getSymbolValue() { return this->value; }
  void setSymbolValue(int val) { this->value = val; }

  string getSymbolType() { return this->type; }

  int getSymbolBinding() { return this->binding; }
  void setSymbolBinding(int bind) { this->binding = bind; }

  string getSymbolSection() { return this->symbolSection; }
  void setSymbolSection(string name) { this->symbolSection = name; }

  // bool getSymbolResolved() { return this->resolved; }
  // void setSymbolResolved(bool res) { this->resolved = res; }

  bool getSymbolExtern() { return this->isExtern; }
  void setSymbolExtern(bool ext) { this->isExtern = ext; }

  bool getSymbolGlobal() { return this->isGlobal; }
  void setSymbolGlobal(bool glob) { this->isGlobal = glob; }
};

#endif