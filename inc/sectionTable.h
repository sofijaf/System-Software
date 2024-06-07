#ifndef SECTION_TABLE_H
#define SECTION_TABLE_H

#include <iostream>
#include <string>
#include <vector>
#include "section.h"

using namespace std;

class SectionTable
{
private:
  vector<Section*> sectionTable;
  
public:
  SectionTable()
  {
    sectionTable = vector<Section*>();
  }

  void addSectionToSectionTable(Section *sec)
  {
    sectionTable.push_back(sec);
  }

  Section* findSectionByName(string name)
  {
    for(auto i = sectionTable.begin(); i != sectionTable.end(); ++i)
    {
      if((*i)->getName() == name)
      {
        return (*i)->getSection();
      }
    }
    return nullptr;
  }

  // PRIVREMENO
  void printSectionTable(std::ostream& os) {
    
    for(int i=0; i<sectionTable.size(); i++) {
      os << std::internal << std::setw(10) << sectionTable[i]->getName();
      os << std::internal << std::setw(10) << sectionTable[i]->getSectionSize();
      os << endl;
    }
  }

  void printLiteralTables(std::ostream& os)
  {
    for (int i=0; i < sectionTable.size(); i++) {
      if(sectionTable.at(i)->getLiteralTable()->size() != 0)
      {
        os << "LiteralTable " << sectionTable.at(i)->getName() << endl;
        sectionTable.at(i)->printLiteralTable(os);
        os << endl;
      }
    }
  }

  void printRelocationTables(std::ostream& os)
  {
    for (int i = 0; i < sectionTable.size(); i++)
    {
      if (sectionTable.at(i)->getRelocationTable().size() != 0)
      {
        os << "RelocTable " << sectionTable.at(i)->getName() << endl;
        sectionTable.at(i)->printRelocationTable(os);
        os << endl;
      }
    }
  }

  void printAllSectionData(std::ostream& os)
  {
    for (int i = 0; i < sectionTable.size(); i++)
    {
      if (sectionTable.at(i)->getSectionData().size() != 0)
      {
        os << "Data " << sectionTable.at(i)->getName();
        sectionTable.at(i)->printData(os);
        // os << endl;
      }

      if (sectionTable.at(i)->getLiteralTable()->size() != 0)
      {
        sectionTable.at(i)->printLiteralTableData(os);
        // os << endl;
      }
    }
  }
};

#endif