#include "../inc/linker.h"

int Linker::id = 0;

Linker::Linker(vector<string> i, string o, vector<pair<string,int>> ps, vector<string> psOrder) : inFilesNames(i), outFileName(o), placeSections(ps), placeSectionsOrder(psOrder)
{
  outFile = ofstream(outFileName);

  if (!outFile.is_open())
  {
    cout << "ERROR: Izlazni fajl nije moguce otvoriti!" << endl;
    exit(-1);
  }
}

Linker::~Linker() {}

void Linker::parseInFiles()
{
  for (int i = 0; i < inFilesNames.size(); i++)
  {
    ParserLD* parser = new ParserLD(inFilesNames[i]);
    parsers.push_back(parser);
    parser->parse();
    // cout << "PARSING FILE " << inFilesNames[i] << " DONE" << endl;
  }
}

ParserLD* Linker::findParserByFileName(string fileName)
{
  for (int i = 0; i < parsers.size(); i++)
  {
    if ((parsers[i]->getFileName()).compare(fileName) == 0)
    {
      return parsers[i];
      break;
    }
  }
  return nullptr;
}

void Linker::redjanjeSekcijaUZajednickuStrukturu()
{
  int poslednjaDodataAdresa;
  int velicinaPoslednjeSekcije;

  // Sekcije sa -place opcijom
  for (auto it = placeSections.begin(); it != placeSections.end(); ++it)
  {
    string sectionName = it->first;
    int address = it->second;

    for (int i = 0; i < parsers.size(); i++)
    {
      // Proveravam da li sekcija sa sectionName postoji u fajlu parser[i]
      SectionLD* sec = parsers[i]->findSectionByName(sectionName);

      if (sec)
      {
        // Postoji, dodajem je u sections sa imenom tog fajla
        // cout << "sec " << sec->getName() << " exists in file " << parsers[i]->getFileName() << endl;

        if (sections.empty())
        {
          // nema nista u sections, samo je dodajem na pocetak
          sections.emplace_back(parsers[i]->getFileName(), std::make_pair(sec, address));
        } else
        {
          // vec ima nesto u sections
          auto last_elem = sections.back();

          // Provera da li je sekcija iz last_elem ISTA kao sekcija sa sectionName
          if (last_elem.second.first->getName().compare(sectionName) == 0)
          {
            // iste su, ovu dodajem na kraj prethodne
            address = last_elem.second.second + last_elem.second.first->getSectionSize();
          }

          sections.emplace_back(parsers[i]->getFileName(), std::make_pair(sec, address));
        }
      }
      // } else
      // {
      //   // Ne postoji, idemo dalje
        // cout << "section " << sectionName << " doesn't exist in file " << parsers[i]->getFileName() << endl;
      // }
    }
  }

  // Ubacivanje sekcija sa -place opcijom GOTOVO
  // Prolazim kroz sve fajlove onim redom kojim su navedeni u cmd liniji
  // Prolazim kroz tabelu sekcija svakog fajla i dodajem sekcije u sections

  // Dohvatam fajl po fajl
  for (int i = 0; i < parsers.size(); i++)
  {
    string fileName = parsers[i]->getFileName();
    vector<SectionLD*> sectionTable = parsers[i]->getSectionTable();

    // Prolazim kroz tabelu sekcija fajla sa imenom fileName
    for (int j = 0; j < sectionTable.size(); j++)
    {
      SectionLD* secTmp = sectionTable[j];

      // Ako je sekcija UND, samo prelazim na sledeci element
      if (secTmp->getName().compare("UND") == 0) continue;

      // Proveravam da li se sekcija secTmp iz fajla fileName vec nalazi u sections
      // Ako se nalazi u toj kombinaciji, preskacem

      // Ako prodje celu ovu for petlju, znaci da u toj kombinaciji ne postoji u sections
      bool combFileSecAlreadyExists = false;
      for (auto k = sections.begin(); k != sections.end(); ++k)
      {
        if (k->first.compare(fileName) == 0 && k->second.first->getName().compare(secTmp->getName()) == 0)
        {
          combFileSecAlreadyExists = true;
          break;
        }
      }
      // Ako ta kombinacija vec postoji, ne radim vise obradu za tu sekciju, vec idem na sledecu iteraciju
      if (combFileSecAlreadyExists) continue;


      // Proveravam da li se sekcija secTmp nalazi u sections, ali iz nekog drugog fajla
      // Ako se nalazi, treba da merge-ujem
      // Ako se ne nalazi, dodajem je samo na kraj
      bool exists =  false;
      pair<string, pair<SectionLD*,int>> elem;
      for (auto k = sections.begin(); k != sections.end(); ++k)
      {
        if (k->second.first->getName().compare(secTmp->getName()) == 0)
        {
          // cuvam mesto iz sections gde sam je nasla
          elem = make_pair(k->first, make_pair(k->second.first, k->second.second));
          exists = true;
          // break;
          // cim je nadjem, break-ujem
          // ne treba da break-ujem cim je nadjem, jer tako uvek uzimam prvo pojavljivanje te sekcije, a treba mi poslednje!!
        }
      }

      if (!exists)  // Ne postoji, dodajem je iza poslednje
      {
        auto last_elem = sections.back();
        auto address = last_elem.second.second + last_elem.second.first->getSectionSize();
        sections.emplace_back(fileName, std::make_pair(secTmp, address));
      } else
      {
        // cout << "ovde bakcem" << endl;
        // Postoji, dodajem je tacno iza elem, gde sam je nasla
        auto address = elem.second.second + elem.second.first->getSectionSize();
        sections.emplace_back(fileName, std::make_pair(secTmp, address));
      }
    }
  }

  // ----------------------------------------------------------------------------

  // PROVERA: Kako su sekcije poredjane u 
  // for (auto itr = sections.begin(); itr != sections.end(); itr++) {  
  //       cout << "First key is " << itr->first
  //             << " And second key is " << itr->second.first->getName() << ", with size: " << hex << itr->second.first->getSectionSize()
  //             << " And value is " << hex << itr->second.second << endl;
  // }
}

