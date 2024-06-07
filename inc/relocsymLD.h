#ifndef RELOC_SYM_LD_H
#define RELOC_SYM_LD_H

#include "symbolLD.h"

class RelocSymbolLD
{
private:
  SymbolLD* mySymbol;
  int offset;   // offset od pocetka sekcije, sekcija pocinje od 0

public:
  RelocSymbolLD(SymbolLD* s, int o)
  {
    this->mySymbol = s;
    this->offset = o;
  }

  // Getters & Setters

  int getOffset() { return this->offset; }
  void setOffset(int o) { this->offset = o; }

  SymbolLD* getSymbol() { return this->mySymbol; }
};

#endif