#ifndef PARSERLD_H
#define PARSERLD_H

#include <iostream>
#include <regex>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include "symbolLD.h"
#include "sectionLD.h"

using namespace std;

class ParserLD
{
private:
  vector<SymbolLD*> symTable;
  vector<SectionLD*> secTable;

  ifstream inFile;
  string fileName;

  vector<string> splitLineInTokens(string line);
  string deleteWhitespaceFromString(string s);
public:
  ParserLD(string fileName);
  void parse();

  vector<SymbolLD*> getSymbolTable() { return this->symTable; }
  void setSymbolTable(vector<SymbolLD*> st) { this->symTable = st; }

  vector<SectionLD*> getSectionTable() { return this->secTable; }

  SectionLD* findSectionByName(string name);
  SymbolLD* findSymbolByName(string name);

  string getFileName() { return fileName; }

  ParserLD* getParser() { return this; }    // pitanje da li mi treba
};

#endif