void Linker::updateRelocTables()
{
  // Prolazim kroz sections i svakoj sekciji radim update reloc tabela

  string fileName;
  SectionLD* sec;
  int secStartAddr;
  
  for (auto it = sections.begin(); it != sections.end(); ++it)
  {
    fileName = it->first;
    sec = it->second.first;
    secStartAddr = it->second.second;

    // cout << "File: " << fileName << ", secName: " << sec->getName() << ", secStartAddr: " << secStartAddr << endl;

    vector<RelocSymbolLD*> relTable = sec->getRelocationTable();

    if (!relTable.empty())
    {
      for (int i = 0; i < relTable.size(); i++)
      {
        // cout << relTable[i]->getSymbol()->getName() << ", ";
        int offset = relTable[i]->getOffset();
        // cout << "old offset: " << offset << ", ";
        offset += secStartAddr;

        relTable[i]->setOffset(offset);
        // cout << "new offset: " << hex << relTable[i]->getOffset() << endl;
      }

      sec->setRelocationData(relTable);
    }    
  }
}

void Linker::updateLiteralTables()
{
  // Prolazim kroz sections i svakoj sekciji radim update tabela literala

  string fileName;
  SectionLD* sec;
  int secStartAddr;

  for (auto it = sections.begin(); it != sections.end(); ++it)
  {
    fileName = it->first;
    sec = it->second.first;
    secStartAddr = it->second.second;

    // cout << "File: " << fileName << ", secName: " << sec->getName() << ", secStartAddr: " << secStartAddr << endl;

    vector<LiteralLD*> litTable = sec->getLiteralTable();

    if (!litTable.empty())
    {
      for (int i = 0; i < litTable.size(); i++)
      {
        // cout << litTable[i]->getLiteralValueString() << endl;

        int literalLocation = litTable[i]->getLiteralLocation();
        // cout << "old loc: " << literalLocation << endl;
        literalLocation += secStartAddr;

        litTable[i]->setLiteralLocation(literalLocation);
        // cout << "new loc: " << litTable[i]->getLiteralLocation() << endl;
      }

      sec->setLiteralTable(litTable);
    }
  }
}

