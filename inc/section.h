#ifndef SECTION_H
#define SECTION_H

#include <iostream>
#include <string>
#include <vector>
#include "relocsym.h"
#include "literal.h"
#include <iomanip>

using namespace std;

class Section
{
private:
  string name;
  int size;
  vector<unsigned char> data;
  vector<RelocSymbol*> relocationTable;
  vector<Literal*> literalTable;  // bazen literala - on pocinje na onom locCounteru na kom se sekcija zavrsava

  int locationCounter;  // svaka sekcija ima svoj locationCounter koji pocinje od 0

public:
  Section() {};

  Section(string name) : literalTable(), relocationTable()
  {
    this->name = name;
    this->locationCounter = 0;
    this->size = 0;
  }

  // Getters & Setters

  string getName() { return this->name; }
  void setName(string name) { this->name = name; }

  int getSectionSize() { return this->size; }
  void setSectionSize(int sz) { this->size = sz; }

  Section* getSection() { return this; }

  int getLocCounter() { return this->locationCounter; }

  void incLocationCounter() { this->locationCounter++; }

  void incLocationCounterFor(int arg) { this->locationCounter += arg; }

  vector<unsigned char> getSectionData() { return data; }
  void setSectionData(vector<unsigned char> d) { data = d; }

  vector<RelocSymbol*> getRelocationTable() { return this->relocationTable; }
  void setRelocationData(vector<RelocSymbol*> r) { relocationTable = r; }

  void resetLocationCounter() { this->locationCounter = 0; }

  vector<Literal*> *getLiteralTable() { return &(this->literalTable); }

  bool entryExistsInLiteralTable(int value)
  {
    bool ret = false;
    for(auto i = this->literalTable.begin(); i != this->literalTable.end(); ++i)
    {
      if((*i)->getLiteralValueInt() == value)
      {
        ret = true;
        break;
      }
    }
    return ret;
  }

  int getLiteralLocation(int value)
  {
    int loc = 0;
    for(auto i = this->literalTable.begin(); i != this->literalTable.end(); ++i)
    {
      if((*i)->getLiteralValueInt() == value)
      {
        loc = (*i)->getLiteralLocation();
        break;
      }
    }
    return loc;
  }

  void updateLiteralTable()
  {
    // literalTable.at(0)->setLiteralLocation(this->locationCounter);
    // incLocationCounterFor(literalTable.at(0)->getLiteralSize());
    // int addr = this->getSectionData().size();
    
    for (int i = 0; i < literalTable.size(); i++)
    {
      literalTable.at(i)->setLiteralLocation(this->locationCounter);
      incLocationCounterFor(literalTable.at(i)->getLiteralSize());
      // literalTable.at(i)->setLiteralLocation(addr);
      // addr+=4;
    }
  }

  void updateRelocationTable()
  {
    // int addr = this->getSectionData().size();
    for (int i = 0; i < relocationTable.size(); i++)
    {
      for (int j = 0; j < literalTable.size(); j++)
      {
        if (literalTable[j]->getLiteralSymbol() == nullptr) continue;
        if (literalTable[j]->getLiteralSymbol()->getSymbolName().compare(relocationTable[i]->getSymbol()->getSymbolName()) == 0 &&
          (literalTable[j]->getLiteralIndex() == relocationTable[i]->getIndex()))
        {
          relocationTable[i]->setOffset(literalTable[j]->getLiteralLocation());
          
        }
      }
    }
  }

  // returns location of a new entry
  vector<pair<int,int>> addNewEntryAtTheEndOfLiteralTable(Symbol* sym)
  {
    int newLoc;
    int newIndex;
    vector<pair<int,int>> addrIndex;
    if (!literalTable.empty())
    {
      int lastLoc = literalTable.at(literalTable.size()-1)->getLiteralLocation();
      int lastIndex = literalTable.at(literalTable.size()-1)->getLiteralIndex();
      newIndex = lastIndex + 1;
      newLoc = lastLoc + 4;
      Literal* l = new Literal(to_string(0), 0, 4, newLoc, sym, newIndex);
      literalTable.push_back(l);
    } else
    {
      // ako je prazna tabela literala, zar ne treba da dodam na adresu 0?
      // ne, dodacu na velicinu sekcije iz prvog prolaza
      // newLoc = 0;
      newLoc = getSectionSize();
      newIndex = 0;
      Literal* l = new Literal(to_string(0), 0, 4, newLoc, sym, newIndex);
      literalTable.push_back(l);
    }

    addrIndex.emplace_back(std::make_pair(newLoc,newIndex));
    return addrIndex;
  }

  // print i ostale pomocne fje

  void printLiteralTable(std::ostream& os)
  {
    // os << std::internal << std::setw(15) << "Value (str)";
    // os << std::internal << std::setw(15) << "Value (int)";
    // os << std::internal << std::setw(10) << "Size";
    // os << std::internal << std::setw(10) << "Location" << endl;
    for (int i = 0; i < literalTable.size(); i++)
    {
      os << std::internal << std::setw(15) << literalTable.at(i)->getLiteralValueString();
      os << std::internal << std::setw(15) << literalTable.at(i)->getLiteralValueInt();
      os << std::internal << std::setw(10) << literalTable.at(i)->getLiteralSize();
      os << std::internal << std::setw(10) << literalTable.at(i)->getLiteralLocation();
      if (literalTable[i]->getLiteralSymbol() != nullptr)
      {
        os << std::internal << std::setw(20) << literalTable.at(i)->getLiteralSymbol()->getSymbolName();
        os << std::internal << std::setw(10) << literalTable.at(i)->getLiteralIndex();
      }

      os << endl;
    }
  }

  void printRelocationTable(std::ostream& os)
  {
    // os << std::internal << std::setw(10) << "Symbol";
    // os << std::internal << std::setw(10) << "Offset" << endl;
    for (int i = 0; i < relocationTable.size(); i++)
    {
      os << std::internal << std::setw(20) << relocationTable.at(i)->getSymbol()->getSymbolName();
      os << std::internal << std::setw(10) << relocationTable.at(i)->getOffset();
      os << std::internal << std::setw(10) << relocationTable.at(i)->getIndex() << endl;
    }
  }

  void printData(std::ostream& os)
  {
    for (int i = 0; i < data.size(); i++)
    {
      if (i % 4 == 0)
        os << endl;
      os << hex << setw(2) << setfill('0') << (int)data.at(i) << " ";
    }
    os << endl;
  }

  void printLiteralTableData(std::ostream& os)
  {
    unsigned char byte1;
    unsigned char byte2;
    unsigned char byte3;
    unsigned char byte4;
    for (int i = 0; i < literalTable.size(); i++)
    {
      int val = literalTable.at(i)->getLiteralValueInt();
      byte1 = val & 0xFF;
      byte2 = (val >> 8) & 0xFF;
      byte3 = (val >> 16) & 0xFF;
      byte4 = (val >> 24) & 0xFF;

      // if (i % 4 == 0) os << endl;
      os << hex << setw(2) << setfill('0') << (int)byte1 << " " << hex << setw(2) << setfill('0') << (int)byte2 << " " << hex << setw(2) << setfill('0') << (int)byte3 << " " << hex << setw(2) << setfill('0') << (int)byte4 << endl;
    }
    os << endl;
  }
};

#endif