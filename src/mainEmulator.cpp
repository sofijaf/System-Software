#include "../inc/emulator.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
  string inFileName;

  if (argc != 2) 
  {
    cout << "ERROR: Neispravni argumenati komandne linije!" << endl;
    return -1;
  }

  inFileName = argv[1];

  // cout << inFileName << endl;

  Emulator emulator(inFileName);

  emulator.fillMemory();

  emulator.work();

  emulator.write();
}