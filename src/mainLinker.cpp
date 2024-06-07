#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>
#include <unordered_map>

#include "../inc/parserLD.h"
#include "../inc/linker.h"
// #include "parserLD.cpp"
// #include "linker.cpp"

using namespace std;

// vector<string> fajlZaObradu;
// ifstream inFile;

vector<string> trimPlaceCmd(string line)
{
  vector<string> args = vector<string>();

  // string new_string = deleteWhitespaceFromString(line);
  std::stringstream ss(line);

  while( ss.good() )
  {
    string substr;
    getline( ss, substr, '@' );
    if(regex_match(substr, std::regex("^\\s*$"))) continue;
    else args.push_back( substr );
  }

  return args;
}

int main(int argc,char *argv[])
{
  // std::map<std::string, int> placeSections;
  vector<pair<string,int>> placeSections;
  vector<string> placeSectionsOrder;

  // vector<ifstream*> inFiles = {};
  string outFileName;
  vector<string> inFilesNames;

  bool hexFlag = false;

  // Minimalan broj argumenata cmd linije (ako nema -place) je 5: -hex, -o, out_file.hex i 2 ulazna fajla (minimum)
  if (argc < 5)
  {
    cout << "ERROR: Nedovoljno argumenata komandne linije!" << endl;
    return -1;
  }

  // trazim hexFlag
  for (int i = 0; i < argc; i++)
  {
    if (strcmp(argv[i], "-hex") == 0)
    {
      hexFlag = true;
      break;
    }
  }

  // trazim da li ima -place flegova
  for (int i = 0; i < argc; i++)
  {
    if (((string)argv[i]).find("-place") != string::npos)
    {
      string strtmp = ((string)argv[i]).erase(0,7); // brisem -place=, ostaje sekcija@adresa
      vector<string> args = trimPlaceCmd(strtmp);
      placeSections.emplace_back(args[0], stol(args[1], nullptr, 16));
      // placeSections.insert(::pair<string,int>(args[0], stol(args[1], nullptr, 16)));
      placeSectionsOrder.push_back(args[0]);
    }
  }

  // trazim -o fleg i output fajl iza njega, nakog toga idu input fajlovi
  int j;
  for (int i = 0; i < argc; i++)
  {
    if (strcmp(argv[i], "-o") == 0)
    {
      // na i se nalazi "-o"
      // na i+1 se nalazi izlazni fajl
      // na i+2 pocinju ulazni fajlovi
      outFileName = argv[i+1];
      j = i+2;
      break;
    }
  }

  for (int i = j; i < argc; i++)
  {
    inFilesNames.push_back(argv[i]);
  }

  
  // Kreiranje linkera
  Linker linker(inFilesNames, outFileName, placeSections, placeSectionsOrder);

  // Obrada
  // 1. Parsiranje ulaznih fajlova
  linker.parseInFiles();

  // 2. Redjanje sekcija u zajednicku strukturu
  linker.redjanjeSekcijaUZajednickuStrukturu();

  // 3. Update literal tables
  // linker.updateLiteralTables();

  // 4. Update reloc tables
  linker.updateRelocTables();

  // 5. Update symbol tables
  linker.updateSymbolTables();

  // 6. Punjenje globalne tabele simbola
  linker.fillSymbolTable();
  // linker.printSymbolTable();

  // 7. Spajanje adresa i podataka
  linker.updateSectionData();

  // 8. Prepravljanje adresa iz reloc zapisa
  linker.fixRealocData();

  // 9. Pisanje Data u hex fajl
  linker.printData(linker.outFile);

  return 0;
}