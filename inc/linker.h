#ifndef LINKER_H
#define LINKER_H

#include <iostream>
#include <regex>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <iomanip>

#include "../inc/symbolLD.h"
#include "../inc/sectionLD.h"
#include "../inc/parserLD.h"

using namespace std;

class Linker
{
private:
  bool hexFlag;
  int placeFlagCnt;   // koliko puta se place opcija pojavljuje u komandnoj liniji

  static int id;

  vector<ParserLD*> parsers;
  
  vector<ifstream> inFiles;
  vector<string> inFilesNames;
  string outFileName;

  vector<unsigned char> outData;

  vector<SymbolLD*> symTable;
  // std::map<string, std::map<SectionLD*, int>> sections; // string - ime fajla, SectionLD* - sekcija, int - adresa sekcije
  vector<pair<string, pair<SectionLD*,int>>> sections;
  vector<pair<string,int>> placeSections;
  // std::map<std::string, int> placeSections;
  
  vector<string> placeSectionsOrder;

  SymbolLD* findSymbolByNameInLinker(string name);

  ParserLD* findParserByFileName(string fileName);
public:
  ofstream outFile;
  Linker();
  Linker(vector<string> inNames, string outName, vector<pair<string,int>> ps, vector<string> psOrder);
  ~Linker();

  void fillSymbolTable();
  void parseInFiles();
  void printSymbolTable();
  void redjanjeSekcijaUZajednickuStrukturu();

  // update internih struktura svih sekcija, pre spajanja u hex fajl
  void updateRelocTables();
  void updateLiteralTables();
  void updateSymbolTables();
  void updateSectionData();
  void fixRealocData();

  void printData(ostream& os);
};

#endif