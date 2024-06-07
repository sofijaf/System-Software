#ifndef RELOC_SYM_H
#define RELOC_SYM_H

#include "symbol.h"

class RelocSymbol
{
private:
  Symbol* mySymbol;
  int offset;   // offset od pocetka sekcije, sekcija pocinje od 0
  int index;

public:
  RelocSymbol(Symbol* s, int o, int i)
  {
    this->mySymbol = s;
    this->offset = o;
    this->index = i;
  }

  RelocSymbol(Symbol* s, int o)
  {
    this->mySymbol = s;
    this->offset = o;
    this->index -1;
  }

  // Getters & Setters

  int getOffset() { return this->offset; }
  void setOffset(int o) { this->offset = o; }

  Symbol* getSymbol() { return this->mySymbol; }

  int getIndex() { return index; }
  void setIndex(int i) { this->index = i; }
};

#endif