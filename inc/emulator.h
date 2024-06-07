#ifndef EMULATOR_H
#define EMULATOR_H

#include "../inc/parserEM.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

using namespace std;

class Emulator
{
private:
  bool end;
  // ifstream inputFile;
  string fileName;

  // registri opste namene
  // r14 - sp
  // r15 - pc
  unsigned int gpr[16];

  // control & status registri
  // 0 - status
  // 1 - handler
  // 2 - cause
  unsigned int csr[3];

  // <adresa, 1B>
  vector<pair<unsigned int, unsigned char>> memory;

  vector<pair<unsigned int, unsigned char>> parseInFile();
  pair<unsigned int, unsigned char> searchMemoryForAddress(unsigned int addr);
  void writeByteInMemory(unsigned int addr, unsigned char data);
public:
  ifstream inFile;
  Emulator(string fileName);
  ~Emulator();

  void fillMemory();
  void executeInstruction();
  void write();
  void work();
};


#endif