void Linker::updateSymbolTables()
{
  string parserFileName;
  string sectionsFileName;
  SectionLD* sectionsSec;
  int sectionsSecStartAddr;

  // Prolazim kroz sve parsere i prepravljam tabele simbola, jednu po jednu
  for (int i = 0; i < parsers.size(); i++)
  {
    parserFileName = parsers[i]->getFileName();

    // tabela simbola fajla i
    vector<SymbolLD*> symTable = parsers[i]->getSymbolTable();
    
    // print za proveru
    // cout << "symbTable pre update, file: " << parserFileName << endl;
    // for (int k = 0; k < symTable.size();k++)
    // {
    //   cout << std::internal << std::setw(5) << symTable.at(k)->getID();
    //   cout << std::internal << std::setw(20) << symTable.at(k)->getName();
    //   cout << std::internal << std::setw(20) << symTable.at(k)->getSection();
    //   cout << std::internal << std::setw(10) << symTable.at(k)->getBind();
    //   cout << std::internal << std::setw(10) << hex << symTable.at(k)->getValue();
    //   cout << std::internal << std::setw(10) << symTable.at(k)->getType();
    //   // cout<<"Value:";
    //   // outfile<<" "<<decimalToHex(std::to_string(symbolTable.at(i).getSymbolValue()));
    //   cout <<std::endl;
    // }

    // prolazim kroz tabelu simbola
    // za simbole kojima sekcija NIJE UND:
    // value sabiram sa velicinom prethodne sekcije
    // value sabiram sa pocetnom adresom sekcije kojoj taj simbol pripada - NE!!!!!!!!!!
    // pocetnu adresu sekcije moram naci u sections -> moraju da se poklope i ime fajla i ime sekcije
    for (int j = 0; j < symTable.size(); j++)
    {
      string symbSection = symTable[j]->getSection();

      if (symbSection.compare("UND") == 0) continue;

      int symbValue = symTable[j]->getValue();

      // Sekcije drugacije prepravljam - njima za value stavljam pocetnu adresu
      // if (symTable[j]->getType().compare("SCTN") == 0)
      // {
      for (auto it = sections.begin(); it != sections.end(); ++it)
      {
        sectionsFileName = it->first;
        sectionsSec = it->second.first;
        sectionsSecStartAddr = it->second.second;

        // Ako je isti fajl i isto ime sekcije - to je ta sekcija za koju mi treba pocetna adresa
        // Samo jedna ovakva kombinacija postoji u sections, pa kad je nadjem i obradim, mogu da break-ujem
        if ((sectionsFileName.compare(parserFileName) == 0) && (symbSection.compare(sectionsSec->getName()) == 0))
        {
          symbValue += sectionsSecStartAddr;
          symTable[j]->setValue(symbValue);
          
          break;
        }
      }
      // } else if (symTable[j]->getType().compare("NOTYPE") == 0)
      // {
      //   for (auto it = sections.begin(); it != sections.end(); ++it)
      //   {
      //     sectionsFileName = it->first;
      //     sectionsSec = it->second.first;
      //     sectionsSecStartAddr = it->second.second;

      //     // cout << "file: " << sectionsFileName << ", secName: " << sectionsSec->getName() << ", startaddr: " << sectionsSecStartAddr << endl;

      //     if (it == sections.begin())
      //     {
      //       continue;
      //     } else
      //     {
      //       auto prethodni = prev(it);

      //       SectionLD* pretSec = prethodni->second.first;
      //       int velicinaPrethodneSekcije = pretSec->getSectionSize();

      //       cout << "prethodna sekcija je " << prethodni->second.first->getName() << ", velicine " << velicinaPrethodneSekcije << endl;

      //       // Ako je isti fajl i isto ime sekcije - to je ta sekcija
      //       // Samo jedna ovakva kombinacija postoji u sections, pa kad je nadjem i obradim, mogu da break-ujem
      //       if ((sectionsFileName.compare(parserFileName) == 0) && (symbSection.compare(sectionsSec->getName()) == 0))
      //       {
      //         symbValue += prethodni->second.first->getSectionSize();
      //         symTable[j]->setValue(symbValue);
              
      //         break;
      //       }
      //     }      
      //   }
      }
      parsers[i]->setSymbolTable(symTable);
    }    

    // print za proveru
    // cout << "symbTable posle update, file: " << parserFileName << endl;
    // for (int k = 0; k < symTable.size();k++)
    // {
    //   cout << std::internal << std::setw(5) << symTable.at(k)->getID();
    //   cout << std::internal << std::setw(20) << symTable.at(k)->getName();
    //   cout << std::internal << std::setw(20) << symTable.at(k)->getSection();
    //   cout << std::internal << std::setw(10) << symTable.at(k)->getBind();
    //   cout << std::internal << std::setw(10) << hex << symTable.at(k)->getValue();
    //   cout << std::internal << std::setw(10) << symTable.at(k)->getType();
    //   // cout<<"Value:";
    //   // outfile<<" "<<decimalToHex(std::to_string(symbolTable.at(i).getSymbolValue()));
    //   cout <<std::endl;
    // }
  // }
}

