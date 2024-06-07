#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "symbolTable.h"
#include "sectionTable.h"
#include "parserASM.h"

using namespace std;

class Assembler
{
private:
  // ostream *outFile;
  SymbolTable symTable;
  SectionTable secTable;
  Section* currentSection;

public:
  Assembler();
  ~Assembler();

  static bool prviProlaz;
  static bool drugiProlaz;
  static bool end;

  Section* getCurrentSection() { return currentSection; };
  void setCurrentSectionNaPocetkuObrade(Section* s) { currentSection = s; }

  void obrada(vector<string>);
  void obradaDirektive(string s);
  void obradaInstrukcije(string s);
  void obradaLabele(string s);
  void obradaLiterala(string s);
  // bool stajeU12Bita(string s);
  // int calculateLiteralSizeInBytesForAdresses(string s); // unsigned only - for call, jmp, bgt, bne & beq
  // int calculateLiteralSizeInBytesForData(string s);    // both unsigned and signed - for ld & st

  void obradaDirektiveGlobal(string s);
  void obradaDirektiveExtern(string s);
  void obradaDirektiveSection(string s);
  void obradaDirektiveWord(string s);
  void obradaDirektiveSkip(string s);
  void obradaDirektiveEnd(string s);

  void writeToFile(ostream &os);
  
};

#endif