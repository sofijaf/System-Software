#ifndef SECTION_LD_H
#define SECTION_LD_H

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

#include "literalLD.h"
#include "relocsymLD.h"

using namespace std;

class SectionLD
{
private:
  string name;
  int size;
  int baseAddress;
  vector<unsigned char> data;
  vector<pair<int, unsigned char>> dataWithAddress;
  vector<RelocSymbolLD*> relocationTable;
  vector<LiteralLD*> literalTable;  // bazen literala - on pocinje na onom locCounteru na kom se sekcija zavrsava

  bool placeFlag;
  int locationCounter;  // da li mi treba?
public:
  SectionLD(string name, int size) : name(name), size(size) {}

  // Getters & Setters
  string getName() { return this->name; }
  void setName(string name) { this->name = name; }

  int getSectionSize() { return this->size; }
  void setSectionSize(int sz) { this->size = sz; }

  int getBaseAddress() { return this->baseAddress; }
  void setBaseAddress(int addr) { this->baseAddress = addr; }

  bool getPlaceFlag() { return this->placeFlag; }
  void setPlaceFlag(bool pl) { this->placeFlag = pl; }

  SectionLD* getSection() { return this; }

  vector<RelocSymbolLD*> getRelocationTable() { return this->relocationTable; }
  void setRelocationData(vector<RelocSymbolLD*> r) { this->relocationTable = r; }

  vector<LiteralLD*> getLiteralTable() { return this->literalTable; }
  void setLiteralTable(vector<LiteralLD*> l) { this->literalTable = l;}

  vector<unsigned char> getSectionData() { return this->data; }
  void setSectionData(vector<unsigned char> d) { this->data = d; }

  vector<pair<int, unsigned char>> getDataWithAddress() { return this->dataWithAddress; }
  void setDataWithAddress(vector<pair<int, unsigned char>> d) { this->dataWithAddress = d; }

  void printDataWithAddress(std::ostream& os)
  {
    int l = 0;
    for (auto it = dataWithAddress.begin(); it != dataWithAddress.end(); ++it)
    {
      if (l % 8 == 0)
      {
        if (l != 0) os << endl;
        os << hex << it->first << ": ";
      }

      os << hex << setw(2) << setfill('0') << (int)it->second << " ";
      l++;
    }

    // while (l % 8 != 0)
    // {
    //   os << hex << setw(2) << setfill('0') << 0 << " ";
    //   l++;
    // }
    os << endl;
  }
};


#endif