void Linker::updateSectionData()
{
  // Prolazim kroz sections
  // Za svaki fajl i svaku sekciju dohvatam Data te sekcije i prepisujem u DataWithAddress sa adresama

  string fileName;
  SectionLD* sec;
  int secStartAddr;
  
  vector<unsigned char> secData;
  vector<pair<int, unsigned char>> secDataWithAddr;
  vector<LiteralLD*> secLitTable;

  for (auto it = sections.begin(); it != sections.end(); ++it)
  {
    fileName = it->first;
    sec = it->second.first;
    secStartAddr = it->second.second;

    // cout << "file:" << fileName << ", sec: " << sec->getName() << ", addr: " << secStartAddr<< endl;

    secData = sec->getSectionData();
    secDataWithAddr = sec->getDataWithAddress();
    secLitTable = sec->getLiteralTable();

    // for (int i = 0; i < secData.size(); i++)
    // {
    //   cout << hex << (int)secData[i] << endl;
    // }

    int addr = secStartAddr;

    for (int i = 0; i < secData.size(); i++)
    {
      secDataWithAddr.emplace_back(std::make_pair(addr, secData[i]));
      addr++;
    }
    // cout << "addr = " << hex << addr << endl;

    // for (int i = 0; i < secLitTable.size(); i++)
    // {
    //   int val = secLitTable[i]->getLiteralValueInt();

    //   // cout << "addr iz bazena: " << hex << secLitTable[i]->getLiteralLocation();
    //   // cout << ", addr: " << addr << ", val: " << hex << val << endl;

    //   unsigned char byte1 = (unsigned char)(val & 0xFF);
    //   unsigned char byte2 = (unsigned char)((val >> 8) & 0xFF);
    //   unsigned char byte3 = (unsigned char)((val >> 16) & 0xFF);
    //   unsigned char byte4 = (unsigned char)((val >> 24) & 0xFF);

    //   secDataWithAddr.emplace_back(std::make_pair(addr, byte1));
    //   addr++;
    //   secDataWithAddr.emplace_back(std::make_pair(addr, byte2));
    //   addr++;
    //   secDataWithAddr.emplace_back(std::make_pair(addr, byte3));
    //   addr++;
    //   secDataWithAddr.emplace_back(std::make_pair(addr, byte4));
    //   addr++;
    // }

    sec->setDataWithAddress(secDataWithAddr);

    // auto dataaddrproba = sec->getDataWithAddress();

    // // cout << "UPDATE DATA PRE RELOC FIX" << endl;

    // cout << "file Name: " << fileName << ", sec Name: " << sec->getName() << ", data size: " << dataaddrproba.size() << endl;

    // for (auto k = dataaddrproba.begin(); k != dataaddrproba.end(); ++k)
    // {
    //   cout << hex << k->first << " : " << hex << (int)k->second << endl;
    // }

    
    // auto reldataproba = sec->getRelocationTable();
    // if (!reldataproba.empty())
    // {
    //   cout << "RELDATA" << endl;
    //   for (int i = 0; i < reldataproba.size(); i++)
    //   {
    //     cout << hex << reldataproba[i]->getOffset() << endl;
    //   }
    // }
    // if (!secLitTable.empty())
    // {
    //   cout << "LITTABLE" << endl;
    //   for (int i = 0; i < secLitTable.size(); i++)
    //   {
    //     cout << hex << secLitTable[i]->getLiteralLocation() << ":" << hex << secLitTable[i]->getLiteralValueInt() << endl;
    //   }
    // }
    
  }
}

