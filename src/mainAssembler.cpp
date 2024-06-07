#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>

#include "../inc/assembler.h"

using namespace std;

ifstream inFile;
ofstream outFile;
int kraj = 0;
int position = 0;
vector<string> fajlZaObradu;

int main(int argc, char *argv[]){

    string inFileName;
    string outFileName;

    if(argc !=4 || strcmp(argv[1],"-o")!=0){
        cout << "ERROR! Nedovoljno argumenata komandne linije!" << endl;
        return -1;
    }
    inFileName = argv[3];
    outFileName = argv[2];

    inFile.open(inFileName);
    outFile.open(outFileName);

    if (!inFile.is_open())
    {
      cout << "ERROR! Ulazni fajl nije moguce otvoriti!" << endl;
      return -2;
    }    // else
    // {
    //   cout << "Ulazni fajl uspesno otvoren!" << endl;
    // }
   
    if (!outFile.is_open())
    {
      cout << "ERROR! Izlazni fajl nije moguce otvoriti!" << endl;
      return -3;
    } //else
    // {
    //   cout << "Izlazni fajl uspesno otvoren!" << endl;
    // }

    Assembler assembler;
  
    fajlZaObradu = prepareTextForAssembling(inFile);

    Assembler::prviProlaz = true;
    assembler.obrada(fajlZaObradu);
    
    // cout << "Prvi prolaz zavrsen!" << endl;

    // ------------------------------------------- KRAJ PRVOG PROLAZA --------------------------------------------------

    Assembler::drugiProlaz = true;
    Assembler::end = false;
    assembler.obrada(fajlZaObradu);

    // cout << "Drugi prolaz zavrsen!" << endl;

    assembler.writeToFile(outFile);

    return 0;
}