void Linker::fixRealocData()
{
  // Prolazim kroz sections
  // Za svaki fajl i svaku sekciju dohvatam reloc tabelu te sekcije (ako je ima) i njen dataWithAddress

  string fileName;
  SectionLD* sec;
  int secStartAddr;
  
  vector<pair<int, unsigned char>> secDataWithAddr;
  vector<RelocSymbolLD*> secRelTable;

  for (auto it = sections.begin(); it != sections.end(); ++it)
  {
    fileName = it->first;
    sec = it->second.first;
    secStartAddr = it->second.second;

    // cout << "file Name: " << fileName << ", sec Name: " << sec->getName() << ", startAddr: " << hex << secStartAddr << endl;

    secDataWithAddr = sec->getDataWithAddress();
    secRelTable = sec->getRelocationTable();

    unsigned char byte1;
    unsigned char byte2;
    unsigned char byte3;
    unsigned char byte4;
    SymbolLD* sym;
    int offset;

    // Ako secRelTable postoji, iteriram kroz nju i na offset mestima u secData prepravljam sadrzaj
    if (!secRelTable.empty())
    {
      // cout << "file Name: " << fileName << ", sec Name: " << sec->getName() << ", startAddr: " << hex << secStartAddr << endl;
      for (int i = 0; i < secRelTable.size(); i++)
      {
        sym = secRelTable[i]->getSymbol();
        offset = secRelTable[i]->getOffset();
        int symValue = findSymbolByNameInLinker(sym->getName())->getValue();

        byte1 = (unsigned char)(symValue & 0xFF);
        byte2 = (unsigned char)((symValue >> 8) & 0xFF);
        byte3 = (unsigned char)((symValue >> 16) & 0xFF);
        byte4 = (unsigned char)((symValue >> 24) & 0xFF);

        
        // Idem kroz dataWithAddress i na adresi offset do offset+4 upisujem 4B vrednosti simbola u little endian
        for (auto k = secDataWithAddr.begin(); k != secDataWithAddr.end(); ++k)
        {
          if (k->first == offset)
          {
            // cout << "sym name: " << sym->getName() << ", sym value: " << hex << symValue << ", offset: " << secRelTable[i]->getOffset() << endl;
            // cout << "sym value: " << hex << symValue << ", byte1: " << hex << (int)byte1 << ", byte2: " << hex << (int)byte2 << ", byte3: " << hex << (int)byte3 << ", byte4: " << hex << (int)byte4 << endl;

            // cout << "data pre ispravke: " << hex << (int)k->second << " " << hex << (int)(k+1)->second << " " << hex << (int)(k+2)->second << " " << hex << (int)(k+3)->second << ", adresa: " << hex << offset << endl;
          
            // unsigned char byte3data = (k+2)->second;
            // unsigned char byte4data = (k+3)->second;

            k->second = byte1;
            (k+1)->second = byte2;
            (k+2)->second = byte3;
            (k+3)->second = byte4;

            // cout << "data posle ispravke: " << hex << (int)k->second << " " << hex << (int)(k+1)->second << " " << hex << (int)(k+2)->second << " " << hex << (int)(k+3)->second << ", adresa: " << hex << offset << endl;
          }
        }
      }
    }

    sec->setDataWithAddress(secDataWithAddr);

    // auto dataaddrproba = sec->getDataWithAddress();

    // cout << "file Name: " << fileName << ", sec Name: " << sec->getName() << endl;

    // for (auto k = dataaddrproba.begin(); k != dataaddrproba.end(); ++k)
    // {
    //   cout << hex << k->first << " : " << hex << (int)k->second << endl;
    // }
  }
}

SymbolLD* Linker::findSymbolByNameInLinker(string name)
{
  for(auto i = symTable.begin(); i != symTable.end(); ++i)
  {
    if((*i)->getName() == name)
    {
      return (*i)->getSymbol();
    }
  }
  return nullptr;
}

void Linker::fillSymbolTable()
{
  for (int i = 0; i < parsers.size(); i++)
  {
    vector<SymbolLD*> symTableTmp = parsers[i]->getSymbolTable();

    for(int j = 0; j < symTableTmp.size(); j++)
    {
      string symbolName = symTableTmp[j]->getName();
      string symbolSec = symTableTmp[j]->getSection();

      SymbolLD* symTmp = findSymbolByNameInLinker(symbolName);
      
      if (!symTmp && symbolSec.compare("UND") != 0)
      {
        symTable.push_back(symTableTmp[j]);
      } else if (symTmp && (symTmp->getType()!= symTableTmp[j]->getType()) && (symTableTmp[j]->getType().compare("LOC") == 0) && (symTmp->getSection().compare(symTableTmp[j]->getSection()) == 0))
      {
        // provera za multiple definition
        cout << "ERROR: Visestruka definicija simbola " << symTableTmp[j]->getName() << ", fajl: " << parsers[i]->getFileName() << endl;
        exit(-1);
      }
    }
  }
}

void Linker::printSymbolTable()
{
  cout << endl << "SYMBOL TABLE" << endl;
  for(int i=0; i<symTable.size(); i++) {
    // cout << std::internal << std::setw(5) << symTable.at(i)->getID();
    cout << std::internal << std::setw(20) << symTable.at(i)->getName();
    cout << std::internal << std::setw(20) << symTable.at(i)->getSection();
    cout << std::internal << std::setw(10) << symTable.at(i)->getBind();
    cout << std::internal << std::setw(10) << hex << symTable.at(i)->getValue();
    cout << std::internal << std::setw(10) << symTable.at(i)->getType();
    // cout<<"Value:";
    // outfile<<" "<<decimalToHex(std::to_string(symbolTable.at(i).getSymbolValue()));
    cout <<std::endl;
  }
}

void Linker::printData(ostream &os)
{
  // Prolazim kroz sections, za svaku sekciju uzimam njen dataWithAddr i smestam u fajl
  for (auto it = sections.begin(); it != sections.end(); ++it)
  {
    it->second.first->printDataWithAddress(os);
  }
}
