#include "../inc/assembler.h"
#include <string.h>

int Symbol::rednibrojSimbola = 0;
bool Assembler::prviProlaz = false;
bool Assembler::drugiProlaz = false;
bool Assembler::end = false;

Assembler::Assembler()
{
  Section* undSec = new Section("UND");
  // Symbol* undSym =  new Symbol("UND", 0, "UND", true, false, false); // stavljam da UND jeste sekcija
  secTable.addSectionToSectionTable(undSec);
  // symTable.addSymboltoSymbolTable(undSym);
  currentSection = undSec;
}

void Assembler::obradaDirektiveGlobal(string s)
{  
  vector<string> args = getArgsFromString(s);

  if (args.size() == 0)
  {
    cout << "ERROR: Sintaksna greska, nedovoljan broj argumenata uz .global direktivu!" << endl;
    exit(-1);
  }

  if(prviProlaz)
  {
    for(int i = 0; i < args.size(); i++)
    {
      Symbol* s = symTable.findSymbolByName(args.at(i));
      if(!s)
      {
        Symbol* newSymbol = new Symbol(args.at(i), currentSection->getLocCounter(), currentSection->getName(), false, true, false);
        symTable.addSymboltoSymbolTable(newSymbol);
      } else {
        // Ako je vec bio u tabeli simbola, samo njegovo bind polje postavim na global
        s->setSymbolBinding(1);
        s->setSymbolGlobal(true);
      }
    }
  }
  // U drugom prolazu nije potrebno uraditi nista novo
}

void Assembler::obradaDirektiveExtern(string s)
{
  vector<string> args = getArgsFromString(s);

  if (args.size() == 0)
  {
    cout << "ERROR: Sintaksna greska, nedovoljan broj argumenata uz .global direktivu!" << endl;
    exit(-1);
  }

  if(prviProlaz)
  {
    for(int i = 0; i < args.size(); i++)
    {
      Symbol* s = symTable.findSymbolByName(args.at(i));
      if(!s)
      {
        Symbol* newSymbol =  new Symbol(args.at(i), 0, "UND", false, true, true);
        symTable.addSymboltoSymbolTable(newSymbol);
      }
      else
      {
        cout << "ERROR: Sintaksna greska, simbol uz direktivu .extern vec postoji!" << endl;
        exit(-1);
      }
    }
  }
  // U drugom prolazu nije potrebno uraditi nista novo
}

void Assembler::obradaDirektiveSection(string s)
{
  vector<string> args = getArgsFromString(s);

  if(args.size() != 1) 
  {
    cout << "ERROR: Sintaksna greska, neispravan broj argumenata uz .section direktivu!" << endl;
    exit(-1);
  }

  if(prviProlaz)
  {
    // Kada se pojavi .section name, to znaci da se prethodna sekcija tu zavrsava -> treba da azuriram njenu velicinu
    // Kada u prvom prolazu napustam sekciju, mogu da azuriram njen bazen literala (lokacije)

    int velicina_bazena = (currentSection->getLiteralTable()->size());  // TODO - DA LI TREBA *4?????????
    velicina_bazena *= 4;

    // if (velicina_bazena > 0)
    // { 
    //   currentSection->incLocationCounterFor(4);   // zbog jmp bazen instrukcije
    // }
    if(currentSection->getName().compare("UND") != 0) currentSection->incLocationCounterFor(4);   // zbog jmp bazen instrukcije
    currentSection->setSectionSize(currentSection->getLocCounter() + velicina_bazena);
    currentSection->updateLiteralTable();

    Section* sec = secTable.findSectionByName(s);
    if(!sec)  // nema je u tabeli sekcija - dodajem je u secTable i symTable
    {
      if (symTable.findSymbolByName(s) != nullptr)
      {
        cout << "ERROR: Simbol uz .section direktivu vec postoji u tabeli simbola!" << endl;
        exit(-1);
      }

      Section* newSec = new Section(s);
      secTable.addSectionToSectionTable(newSec);

      Symbol* newSym = new Symbol(s, 0, s, true, false, false);
      symTable.addSymboltoSymbolTable(newSym);
      currentSection = newSec;
    } else
    {
      // Ako pronadjem sekciju u tabeli sekcija
      // currentSection setujem na tu sekciju
      // To je slucaj da se nastavlja neka prethodna sekcija koja je ranije prekinuta
      currentSection = sec;
    }
    
  }
  else if (drugiProlaz)
  {
    // Kao poslednju instrukciju u sekciji dodajem jmp velicina_bazena_literala
    // Velicina bazena: bazen.size()*4 -> broj ulaza * velicina ulaza (4B)
    int velicina_bazena = (currentSection->getLiteralTable()->size());  // TODO - DA LI TREBA *4?????????
    velicina_bazena *= 4;

    // if (velicina_bazena > 0)
    // {
    if(currentSection->getName().compare("UND") != 0)
    {
      vector<unsigned char> data = currentSection->getSectionData();

      // A = 15, B = 0, C = 0, D = velicina_bazena
      unsigned char byte1 = 0b00110000;
      unsigned char byte2 = 0b11110000;

      unsigned int tmp = (velicina_bazena >> 8) & 0b1111;
      unsigned char byte3 = 0x00 | ((unsigned char)tmp);

      unsigned char byte4 = (unsigned char)(velicina_bazena & 0xFF);

      data.push_back(byte1);
      data.push_back(byte2);
      data.push_back(byte3);
      data.push_back(byte4);

      currentSection->setSectionData(data);
      currentSection->incLocationCounterFor(4);   // zbog jmp bazen instrukcije
    }
    // }

    // currentSection->setSectionSize(currentSection->getLocCounter() + velicina_bazena + 4);
    currentSection->setSectionSize(currentSection->getLocCounter() + velicina_bazena);
    // currentSection->updateLiteralTable();
    currentSection->updateRelocationTable();

    // Nakon toga postavljam sta je nova sekcija
    currentSection = secTable.findSectionByName(s);
    currentSection->resetLocationCounter();
  }
}

void Assembler::obradaDirektiveWord(string s) // TODO: DA LI U PRVOM PROLAZU UBACUJEM LITERAL U BAZEN LITERALA?
{
  // 1. prolaz: sacuva nove simbole u tabelu simbola i uveca locationCounter za 4
  // 2. prolaz: upisuje kao masinsku instrukciju njihove vrednosti
  vector<string> args = getArgsFromString(s);

  if(args.size() == 0) 
  {
    cout << "ERROR: Sintaksna greska, nedovoljan broj argumenata uz .word direktivu!" << endl;
    exit(-1);
  }

  if(prviProlaz)
  {
    for(int i = 0; i < args.size(); i++)
    {
      // prvo treba da proverim da li je args[i] simbol ili literal
      if (isSymbol(args[i]))
      {
        // simbol
        // proveravam da li simbol s postoji u tabeli simbola
        Symbol* s = symTable.findSymbolByName(args[i]);
        if(!s)
        {
          // ako ne postoji dodajem ga
          // Vrednost novog simbola postavljam na 0,
          // jer svakako u drugom prolazu ide u relokacionu tabelu,
          // pa je posao linkera da zakrpi vrednost
          Symbol* newSymbol =  new Symbol(args.at(i), 0,  currentSection->getName(), false, false, false);
          symTable.addSymboltoSymbolTable(newSymbol);
          currentSection->incLocationCounterFor(4);
        } // else
        // {
        //   cout << "Vec je definisan simbol sa imenom " << s << endl;
        // }
      } else
      {
        // literal
        currentSection->incLocationCounterFor(4);
      }
    }
  } else if(drugiProlaz)  // TODO - CHECK!!!
  {
    for(int i = 0; i < args.size(); i++)
    {
      if(isLiteral(args[i]))
      {
        // HEX ili DEC broj
        vector<unsigned char> data = currentSection->getSectionData();
        int num = 0;
        if(isDecNumber(s))
          num = stoi(s);
        else if(isHexNumber(s))
          num = stoi(s, nullptr, 16);

        data.push_back(num & 0xFF);
        data.push_back((num >> 8) & 0xFF);
        data.push_back((num >> 16) & 0xFF);
        data.push_back((num >> 24) & 0xFF);
        currentSection->setSectionData(data);

        currentSection->incLocationCounterFor(4);
      } else if(isSymbol(args[i]))
      {
        // provera da li ga ima u tabeli simbola
        Symbol* sym = symTable.findSymbolByName(args[i]);
        if(!sym)
        {
          cout << "ERROR: Sintaksna greska. Simbol naveden uz .word direktivu ne postoji u tabeli simbola!" << endl;
          exit(-1);
        }
        
        // Ako postoji, upisujem nule u data za tu sekciju, a taj simbol smestam u relokacionu tabelu za tu sekciju
        vector<unsigned char> data = currentSection->getSectionData();

        data.push_back(0b00000000);
        data.push_back(0b00000000);
        data.push_back(0b00000000);
        data.push_back(0b00000000);

        currentSection->setSectionData(data);

        // ubacujem simbol u relokacionu tabelu
        RelocSymbol* relSym = new RelocSymbol(sym, currentSection->getLocCounter());
        vector<RelocSymbol*> relData = currentSection->getRelocationTable();
        relData.push_back(relSym);
        currentSection->setRelocationData(relData);

        // Prvo dodam relokacioni zapis, pa onda uvecam locCounter
        currentSection->incLocationCounterFor(4);
      }
    }
  }
}

void Assembler::obradaDirektiveSkip(string s)
{
  if(prviProlaz)
  {
    if(isDecNumber(s))
    {
      currentSection->incLocationCounterFor(stoi(s));
    }
    else if(isHexNumber(s))
    {
      currentSection->incLocationCounterFor(stoi(s, nullptr, 16));
    } else
    {
      // nije literal, prijavljuj gresku
      cout << "ERROR: Sintaksna greska, argument uz .skip direktivu mora biti literal (broj)!" << endl;
      exit(-1);
    }
  } else if(drugiProlaz)  // TODO - CHECK
  {
    // drugi prolaz
    int num = 0;
    if(isDecNumber(s))
      num = stoi(s);
    else if(isHexNumber(s))
      num = stoi(s, nullptr, 16);
    else
    {
      // nije literal, prijavljuj gresku
      cout << "ERROR: Sintaksna greska, argument uz .skip direktivu mora biti literal (broj)!" << endl;
      exit(-1);
    }

    vector<unsigned char> data = currentSection->getSectionData();
    for(int i = 0; i < num; i++)
      data.push_back(0);  // 0b00000000
    currentSection->incLocationCounterFor(num);
    currentSection->setSectionData(data);
  } 
}

void Assembler::obradaDirektiveEnd(string s)
{
  // da li ovde uopste ima razlike kad su prvi i drugi prolaz u pitanju?
  // cout << "prviProlaz: " << prviProlaz << "\n";
  // cout << "drugiProlaz: " << drugiProlaz << "\n";
  // if(s != "")                  VIDI STA CES SA OVIM, iza .end NE SME da se nadje nikakav string!!!
  // {
  //   cout << "ERROR!!!\n";
  // }
  int velicina_bazena = (currentSection->getLiteralTable()->size());  // TODO - DA LI TREBA *4?????????
  velicina_bazena *= 4;

  
  // probam sa updateRelocTable
  // currentSection->updateRelocationTable();

  Assembler::end = true;
  if(prviProlaz)
  {
    // if (velicina_bazena > 0)
    // {
    //   currentSection->incLocationCounterFor(4);   // zbog jmp bazen instrukcije
    // }
    if(currentSection->getName().compare("UND") != 0) currentSection->incLocationCounterFor(4);   // zbog jmp bazen instrukcije

    currentSection->setSectionSize(currentSection->getLocCounter() + velicina_bazena);
    currentSection->updateLiteralTable();
    
    // currentSection->setSectionSize(currentSection->getLocCounter());
    currentSection = secTable.findSectionByName("UND");
    prviProlaz = false;
  } else if (drugiProlaz)
  {
    // if (velicina_bazena > 0)
    // {

    if (currentSection->getName().compare("UND") != 0)
    {
      vector<unsigned char> data = currentSection->getSectionData();

      // A = 15, B = 0, C = 0, D = velicina_bazena
      unsigned char byte1 = 0b00110000;
      unsigned char byte2 = 0b11110000;

      unsigned int tmp = (velicina_bazena >> 8) & 0b1111;
      unsigned char byte3 = 0x00 | ((unsigned char)tmp);

      unsigned char byte4 = (unsigned char)(velicina_bazena & 0xFF);

      data.push_back(byte1);
      data.push_back(byte2);
      data.push_back(byte3);
      data.push_back(byte4);

      currentSection->setSectionData(data);
      currentSection->incLocationCounterFor(4);   // zbog jmp bazen instrukcije
    }
    
    // }

    currentSection->setSectionSize(currentSection->getLocCounter() + velicina_bazena);
    // currentSection->updateLiteralTable();
    currentSection->updateRelocationTable();
    drugiProlaz = false;
  }
}

void Assembler::obradaDirektive(string s)
{
  if(isDirectiveGlobal(s)) 
  {
    s.erase(0,8); // uklanjam .global i space posle .global
    obradaDirektiveGlobal(s);
  } else if(isDirectiveExtern(s))
  {
    s.erase(0,8); // uklanjam .extern i space posle .extern
    obradaDirektiveExtern(s);
  } else if(isDirectiveSection(s))
  {
    s.erase(0,9); // uklanjam .section i space posle .section
    obradaDirektiveSection(s);
  } else if(isDirectiveWord(s))
  {
    s.erase(0,6); // uklanjam .word i space posle .word
    obradaDirektiveWord(s);
  } else if(isDirectiveSkip(s))
  {
    s.erase(0,6); // uklanjam .skip i space posle .skip
    obradaDirektiveSkip(s);
  } else if(isDirectiveEnd(s)) 
  {
    // ne skracujem string - NE OCEKUJEM NIKAKAV STRING
    obradaDirektiveEnd(s);
  }
}

void Assembler::obradaInstrukcije(string s)
{
  
  // 1. prolaz: uvecava locCounter tekuce sekcije za 4 (jer je to velicina instrukcije)
  // argumente (koji nisu registri) smestam u symTable

  // Instrukcija mora da se nalazi unutar neke sekcije koja nije "ABS" i "UND"
  if(currentSection->getName().compare("UND") == 0 || currentSection->getName().compare("ABS") == 0)
  {
    cout << "ERROR: Instrukcija se mora naci unutar neke sekcije!" << endl;
    exit(-1);
  }

  // No Operand Instructions - halt, int, iret, ret
  if (isNoOpInstruction(s))
  {
    if (prviProlaz)
    {
      if (isInstructionIret(s))
        currentSection->incLocationCounterFor(8);
      else
        currentSection->incLocationCounterFor(4);
    } else if (drugiProlaz)
    {
      if (isInstructionHalt(s))
      {
        vector<unsigned char> data = currentSection->getSectionData();

        unsigned char byte1 = 0b00000000;
        unsigned char byte2 = 0b00000000;
        unsigned char byte3 = 0b00000000;
        unsigned char byte4 = 0b00000000;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);
        currentSection->incLocationCounterFor(4);
      } else if (isInstructionInt(s))
      {
        vector<unsigned char> data = currentSection->getSectionData();

        unsigned char byte1 = 0b00010000;
        unsigned char byte2 = 0b00000000;
        unsigned char byte3 = 0b00000000;
        unsigned char byte4 = 0b00000000;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);
        currentSection->incLocationCounterFor(4);
      } else if (isInstructionIret(s))    // TODO - CHECK!!!
      {
        // pop status; - Instrukcija ucitavanja podataka - MMMM = 0b0111;
        // csr[A] <= mem32[gpr[B]]; gpr[B] <= gpr[B] + D;
        // A =  0 (status); B = 14 (SP); D = +4;

        vector<unsigned char> data = currentSection->getSectionData();

        unsigned char byte1ST = 0b10010011;
        unsigned char byte2ST = 0b00001110;
        unsigned char byte3ST = 0b00000000;
        unsigned char byte4ST = 0b00000100;

        data.push_back(byte1ST);
        data.push_back(byte2ST);
        data.push_back(byte3ST);
        data.push_back(byte4ST);

        // pop pc; - Instrukcija ucitavanja podataka - MMMM = 0b0011;
        // gpr[A] <= mem32[gpr[B]]; gpr[B] <= gpr[B] + D;
        // A = 15 (PC); B = 14 (SP); D = +4;

        unsigned char byte1PC = 0b10010011;   // instrCode = 0b1001; instrMode = 0b0011;
        unsigned char byte2PC = 0b11111110;   // A = 15 (pc); B = sp (14)
        unsigned char byte3PC = 0b00000000;   // C = 0; najvisa 4b D = 0;
        unsigned char byte4PC = 0b00000100;   // nizih 8b D = +4;

        data.push_back(byte1PC);
        data.push_back(byte2PC);
        data.push_back(byte3PC);
        data.push_back(byte4PC);

        currentSection->setSectionData(data);
        currentSection->incLocationCounterFor(8);
      } else if (isInstructionRet(s))   // TODO - CHECK!!!
      {
        // ret -> pop pc

        // Instrukcija ucitavanja podataka
        // MMMM = 0b0011
        // gpr[A] <= mem32[gpr[B]]; gpr[B] <= gpr[B] + D;
        // gpr[A] == PC (R15); gpr[B] == SP (R14);

        vector<unsigned char> data = currentSection->getSectionData();

        unsigned char byte1 = 0b10010011;   // instrCode = 0b1001; instrMode = 0b0011;
        unsigned char byte2 = 0b11111110;   // A = 15 (pc); B = sp (14)
        unsigned char byte3 = 0b00000000;   // C = 0; najvisa 4b D = 0;
        unsigned char byte4 = 0b00000100;   // nizih 8b D = +4;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);
        currentSection->incLocationCounterFor(4);
      }
    }
  }

  // One Operand Instructions - call, jump, push, pop, not
  else if (isOneOpInstruction(s))
  {
    // Operand moze biti samo %gpr, osim u slucaju call i jmp instrukcija
    
    if (isInstructionJmp(s))
    {
      s.erase(0,4); // brisem jmp i space iza jmp
      vector<string> args = getArgsFromString(s);
      if(args.size() != 1)
      {
        cout << "ERROR: Sintaksna greska, ocekivan samo jedan operand!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);
        if (isLiteral(args[0]))
        {
          obradaLiterala(args[0]);
        } else if (isSymbol(args[0]))
        {
          // Ako jeste, proveravam da li se taj simbol nalazi u tabeli simbola
          // Ako se ne nalazi, dodajem ga sa vrednoscu 0 i sekcijom UND
          // Ako se vec nalazi u tabeli simbola, idem dalje, sve OK
          if (!symTable.findSymbolByName(args[0]))  // ako je povratna vrednost nullptr
          {
            Symbol* newSym = new Symbol(args[0], 0, "UND", false, false, false); // stavljam da je lokalan
            symTable.addSymboltoSymbolTable(newSym);
          }
        } else
        {
          // Nije ni labela, ni literal, znaci registar je - ERROR
          cout << "ERROR: Sintaksna greska! Argument uz instrukciju JMP mora biti labela ili literal!" << endl;
          exit(-1);
        }
      } else if (drugiProlaz)   // TODO - CHECK!!!!!!!!!!!!!!!!!!!!!!1
      {
        // jmp operand
        // pc <= operand
        // opreand: <literal> (vrednost literal) ili <simbol> (vrednost simbol)

        vector<unsigned char> data = currentSection->getSectionData();

        unsigned char byte1 = 0x00;
        unsigned char byte2 = 0x00;
        unsigned char byte3 = 0x00;
        unsigned char byte4 = 0x00;

        // A = 15
        // B i C su 0

        if (isLiteral(args[0]))
        {
          int num = 0;
          
          if (isHexNumber(args[0]))
            num = (int)stol(args[0], nullptr, 16);
          else
            num = stoi(args[0]);

          if (!stajeU12Bita(args[0])) // u D bite smestam pomeraj do literala iz bazena literala
          {
            byte1 = 0b00111000;   // instrCode = 0b0011; instrMode = 0b1000;
            byte2 = 0b11110000;   // A = 15, B = 0

            int displacement = 0;

            if (currentSection->entryExistsInLiteralTable(num))
            {
              displacement = currentSection->getLiteralLocation(num) - currentSection->getLocCounter();
            } else
            {
              cout << "ERROR: Literal " << args[0] << " ne postoji u tabeli literala!" << endl;
              exit(-1);
            }

            unsigned int tmp = (displacement >> 8) & 0b1111;
            byte3 = 0x00 | ((unsigned char)tmp);

            byte4 = (unsigned char)(displacement & 0xFF);
          } else // literal moze da se smesti u D bite
          {
            byte1 = 0b00110000;   // instrCode = 0b0011; instrMode = 0b0000;
            byte2 = 0b11110000;   // A = 15, B = 0
            
            unsigned int tmp = (num >> 8) & 0b1111;
            byte3 = 0x00 | ((unsigned char)tmp);

            byte4 = (unsigned char)(num & 0xFF);
          }

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        } else if (isSymbol(args[0]))   // TODO - CHECK!!!!!!!!!!!!!!!!
        {
          Symbol* sym = symTable.findSymbolByName(args[0]);
          
          // if (currentSection->getName() == sym->getSymbolSection())
          // {
          //   // Simbol se nalazi u istoj sekciji -> LAKSI SLUCAJ
          //   int val = sym->getSymbolValue();

          //   byte1 = 0b00110000;
          //   byte2 = 0b11110000;   // A = 15, B = 0
            
          //   unsigned int tmp = (val >> 8) & 0b1111;
          //   byte3 = 0x00 | ((unsigned char)tmp);

          //   byte4 = (unsigned char)(val & 0xFF);
          // } else
          // {
          // Simbol se ne nalazi u istoj sekciji -> TEZI SLUCAJ
          vector<pair<int,int>> addrIndex = currentSection->addNewEntryAtTheEndOfLiteralTable(sym); // Ova addr mi treba za relok zapis

          int addr,index;
          for (auto it = addrIndex.begin(); it != addrIndex.end(); ++it)
          {
            addr = it->first;
            index = it->second;
          }

          byte1 = 0b00111000;   // instrCode = 0b0011; instrMode = 0b1000;
          byte2 = 0b11110000;   // A = 15, B = 0

          int displacement = addr - currentSection->getLocCounter();

          unsigned int tmp = (displacement >> 8) & 0b1111;
          byte3 = 0x00 | ((unsigned char)tmp);

          byte4 = (unsigned char)(displacement & 0xFF);

          RelocSymbol* relSym = new RelocSymbol(sym, addr, index);
          vector<RelocSymbol*> relData = currentSection->getRelocationTable();
          relData.push_back(relSym);
          currentSection->setRelocationData(relData);
          // }

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        }
      }
    } else if (isInstructionCall(s))
    {
      s.erase(0,5); // brisem call i space iza call
      vector<string> args = getArgsFromString(s);
      if(args.size() != 1)
      {
        cout << "ERROR: Sintaksna greska, ocekivan samo jedan operand!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);

        if (isLiteral(args[0]))
        {
          obradaLiterala(args[0]);
        } else if (isSymbol(args[0]))
        {
          // Ako jeste, proveravam da li se taj simbol nalazi u tabeli simbola
          // Ako se ne nalazi, dodajem ga sa vrednoscu 0 i sekcijom UND
          // Ako se vec nalazi u tabeli simbola, idem dalje, sve OK
          if (!symTable.findSymbolByName(args[0]))  // ako je povratna vrednost nullptr
          {
            Symbol* newSym = new Symbol(args[0], 0, "UND", false, false, false); // stavljam da je lokalan
            symTable.addSymboltoSymbolTable(newSym);
          }
        } else
        {
          // Nije ni labela, ni literal, znaci registar je - ERROR
          cout << "ERROR: Sintaksna greska! Argument uz instrukciju CALL mora biti labela ili literal!" << endl;
          exit(-1);
        }
      } else if (drugiProlaz)
      {
        // call operand
        // push pc; pc <= operand
        // opreand: <literal> (vrednost literal) ili <simbol> (vrednost simbol)

        vector<unsigned char> data = currentSection->getSectionData();

        unsigned char byte1 = 0x00;
        unsigned char byte2 = 0x00;
        unsigned char byte3 = 0x00;
        unsigned char byte4 = 0x00;

        // A i B su 0

        if (isLiteral(args[0]))
        {
          int num = 0;
          
          if (isHexNumber(args[0]))
            num = (int)stol(args[0], nullptr, 16);
          else
            num = stoi(args[0]);

          if (!stajeU12Bita(args[0])) // u D bite smestam pomeraj do literala iz bazena literala
          {
            byte1 = 0b00100001;   // instrCode = 0b0010; instrMode = 0b0001;
            // byte2 se ne menja, posto su A i B 0 - ipak NE
            byte2 = 0b11110000; // A = 15 (PC)

            int displacement = 0;

            if (currentSection->entryExistsInLiteralTable(num))
            {
              displacement = currentSection->getLiteralLocation(num) - currentSection->getLocCounter() - 4;
            } else
            {
              cout << "ERROR: Literal " << args[0] << " ne postoji u tabeli literala!" << endl;
              exit(-1);
            }

            unsigned int tmp = (displacement >> 8) & 0b1111;
            byte3 = 0x00 | ((unsigned char)tmp);

            byte4 = (unsigned char)(displacement & 0xFF);
          } else // literal moze da se smesti u D bite
          {
            byte1 = 0b00100000;   // instrCode = 0b0010; instrMode = 0b0000;
            // byte2 se ne menja, posto su A i B 0 - ipak NE
            byte2 = 0b11110000; // A = 15 (PC)
            
            unsigned int tmp = (num >> 8) & 0b1111;
            byte3 = 0x00 | ((unsigned char)tmp);

            byte4 = (unsigned char)(num & 0xFF);
          }

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        } else if (isSymbol(args[0]))   // TODO - CHECK!!!!!!!!!!!!!!!!
        {
          Symbol* sym = symTable.findSymbolByName(args[0]);
          
          // if (currentSection->getName() == sym->getSymbolSection())
          // {
          //   // Simbol se nalazi u istoj sekciji -> LAKSI SLUCAJ
          //   int val = sym->getSymbolValue();

          //   byte1 = 0b00100000;   // instrCode = 0b0010; instrMode = 0b0000;
          //   // byte2 se ne menja, posto su A i B 0 - ipak NE
          //   byte2 = 0b11110000; // A = 15 (PC)
            
          //   unsigned int tmp = (val >> 8) & 0b1111;
          //   byte3 = 0x00 | ((unsigned char)tmp);

          //   byte4 = (unsigned char)(val & 0xFF);
          // } else
          // {
          // Simbol se ne nalazi u istoj sekciji -> TEZI SLUCAJ
          vector<pair<int,int>> addrIndex = currentSection->addNewEntryAtTheEndOfLiteralTable(sym); // Ova addr mi treba za relok zapis

          int addr, index;
          for (auto it = addrIndex.begin(); it != addrIndex.end(); ++it)
          {
            addr = it->first;
            index = it->second;
          }

          byte1 = 0b00100001;   // instrCode = 0b0010; instrMode = 0b0001;
          // byte2 se ne menja, posto su A i B 0 - ipak NE
          byte2 = 0b11110000; // A = 15 (PC)

          int displacement = addr - currentSection->getLocCounter() - 4;

          unsigned int tmp = (displacement >> 8) & 0b1111;
          byte3 = 0x00 | ((unsigned char)tmp);

          byte4 = (unsigned char)(displacement & 0xFF);

          RelocSymbol* relSym = new RelocSymbol(sym, addr, index);
          vector<RelocSymbol*> relData = currentSection->getRelocationTable();
          relData.push_back(relSym);
          currentSection->setRelocationData(relData);
          // }

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        }
      }
    } else if (isInstructionPush(s))
    {
      s.erase(0,5); // brisem push i space iza push
      vector<string> args = getArgsFromString(s);
      if(args.size() != 1)
      {
        cout << "ERROR: Sintaksna greska, ocekivan samo jedan operand!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        // Gledam sta je arg: Ako je gpr, sp ili pc OK
        // Ako je nesto drugo (labela), ERROR
        // Posto arg pocinje sa %, to moram da uklonim
        args[0].erase(0,1);
        currentSection->incLocationCounterFor(4);
        if (!(isRegisterR0toR13(args[0]) || isRegisterSPorR14(args[0]) || isRegisterPCorR15(args[0])))
        {
          cout << "ERROR: Sintaksna greska! Argument uz instrukciju PUSH mora biti gpr, sp ili pc!" << endl;
          exit(-1);
        }
      } else if (drugiProlaz)
      {
        // push %gpr
        // sp <= sp - 4; mem32[sp] <= gpr;

        // Instrukcija smestanja podatka
        // MMMM = 0b0001
        // gpr[A] <= gpr[A] + D; mem32[gpr[A]] <= gpr[C]; 
        // gpr[A] == SP (R14); gpr[B] == gpr;

        vector<unsigned char> data = currentSection->getSectionData();

        args[0].erase(0,1);
        int gprA = 14;  // SP
        int gprB = resolveGeneralPurposeRegister(args[0]);

        unsigned char byte1 = 0b10000001;   // instrCode = 0b1000; instrMode = 0b0001;
        unsigned char byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);
        unsigned char byte3 = 0x0F;   // C = 0;
        unsigned char byte4 = 0b00000100;   // D = +4 = 0xFFC;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);
        currentSection->incLocationCounterFor(4);
      }
    } else if (isInstructionPop(s))
    {
      s.erase(0,4); // brisem pop i space iza pop
      vector<string> args = getArgsFromString(s);
      if(args.size() != 1)
      {
        cout << "ERROR: Sintaksna greska, ocekivan samo jedan operand!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        // Gledam sta je arg: Ako je gpr, sp ili pc OK
        // Ako je nesto drugo (labela), ERROR
        // Posto arg pocinje sa %, to moram da uklonim
        args[0].erase(0,1);
        currentSection->incLocationCounterFor(4);
        if (!(isRegisterR0toR13(args[0]) || isRegisterSPorR14(args[0]) || isRegisterPCorR15(args[0])))
        {
          cout << "ERROR: Sintaksna greska! Argument uz instrukciju POP mora biti gpr, sp ili pc!" << endl;
          exit(-1);
        }
      } else if (drugiProlaz)
      {
        // pop %gpr
        // gpr <= mem32[sp]; sp <= sp + 4

        // Instrukcija ucitavanja podataka
        // MMMM = 0b0011
        // gpr[A] <= mem32[gpr[B]]; gpr[B] <= gpr[B] + D;
        // gpr[A] == gpr; gpr[B] == SP (R14);

        vector<unsigned char> data = currentSection->getSectionData();

        args[0].erase(0,1);
        int gprA = resolveGeneralPurposeRegister(args[0]);
        int gprB = 14; // SP

        unsigned char byte1 = 0b10010011;   // instrCode = 0b1001; instrMode = 0b0011;
        unsigned char byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);
        unsigned char byte3 = 0b00000000;   // C = 0; najvisa 4b D = 0;
        unsigned char byte4 = 0b00000100;   // nizih 8b D = +4;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);
        currentSection->incLocationCounterFor(4);
      }
    } else if (isInstructionNot(s))
    {
      s.erase(0,4); // brisem not i space iza not
      vector<string> args = getArgsFromString(s);
      if(args.size() != 1)
      {
        cout << "ERROR: Sintaksna greska, ocekivan samo jedan operand!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        // Gledam sta je arg: Ako je gpr, sp ili pc OK
        // Ako je nesto drugo (labela), ERROR
        // Posto arg pocinje sa %, to moram da uklonim
        args[0].erase(0,1);
        currentSection->incLocationCounterFor(4);
        if (!(isRegisterR0toR13(args[0]) || isRegisterSPorR14(args[0]) || isRegisterPCorR15(args[0])))
        {
          cout << "ERROR: Sintaksna greska! Argument uz instrukciju NOT mora biti gpr, sp ili pc!" << endl;
          exit(-1);
        }
      } else if (drugiProlaz)
      {
        // not %gpr

        vector<unsigned char> data = currentSection->getSectionData();

        // args[0] - %gpr
        args[0].erase(0,1);

        int gprA = resolveGeneralPurposeRegister(args[0]);
        int gprB = gprA;

        unsigned char byte1 = 0b01100000;
        unsigned char byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);
        unsigned char byte3 = 0b00000000;
        unsigned char byte4 = 0b00000000;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);
        currentSection->incLocationCounterFor(4);
      }
    }
  }

  // Two Operand Instructions - xchg, add, sub, mul, div, and, or, xor, shl, shr, ld, st, csrrd, csrwr
  else if (isTwoOpInstruction(s))
  {
    if (isInstructionXchg(s))
    {
      s.erase(0,5); // uklanjam xchg i space odmah iza xchg
      vector<string> args = getArgsFromString(s);
      if(args.size() != 2)
      {
        cout << "ERROR: Sintaksna greska, ocekivana tacno dva operanda uz instrukciju XCHG!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);

        for(int i = 0; i < args.size(); i++)
        {
          // Proveravam da li su oba elementa u argsXchg gpr
          // Ako jesu, sve OK, ERROR ako nisu
          // Posto args[i] pocinje sa %, uklanjam %
          args[i].erase(0,1);
          if (!(isRegisterR0toR13(args[i]) || isRegisterSPorR14(args[i]) || isRegisterPCorR15(args[i])))
          {
            cout << "ERROR: Sintaksna greska! Argument uz instrukciju XCHG mora biti gpr, sp ili pc!" << endl;
            exit(-1);
          }
        }
      } else if (drugiProlaz)
      {
        // xchg %gprS, %gprD

        vector<unsigned char> data = currentSection->getSectionData();
        
        // args[0] - %gprS
        // args[1] - %gprD
        args[0].erase(0,1);
        args[1].erase(0,1);

        unsigned int gprS, gprD;

        gprS = resolveGeneralPurposeRegister(args[0]);
        gprD = resolveGeneralPurposeRegister(args[1]);

        unsigned char byte1 = 0b01000000;
        unsigned char byte2 = 0x0F & (unsigned char)gprS;
        // unsigned char byte3 = 0xF0 & (unsigned char)gprD;
        unsigned char byte3 = (unsigned char)((gprD << 4) & 0xF0);
        unsigned char byte4 = 0b00000000;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);
        currentSection->incLocationCounterFor(4);
      }
    } else if (isInstructionAdd(s))
    {
      s.erase(0,4); // uklanjam add i space odmah iza add
      vector<string> args = getArgsFromString(s);
      if(args.size() != 2)
      {
        cout << "ERROR: Sintaksna greska, ocekivana tacno dva operanda uz instrukciju ADD!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);

        for(int i = 0; i < args.size(); i++)
        {
          // Proveravam da li su oba elementa u argsAdd gpr
          // Ako jesu, sve OK, ERROR ako nisu
          // Posto args[i] pocinje sa %, uklanjam %
          args[i].erase(0,1);
          if (!(isRegisterR0toR13(args[i]) || isRegisterSPorR14(args[i]) || isRegisterPCorR15(args[i])))
          {
            cout << "ERROR: Sintaksna greska! Argument uz instrukciju ADD mora biti gpr, sp ili pc!" << endl;
            exit(-1);
          }
        }
      } else if (drugiProlaz)
      {
        // add %gprS, %gprD

        vector<unsigned char> data = currentSection->getSectionData();
      
        // args[0] - %gprS
        // args[1] - %gprD
        args[0].erase(0,1);
        args[1].erase(0,1);

        int gprA, gprB, gprC; // gprA <= gprB + gprC; gprD = gprD + gprS;

        gprB = resolveGeneralPurposeRegister(args[1]);
        gprC = resolveGeneralPurposeRegister(args[0]);
        gprA = gprB;

        unsigned char byte1 = 0b01010000;
        unsigned char byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);
        unsigned char byte3 = (((unsigned char)gprC) << 4) & 0xF0;
        unsigned char byte4 = 0b00000000;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);

        currentSection->incLocationCounterFor(4);
      }
    } else if (isInstructionSub(s))
    {
      s.erase(0,4); // uklanjam sub i space odmah iza sub
      vector<string> args = getArgsFromString(s);
      if(args.size() != 2)
      {
        cout << "ERROR: Sintaksna greska, ocekivana tacno dva operanda uz instrukciju SUB!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);

        for(int i = 0; i < args.size(); i++)
        {
          // Proveravam da li su oba elementa u argsSub gpr
          // Ako jesu, sve OK, ERROR ako nisu
          // Posto args[i] pocinje sa %, uklanjam %
          args[i].erase(0,1);
          if (!(isRegisterR0toR13(args[i]) || isRegisterSPorR14(args[i]) || isRegisterPCorR15(args[i])))
          {
            cout << "ERROR: Sintaksna greska! Argument uz instrukciju SUB mora biti gpr, sp ili pc!" << endl;
            exit(-1);
          }
        }
      } else if (drugiProlaz)
      {
        // sub %gprS, %gprD

        vector<unsigned char> data = currentSection->getSectionData();
      
        // args[0] - %gprS
        // args[1] - %gprD
        args[0].erase(0,1);
        args[1].erase(0,1);

        int gprA, gprB, gprC; // gprA <= gprB - gprC; gprD <= gprD - gprS;

        gprB = resolveGeneralPurposeRegister(args[1]);
        gprC = resolveGeneralPurposeRegister(args[0]);
        gprA = gprB;

        unsigned char byte1 = 0b01010001;
        unsigned char byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);
        unsigned char byte3 = (((unsigned char)gprC) << 4) & 0xF0;
        unsigned char byte4 = 0b00000000;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);

        currentSection->incLocationCounterFor(4);
      }
    } else if (isInstructionMul(s))
    {
      s.erase(0,4); // uklanjam mul i space odmah iza mul
      vector<string> args = getArgsFromString(s);
      if(args.size() != 2)
      {
        cout << "ERROR: Sintaksna greska, ocekivana tacno dva operanda uz instrukciju MUL!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);

        for(int i = 0; i < args.size(); i++)
        {
          // Proveravam da li su oba elementa u argsMul gpr
          // Ako jesu, sve OK, ERROR ako nisu
          // Posto args[i] pocinje sa %, uklanjam %
          args[i].erase(0,1);
          if (!(isRegisterR0toR13(args[i]) || isRegisterSPorR14(args[i]) || isRegisterPCorR15(args[i])))
          {
            cout << "ERROR: Sintaksna greska! Argument uz instrukciju MUL mora biti gpr, sp ili pc!" << endl;
            exit(-1);
          }
        }
      } else if (drugiProlaz)
      {
        // mul %gprS, %gprD

        vector<unsigned char> data = currentSection->getSectionData();
      
        // args[0] - %gprS
        // args[1] - %gprD
        args[0].erase(0,1);
        args[1].erase(0,1);

        int gprA, gprB, gprC; // gprA <= gprB * gprC; gprD <= gprD * gprS;

        gprB = resolveGeneralPurposeRegister(args[1]);
        gprC = resolveGeneralPurposeRegister(args[0]);
        gprA = gprB;

        unsigned char byte1 = 0b01010010;
        unsigned char byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);
        unsigned char byte3 = (((unsigned char)gprC) << 4) & 0xF0;
        unsigned char byte4 = 0b00000000;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);

        currentSection->incLocationCounterFor(4);
      }
    } else if (isInstructionDiv(s))
    {
      s.erase(0,4); // uklanjam div i space odmah iza div
      vector<string> args = getArgsFromString(s);
      if(args.size() != 2)
      {
        cout << "ERROR: Sintaksna greska, ocekivana tacno dva operanda uz instrukciju DIV!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);

        for(int i = 0; i < args.size(); i++)
        {
          // Proveravam da li su oba elementa u argsDiv gpr
          // Ako jesu, sve OK, ERROR ako nisu
          // Posto args[i] pocinje sa %, uklanjam %
          args[i].erase(0,1);
          if (!(isRegisterR0toR13(args[i]) || isRegisterSPorR14(args[i]) || isRegisterPCorR15(args[i])))
          {
            cout << "ERROR: Sintaksna greska! Argument uz instrukciju DIV mora biti gpr, sp ili pc!" << endl;
            exit(-1);
          }
        }
      } else if (drugiProlaz)
      {
        // div %gprS, %gprD

        vector<unsigned char> data = currentSection->getSectionData();
      
        // args[0] - %gprS
        // args[1] - %gprD
        args[0].erase(0,1);
        args[1].erase(0,1);

        int gprA, gprB, gprC; // gprA <= gprB / gprC; gprD <= gprD / gprS;

        gprB = resolveGeneralPurposeRegister(args[1]);
        gprC = resolveGeneralPurposeRegister(args[0]);
        gprA = gprB;

        unsigned char byte1 = 0b01010011;
        unsigned char byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);
        unsigned char byte3 = (((unsigned char)gprC) << 4) & 0xF0;
        unsigned char byte4 = 0b00000000;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);

        currentSection->incLocationCounterFor(4);
      }
    } else if (isInstructionAnd(s))
    {
      s.erase(0,4); // uklanjam and i space odmah iza and
      vector<string> args = getArgsFromString(s);
      if(args.size() != 2)
      {
        cout << "ERROR: Sintaksna greska, ocekivana tacno dva operanda uz instrukciju AND!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);

        for(int i = 0; i < args.size(); i++)
        {
          // Proveravam da li su oba elementa u argsAnd gpr
          // Ako jesu, sve OK, ERROR ako nisu
          // Posto args[i] pocinje sa %, uklanjam %
          args[i].erase(0,1);
          if (!(isRegisterR0toR13(args[i]) || isRegisterSPorR14(args[i]) || isRegisterPCorR15(args[i])))
          {
            cout << "ERROR: Sintaksna greska! Argument uz instrukciju AND mora biti gpr, sp ili pc!" << endl;
            exit(-1);
          }
        }
      } else if (drugiProlaz)
      {
        // and %gprS, %gprD

        vector<unsigned char> data = currentSection->getSectionData();
      
        // args[0] - %gprS
        // args[1] - %gprD
        args[0].erase(0,1);
        args[1].erase(0,1);

        int gprA, gprB, gprC; // gprA <= gprB & gprC; gprD <= gprD & gprS;

        gprB = resolveGeneralPurposeRegister(args[1]);
        gprC = resolveGeneralPurposeRegister(args[0]);
        gprA = gprB;

        unsigned char byte1 = 0b01100001;
        unsigned char byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);
        unsigned char byte3 = (((unsigned char)gprC) << 4) & 0xF0;
        unsigned char byte4 = 0b00000000;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);

        currentSection->incLocationCounterFor(4);
      }
    } else if (isInstructionOr(s))
    {
      s.erase(0,3); // uklanjam or i space odmah iza or
      vector<string> args = getArgsFromString(s);
      if(args.size() != 2)
      {
        cout << "ERROR: Sintaksna greska, ocekivana tacno dva operanda uz instrukciju OR!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);

        for(int i = 0; i < args.size(); i++)
        {
          // Proveravam da li su oba elementa u argsAnd gpr
          // Ako jesu, sve OK, ERROR ako nisu
          // Posto args[i] pocinje sa %, uklanjam %
          args[i].erase(0,1);
          if (!(isRegisterR0toR13(args[i]) || isRegisterSPorR14(args[i]) || isRegisterPCorR15(args[i])))
          {
            cout << "ERROR: Sintaksna greska! Argument uz instrukciju OR mora biti gpr, sp ili pc!" << endl;
            exit(-1);
          }
        }
      } else if (drugiProlaz)
      {
        // or %gprS, %gprD

        vector<unsigned char> data = currentSection->getSectionData();
      
        // args[0] - %gprS
        // args[1] - %gprD
        args[0].erase(0,1);
        args[1].erase(0,1);

        int gprA, gprB, gprC; // gprA <= gprB | gprC; gprD <= gprD | gprS;

        gprB = resolveGeneralPurposeRegister(args[1]);
        gprC = resolveGeneralPurposeRegister(args[0]);
        gprA = gprB;

        unsigned char byte1 = 0b01100010;
        unsigned char byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);
        unsigned char byte3 = (((unsigned char)gprC) << 4) & 0xF0;
        unsigned char byte4 = 0b00000000;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);

        currentSection->incLocationCounterFor(4);
      }
    } else if (isInstructionXor(s))
    {
      s.erase(0,4); // uklanjam xor i space odmah iza xor
      vector<string> args = getArgsFromString(s);
      if(args.size() != 2)
      {
        cout << "ERROR: Sintaksna greska, ocekivana tacno dva operanda uz instrukciju XOR!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);

        for(int i = 0; i < args.size(); i++)
        {
          // Proveravam da li su oba elementa u argsAnd gpr
          // Ako jesu, sve OK, ERROR ako nisu
          // Posto args[i] pocinje sa %, uklanjam %
          args[i].erase(0,1);
          if (!(isRegisterR0toR13(args[i]) || isRegisterSPorR14(args[i]) || isRegisterPCorR15(args[i])))
          {
            cout << "ERROR: Sintaksna greska! Argument uz instrukciju XOR mora biti gpr, sp ili pc!" << endl;
            exit(-1);
          }
        }
      } else if (drugiProlaz)
      {
        // xor %gprS, %gprD

        vector<unsigned char> data = currentSection->getSectionData();
      
        // args[0] - %gprS
        // args[1] - %gprD
        args[0].erase(0,1);
        args[1].erase(0,1);

        int gprA, gprB, gprC; // gprA <= gprB ^ gprC; gprD <= gprD ^ gprS;

        gprB = resolveGeneralPurposeRegister(args[1]);
        gprC = resolveGeneralPurposeRegister(args[0]);
        gprA = gprB;

        unsigned char byte1 = 0b01100011;
        unsigned char byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);
        unsigned char byte3 = (((unsigned char)gprC) << 4) & 0xF0;
        unsigned char byte4 = 0b00000000;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);

        currentSection->incLocationCounterFor(4);
      }
    } else if (isInstructionShl(s))
    {
      s.erase(0,4); // uklanjam shl i space odmah iza shl
      vector<string> args = getArgsFromString(s);
      if(args.size() != 2)
      {
        cout << "ERROR: Sintaksna greska, ocekivana tacno dva operanda uz instrukciju SHL!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);

        for(int i = 0; i < args.size(); i++)
        {
          // Proveravam da li su oba elementa u argsAnd gpr
          // Ako jesu, sve OK, ERROR ako nisu
          // Posto args[i] pocinje sa %, uklanjam %
          args[i].erase(0,1);
          if (!(isRegisterR0toR13(args[i]) || isRegisterSPorR14(args[i]) || isRegisterPCorR15(args[i])))
          {
            cout << "ERROR: Sintaksna greska! Argument uz instrukciju SHL mora biti gpr, sp ili pc!" << endl;
            exit(-1);
          }
        }
      } else if (drugiProlaz)
      {
        // shl %gprS, %gprD

        vector<unsigned char> data = currentSection->getSectionData();
      
        // args[0] - %gprS
        // args[1] - %gprD
        args[0].erase(0,1);
        args[1].erase(0,1);

        int gprA, gprB, gprC; // gprA <= gprB << gprC; gprD <= gprD << gprS;

        gprB = resolveGeneralPurposeRegister(args[1]);
        gprC = resolveGeneralPurposeRegister(args[0]);
        gprA = gprB;

        unsigned char byte1 = 0b01110000;
        unsigned char byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);
        unsigned char byte3 = (((unsigned char)gprC) << 4) & 0xF0;
        unsigned char byte4 = 0b00000000;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);

        currentSection->incLocationCounterFor(4);
      }
    } else if (isInstructionShr(s))
    {
      s.erase(0,4); // uklanjam shr i space odmah iza shr
      vector<string> args = getArgsFromString(s);
      if(args.size() != 2)
      {
        cout << "ERROR: Sintaksna greska, ocekivana tacno dva operanda uz instrukciju SHR!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);

        for(int i = 0; i < args.size(); i++)
        {
          // Proveravam da li su oba elementa u argsAnd gpr
          // Ako jesu, sve OK, ERROR ako nisu
          // Posto args[i] pocinje sa %, uklanjam %
          args[i].erase(0,1);
          if (!(isRegisterR0toR13(args[i]) || isRegisterSPorR14(args[i]) || isRegisterPCorR15(args[i])))
          {
            cout << "ERROR: Sintaksna greska! Argument uz instrukciju SHR mora biti gpr, sp ili pc!" << endl;
            exit(-1);
          }
        }
      } else if (drugiProlaz)
      {
        // shr %gprS, %gprD

        vector<unsigned char> data = currentSection->getSectionData();
      
        // args[0] - %gprS
        // args[1] - %gprD
        args[0].erase(0,1);
        args[1].erase(0,1);

        int gprA, gprB, gprC; // gprA <= gprB >> gprC; gprD <= gprD >> gprS;

        gprB = resolveGeneralPurposeRegister(args[1]);
        gprC = resolveGeneralPurposeRegister(args[0]);
        gprA = gprB;

        unsigned char byte1 = 0b01110001;
        unsigned char byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);
        unsigned char byte3 = (((unsigned char)gprC) << 4) & 0xF0;
        unsigned char byte4 = 0b00000000;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);
        currentSection->incLocationCounterFor(4);
      }
    } else if (isInstructionLd(s))
    {
      s.erase(0,3); // brisem ld i space iza
      // ld operand, %gpr
      vector<string> args = getArgsFromString(s);
      if (args.size() != 2)
      {
        cout << "ERROR: Sintaksna greska! Broj argumenata uz LD instrukciju mora biti 2!" << endl;
        exit(-1);
      }
        
      if (prviProlaz)
      {
        // args[0] -> operand
        // args[1] -> %gpr
        int res = ldAndStOperand(args[0]);
        if (res == -1)
        {
          cout << "ERROR: Sintaksna greska! Prvi argument uz LD instrukciju mora biti neki operand!" << endl;
          exit(-1);
        } else if (res == 5)
        {
          currentSection->incLocationCounterFor(4);
        } else if (res == 6)
        {
          currentSection->incLocationCounterFor(4);
        } else if (res == 2)
        {
          // cout << "res: " << res << endl;
          if (res == 2) // onda ima $ ispred simbola
          {
            args[0].erase(0,1);
          }

          if (!symTable.findSymbolByName(args[0]))  // ako je povratna vrednost nullptr
          {
            // cout << "Simbol " << args[0] << " ne postoji u tabeli simbola. Dodajem ga!" << endl;
            Symbol* newSym = new Symbol(args[0], 0, "UND", false, false, false); // stavljam da je lokalan
            symTable.addSymboltoSymbolTable(newSym);
          }

          currentSection->incLocationCounterFor(4);
        } else if (res == 4)
        {
          if (!symTable.findSymbolByName(args[0]))  // ako je povratna vrednost nullptr
          {
            // cout << "Simbol " << args[0] << " ne postoji u tabeli simbola. Dodajem ga!" << endl;
            Symbol* newSym = new Symbol(args[0], 0, "UND", false, false, false); // stavljam da je lokalan
            symTable.addSymboltoSymbolTable(newSym);
          }

          // UVEK RADIM PREKO DVE INSTRUKCIJE
          currentSection->incLocationCounterFor(8);   
        } else if (res == 7)
        {
          vector<string> newArgs = trimStringByPlus(args[0]);

          // newArgs[1] moze biti simbol ili literal

          if (newArgs.size() != 2)
          {
            cout << "ERROR: Sintaksna greska, LD instrukcija! Broj argumenata za string " << args[0] << " mora biti 2!" << endl;
            exit(-1);
          }
            
          if (resolveGeneralPurposeRegister(newArgs[0]) == -1)
          {
            cout << "ERROR: LD instrukcija! Ocekujem neki od registara opste namene!" << endl;
            exit(-1);
          }    

          if (isSymbol(newArgs[1]))
          {
            if (!symTable.findSymbolByName(newArgs[1]))  // ako je povratna vrednost nullptr
            {
              // cout << "Simbol " << newArgs[1] << " ne postoji u tabeli simbola. Dodajem ga!" << endl;
              Symbol* newSym = new Symbol(newArgs[1], 0, "UND", false, false, false); // stavljam da je lokalan
              symTable.addSymboltoSymbolTable(newSym);
            }
          } else if (isLiteral(newArgs[1]))
          {
            // Ako ne staje u 12b, prijaviti gresku u ASM!
            if (!stajeU12Bita(newArgs[1]))
            {
              cout << "ERROR: Literal " << newArgs[1] << " u instrukciji " << s << " ne staje u 12 bita!" << endl;
              exit(-1);
            }
          }

          currentSection->incLocationCounterFor(4);
        } else if (res == 1)
        {
          args[0].erase(0,1);
          obradaLiterala(args[0]);

          currentSection->incLocationCounterFor(4);
        } else if (res == 3)
        {
          // <literal> - vrednost iz memorije na adresi <literal>
          // obradaLiterala(args[0]);

          // UVEK RADIM PREKO DVE INSTRUKCIJE
          // <literal> smestam u bazen literala

          int newArg = 0;
          if (isHexNumber(s))
          {
            newArg = (int)stol(s, nullptr, 16);
          } else if (isDecNumber(s))
          {
            newArg = stoi(s);
          }
        
          // Stavljam ga u tabelu literala, nevazno je da li staje na 12b
          if (!currentSection->entryExistsInLiteralTable(newArg))
          {
            // Dodajem novi ulaz u tabelu literala
            Literal* newLit = new Literal(s, newArg, 4, 0);
            currentSection->getLiteralTable()->push_back(newLit);
          }
          
          currentSection->incLocationCounterFor(8);
        }

        if (args[1].find_first_of("%") == 0)
        {
          args[1].erase(0,1);
          if (resolveGeneralPurposeRegister(args[1]) == -1)
          {
            cout << "ERROR: Sintaksna greska! Drugi argument uz LD instrukciju mora biti neki od registara opste namene!" << endl;
            exit(-1);
          }    
        } else
        {
          cout << "ERROR: Sintaksna greska, LD instrukcija! String " << args[1] << " mora imati '%' na pocetku" << endl;
          exit(-1);
        } 
      } else if (drugiProlaz)
      {
        // ld operand, %gpr -> gpr <= operand

        // regularni operandi za load:
        // 1. $<literal>            - vrednost literal
        // 2. $<simbol>             - vrednost simbol
        // 3. <literal>             - vrednost iz memorije na adresi <literal>
        // 4. <simbol>              - vrednost iz memorije na adresi <simbol>
        // 5. %<reg>                - vrednost u registru <reg>
        // 6. [%<reg>]              - vrednost iz memorije na adresi <reg> 
        // 7. [%<reg> + <literal>] ili [%<reg> + <simbol>]

        // Ako u 7. vrednost nije moguce zapisati na 12b kao oznacenu vrednost, prijaviti gresku

        vector<unsigned char> data = currentSection->getSectionData();

        unsigned char byte1 = 0b00000000;
        unsigned char byte2 = 0b00000000;
        unsigned char byte3 = 0b00000000;
        unsigned char byte4 = 0b00000000;

        // args[0] -> operand
        // args[1] -> %gpr

        args[1].erase(0,1);
        int gprA = resolveGeneralPurposeRegister(args[1]);

        int res = ldAndStOperand(args[0]);

        if (res == 1)
        {
          // $<literal> - vrednost <literal>
          args[0].erase(0,1); // brisem $

          int num = 0;

          if (isHexNumber(args[0]))
            num = (int)stol(args[0], nullptr, 16);
          else
            num = stoi(args[0]);

          if (stajeU12Bita(args[0]))
          {
            // literal staje u 12b

            byte1 = 0b10010001; // instrCode = 0b1001; instrMode = 0b0001;

            // B i C su 0
            byte2 = (((unsigned char)gprA) << 4) & 0xF0;

            unsigned int tmp = (num >> 8) & 0b1111;
            byte3 = 0x00 | (((unsigned char)tmp) & 0x0F);
            byte4 = (unsigned char)(num & 0xFF);
          } else
          {
            // literal ne staje u 12b, mora se pristupiti bazenu literala

            byte1 = 0b10010010; // instrCode = 0b1001; instrMode = 0b0010;

            // byte2 = (((unsigned char)gprA) << 4) & 0xF0;

            //PROBA - B = 15 i C = 0
            int gprB = 15;
            byte2 = (((unsigned char)gprA) << 4) & 0xF0 | (((unsigned char)gprB) & 0x0F);

            int displacement = 0;

            if (currentSection->entryExistsInLiteralTable(num))
            {
              displacement = currentSection->getLiteralLocation(num) - currentSection->getLocCounter();

              // cout << hex << displacement << ", int displacement: " << displacement << endl;
              // cout << "bazen size " << currentSection->getLiteralTable()->size() << endl;
            } else
            {
              cout << "ERROR: Literal " << args[0] << " ne postoji u tabeli literala!" << endl;
              exit(-1);
            }

            unsigned int tmp = (displacement >> 8) & 0b1111;

            byte3 = 0x00 | (((unsigned char)tmp) & 0x0F);
            byte4 = (unsigned char)(displacement & 0xFF);
          }

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        } else if (res == 2)
        {
          // $<simbol> - vrednost <simbol>

          args[0].erase(0,1); // brisem $

          Symbol* sym = symTable.findSymbolByName(args[0]);
          
          // if (currentSection->getName() == sym->getSymbolSection())
          // {
          //   // Simbol se nalazi u istoj sekciji -> LAKSI SLUCAJ
          //   int val = sym->getSymbolValue();

          //   byte1 = 0b10010001;   // instrCode = 0b1001; instrMode = 0b0001;
          //   byte2 = (((unsigned char)gprA) << 4) & 0xF0;

          //   // B i C su 0
            
          //   unsigned int tmp = (val >> 8) & 0b1111;

          //   byte3 = 0x00 | (((unsigned char)tmp) & 0x0F);
          //   byte4 = (unsigned char)(val & 0xFF);
          // } else
          // {

          // PROMENA: Ipak me ne interesuje da li se simbol nalazi u istoj sekciji
          // Simbol se ne nalazi u istoj sekciji -> TEZI SLUCAJ
          vector<pair<int,int>> addrIndex = currentSection->addNewEntryAtTheEndOfLiteralTable(sym); // Ova addr mi treba za relok zapis

          int addr,index;
          for (auto it = addrIndex.begin(); it != addrIndex.end(); ++it)
          {
            addr = it->first;
            index = it->second;
          }

          // B = 15 i C = 0
          int gprB = 15;

          byte1 = 0b10010010; // instrCode = 0b1001; instrMode = 0b0010;
          byte2 = (((unsigned char)gprA) << 4) & 0xF0 | ((unsigned char) gprB) & 0x0F;

          int displacement = addr - currentSection->getLocCounter();

          unsigned int tmp = (displacement >> 8) & 0b1111;

          byte3 = 0x00 | (((unsigned char)tmp) & 0x0F);
          byte4 = (unsigned char)(displacement & 0xFF);

          RelocSymbol* relSym = new RelocSymbol(sym, addr, index);
          vector<RelocSymbol*> relData = currentSection->getRelocationTable();
          relData.push_back(relSym);
          currentSection->setRelocationData(relData);
          // }

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        } else if (res == 3)
        {
          // UVEK PREKO DVE INSTRUKCIJE
          // <literal> se smesta u bazen
          // <literal> - vrednost iz memorije na adresi <literal>

          int num = 0;

          if (isHexNumber(args[0]))
            num = (int)stol(args[0], nullptr, 16);
          else
            num = stoi(args[0]);
          
          // mora preko dve instrukcije
          // 2x MMMM = 0b0010

          // 1.
          int gprAtmp = gprA;

          byte1 = 0b10010010; // instrCode = 0b1001; instrMode = 0b0010;

          int gprB = 15;
          // B =15 C = 0
          byte2 = (((unsigned char)gprAtmp) << 4) & 0xF0 | ((unsigned char)gprB & 0x0F);

          int displacement = 0;

          if (currentSection->entryExistsInLiteralTable(num))
          {
            displacement = currentSection->getLiteralLocation(num) - currentSection->getLocCounter();
            // cout << "displacement = " << displacement << endl;
          } else
          {
            cout << "ERROR: Literal " << args[0] << " ne postoji u tabeli literala!" << endl;
            exit(-1);
          }

          unsigned int tmp = (displacement >> 8) & 0b1111;

          byte3 = 0x00 | (((unsigned char)tmp) & 0x0F);
          byte4 = (unsigned char)(displacement & 0xFF);

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);

          // --------------------------------------------------------------------------------------------

          // 2. ono sto je u gprAtmp upisujem u gprB, a gprA ostaje onaj gpr iz postavke instrukcije
          // C i D su 0

          byte1 = 0b10010010; // instrCode = 0b1001; instrMode = 0b0010;

          gprB = gprAtmp;
          byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);
          byte3 = 0b00000000;
          byte4 = 0b00000000;

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        } else if (res == 4)
        {
          // UVEK PREKO DVE INSTRUKCIJE
          // Nevazno je da li je simbol definisan u istoj sekciji ili ne
          // <simbol> svakako ide u bazen
          // <simbol> - vrednost iz memorije na adresi <simbol>

          Symbol* sym = symTable.findSymbolByName(args[0]);
          
          // Simbol je definisan u drugoj sekciji ili drugom fajlu

          // 2x MMMM = 0b0010;

          // 1.

          int gprAtmp = gprA;
          vector<pair<int,int>> addrIndex = currentSection->addNewEntryAtTheEndOfLiteralTable(sym); // Ova addr mi treba za relok zapis

          int addr,index;
          for (auto it = addrIndex.begin(); it != addrIndex.end(); ++it)
          {
            addr = it->first;
            index = it->second;
          }

          // B = 15 i C = 0

          int gprB = 15;

          byte1 = 0b10010010; // instrCode = 0b1001; instrMode = 0b0010;
          byte2 = (((unsigned char)gprAtmp) << 4) & 0xF0 | ((unsigned char)gprB & 0x0F);

          int displacement = addr - currentSection->getLocCounter();

          unsigned int tmp = (displacement >> 8) & 0b1111;

          byte3 = 0x00 | (((unsigned char)tmp) & 0x0F);
          byte4 = (unsigned char)(displacement & 0xFF);

          RelocSymbol* relSym = new RelocSymbol(sym, addr, index);
          vector<RelocSymbol*> relData = currentSection->getRelocationTable();
          relData.push_back(relSym);
          currentSection->setRelocationData(relData);

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);

          // --------------------------------------------------------------------------------------------

          // 2. ono sto je u gprAtmp upisujem u gprB, a gprA ostaje onaj gpr iz postavke instrukcije
          // C i D su 0

          byte1 = 0b10010010; // instrCode = 0b1001; instrMode = 0b0010;

          gprB = gprAtmp;
          byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);
          byte3 = 0b00000000;
          byte4 = 0b00000000;

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        } else if (res == 5)
        {
          // %<reg> - vrednost iz registra <reg>

          args[0].erase(0,1); // brisem %

          int gprB = resolveGeneralPurposeRegister(args[0]);

          byte1 = 0b10010001; // instrCode = 0b1001; instrMode = 0b0001;
          byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);

          // C i D su 0 -> byte3 i byte 4 ostaju 0

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        } else if (res == 6)
        {
          // [%<reg>] - vrednost iz memorije na adresi <reg>

          args[0].erase(0,2); // brisem [ i %
          args[0].erase(std::prev(args[0].end()));  // brisem ]

          int gprB = resolveGeneralPurposeRegister(args[0]);

          byte1 = 0b10010010; // instrCode = 0b1001; instrMode = 0b0010;
          byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);
          
          // C i D su 0 -> byte3 i byte 4 ostaju 0

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        } else if (res == 7)
        {
          // [%<reg> + <literal>] ili [%<reg> + <simbol>]

          vector<string> newArgs = trimStringByPlus(args[0]);

          // newArgs[0] -> %<reg>
          // newArgs[1] -> <literal> ili <simbol> -> Ako je simbol ERROR, ako je literal i ne staje na 12b ERROR

          if (isLiteral(newArgs[1]))
          {
            if (stajeU12Bita(newArgs[1]))
            {
              // OK

              int num = 0;

              if (isHexNumber(newArgs[1]))
                num = (int)stol(newArgs[1], nullptr, 16);
              else
                num = stoi(newArgs[1]);

              int gprB = resolveGeneralPurposeRegister(newArgs[0]);

              byte1 = 0b10010010; // instrCode = 0b1001; instrMode = 0b0010;
              byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);

              // C je 0

              unsigned int tmp = (num >> 8) & 0b1111;

              byte3 = 0x00 | (((unsigned char)tmp) & 0x0F);
              byte4 = (unsigned char)(num & 0xFF);

              data.push_back(byte1);
              data.push_back(byte2);
              data.push_back(byte3);
              data.push_back(byte4);

              currentSection->setSectionData(data);
              currentSection->incLocationCounterFor(4);
            } else
            {
              // ERROR
              cout << "ERROR: Vrednost literala " << newArgs[1] << " nije moguce zapisati na sirini od 12 bita!" << endl;
              exit(-1);
            }
          } else if (isSymbol(newArgs[1]))
          {
            cout << "ERROR: Konacna vrednost simbola " << newArgs[1] << " nije poznata u trenutku asembliranja!" << endl;
            exit(-1);
          }
        }
      }
    } else if (isInstructionSt(s))
    {
      s.erase(0,3); // brisem st i space iza
      // st %gpr, operand
      vector<string> args = getArgsFromString(s);
      if (args.size() != 2)
      {
        cout << "ERROR: Sintaksna greska! Broj argumenata uz ST instrukciju mora biti 2!" << endl;
        exit(-1);
      }
        
      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);

        // args[0] -> %gpr
        // args[1] -> operand
        if (args[0].find_first_of("%") == 0)
        {
          args[0].erase(0,1);
          if (resolveGeneralPurposeRegister(args[0]) == -1)
          {
            cout << "ERROR: Prvi argument uz ST instrukciju mora biti neki od registara opste namene!" << endl;
            exit(-1);
          }
        } else
        {
          cout << "ERROR: Sintaksna greska, ST instrukcija! String " << args[0] << " mora imati '%' na pocetku" << endl;
          exit(-1);
        }

        int res = ldAndStOperand(args[1]);
        if (res == -1)
        {
          cout << "ERROR: Sintaksna greska! Drugi argument uz ST instrukciju mora biti neki operand!" << endl;
          exit(-1);
        }
          
        else if (res == 2 || res == 4)
        {
          // cout << "res: " << res << endl;
          if (res == 2) // onda ima $ ispred simbola
          {
            args[1].erase(0,1);
          } 
          if (!symTable.findSymbolByName(args[1]))  // ako je povratna vrednost nullptr
          {
            cout << "Simbol " << args[1] << " ne postoji u tabeli simbola. Dodajem ga!" << endl;
            Symbol* newSym = new Symbol(args[1], 0, "UND", false, false, false); // stavljam da je lokalan
            symTable.addSymboltoSymbolTable(newSym);
          }
        } else if (res == 7)
        {
          vector<string> newArgs = trimStringByPlus(args[1]);

          if (newArgs.size() != 2)
          {
            cout << "ERROR: Sintaksna greska, ST instrukcija! Broj argumenata za string " << args[1] << " mora biti 2!" << endl;
            exit(-1);
          }

          if (resolveGeneralPurposeRegister(newArgs[0]) == -1)
          {
            cout << "ERROR: Sintaksna greska, ST instrukcija! Ocekujem neki od registara opste namene!" << endl;
            exit(-1);
          }
            
          if (isSymbol(newArgs[1]))
          {
            if (!symTable.findSymbolByName(newArgs[1]))  // ako je povratna vrednost nullptr
            {
              cout << "Simbol " << newArgs[1] << " ne postoji u tabeli simbola. Dodajem ga!" << endl;
              Symbol* newSym = new Symbol(newArgs[1], 0, "UND", false, false, false); // stavljam da je lokalan
              symTable.addSymboltoSymbolTable(newSym);
            }
          } else if (isLiteral(newArgs[1]))
          {
            // Ako ne staje u 12b, prijaviti gresku u ASM!
            if (!stajeU12Bita(newArgs[1]))
            {
              cout << "ERROR: Literal " << newArgs[1] << " u instrukciji " << s << " ne staje u 12 bita!" << endl;
              exit(-1);
            }
          }
        } else if (res == 1)
        {
          args[1].erase(0,1);
          obradaLiterala(args[1]);
        } else if (res == 3)
        {
          obradaLiterala(args[1]);
        }
      } else if (drugiProlaz)
      {
        // st %gpr, operand -> operand <= gpr

        // regularni operandi za store:
        // 3. <literal>             - vrednost iz memorije na adresi <literal>
        // 4. <simbol>              - vrednost iz memorije na adresi <simbol>
        // 6. [%<reg>]              - vrednost iz memorije na adresi <reg> 
        // 7. [%<reg> + <literal>]  - vrednost iz memorije na adresi <reg> + <literal>

        // Ako u 7., vrednost nije moguce zapisati na 12b kao oznacenu vrednost, prijaviti gresku

        // args[0] -> %gpr
        // args[1] -> operand

        vector<unsigned char> data = currentSection->getSectionData();

        unsigned char byte1 = 0b00000000;
        unsigned char byte2 = 0b00000000;
        unsigned char byte3 = 0b00000000;
        unsigned char byte4 = 0b00000000;

        args[0].erase(0,1);
        int gprC = resolveGeneralPurposeRegister(args[0]);

        int res = ldAndStOperand(args[1]);

        if (res == 3)         // <literal>
        {
          int num = 0;

          if (isHexNumber(args[1]))
            num = (int)stol(args[1], nullptr, 16);
          else
            num = stoi(args[1]);

          if (stajeU12Bita(args[1]))
          {
            // A = 0; B = 0
            // literal staje u 12b
            byte1 = 0b10000000; // instrCode = 0b1000; instrMode = 0b0000;

            unsigned int tmp = (num >> 8) & 0b1111;

            byte3 = ((((unsigned char)gprC) << 4) & 0xF0) | (((unsigned char)tmp) & 0x0F);
            byte4 = (unsigned char)(num & 0xFF);            
          } else
          {
            // A = 15; B = 0
            // literal NE staje u 12b, mora se pristupiti bazenu literala
            byte1 = 0b10000010; // instrCode = 0b1000; instrMode = 0b0010;
            byte2 = 0b11110000;

            int displacement = 0;

            if (currentSection->entryExistsInLiteralTable(num))
            {
              displacement = currentSection->getLiteralLocation(num) - currentSection->getLocCounter();
            } else
            {
              cout << "ERROR: Literal " << args[1] << " ne postoji u tabeli literala!" << endl;
              exit(-1);
            }

            unsigned int tmp = (displacement >> 8) & 0b1111;

            byte3 = ((((unsigned char)gprC) << 4) & 0xF0) | (((unsigned char)tmp) & 0x0F);
            byte4 = (unsigned char)(displacement & 0xFF);
          }

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        } else if (res == 4)  // <simbol>
        {
          // A = 15; B = 0

          Symbol* sym = symTable.findSymbolByName(args[1]);
          
          // if (currentSection->getName() == sym->getSymbolSection())
          // {
          //   // Simbol se nalazi u istoj sekciji -> LAKSI SLUCAJ
          //   int val = sym->getSymbolValue();

          //   byte1 = 0b10000000;   // instrCode = 0b1000; instrMode = 0b0000;
            
          //   unsigned int tmp = (val >> 8) & 0b1111;

          //   byte3 = ((((unsigned char)gprC) << 4) & 0xF0) | (((unsigned char)tmp) & 0x0F);
          //   byte4 = (unsigned char)(val & 0xFF);
          // } else
          // {

          // PROMENA: Ne insteresuje me da li se simbol nalazi u istoj sekciji
          // Simbol se ne nalazi u istoj sekciji -> TEZI SLUCAJ
          vector<pair<int,int>> addrIndex = currentSection->addNewEntryAtTheEndOfLiteralTable(sym); // Ova addr mi treba za relok zapis

          int addr,index;
          for (auto it = addrIndex.begin(); it != addrIndex.end(); ++it)
          {
            addr = it->first;
            index = it->second;
          }

          byte1 = 0b10000010; // instrCode = 0b1000; instrMode = 0b0010;
          byte2 = 0b11110000;

          int displacement = addr - currentSection->getLocCounter();

          unsigned int tmp = (displacement >> 8) & 0b1111;

          byte3 = ((((unsigned char)gprC) << 4) & 0xF0) | (((unsigned char)tmp) & 0x0F);
          byte4 = (unsigned char)(displacement & 0xFF);

          RelocSymbol* relSym = new RelocSymbol(sym, addr, index);
          vector<RelocSymbol*> relData = currentSection->getRelocationTable();
          relData.push_back(relSym);
          currentSection->setRelocationData(relData);
          // }

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        } else if (res == 6)  // [%<reg>]
        {
          // A je reg (args[1])
          // B je 0

          args[1].erase(0,2); // brisem [ i %
          args[1].erase(std::prev(args[1].end()));  // brisem ]

          int gprA = resolveGeneralPurposeRegister(args[1]);

          byte1 = 0b10000000; // instrCode = 0b1000; instrMode = 0b0000;
          byte2 = ((((unsigned char)gprA) << 4) & 0xF0);
          byte3 = ((((unsigned char)gprC) << 4) & 0xF0);
          // byte4 ostaje 0

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        } else if (res == 7)  // [%<reg> + <literal>] ili [%<reg> + <simbol>]
        {
          vector<string> newArgs = trimStringByPlus(args[1]);

          // newArgs[0] -> %<reg>
          // newArgs[1] -> <literal> ili <simbol> -> Ako je simbol ERROR, ako je literal i ne staje na 12b ERROR

          if (isLiteral(newArgs[1]))
          {
            if (stajeU12Bita(newArgs[1]))
            {
              // OK

              int num = 0;

              if (isHexNumber(newArgs[1]))
                num = (int)stol(newArgs[1], nullptr, 16);
              else
                num = stoi(newArgs[1]);

              int gprA = resolveGeneralPurposeRegister(newArgs[0]);
              // B je 0

              byte1 = 0b10000000; // instrCode = 0b1000; instrMode = 0b0000;
              byte2 = ((((unsigned char)gprA) << 4) & 0xF0);

              unsigned int tmp = (num >> 8) & 0b1111;

              byte3 = ((((unsigned char)gprC) << 4) & 0xF0) | (((unsigned char)tmp) & 0x0F);
              byte4 = (unsigned char)(num & 0xFF);

              data.push_back(byte1);
              data.push_back(byte2);
              data.push_back(byte3);
              data.push_back(byte4);

              currentSection->setSectionData(data);
              currentSection->incLocationCounterFor(4);
            } else
            {
              // ERROR
              cout << "ERROR: Vrednost literala " << newArgs[1] << " nije moguce zapisati na sirini od 12 bita!" << endl;
              exit(-1);
            }
          } else if (isSymbol(newArgs[1]))
          {
            cout << "ERROR: Konacna vrednost simbola " << newArgs[1] << " nije poznata u trenutku asembliranja!" << endl;
            exit(-1);
          }
        }
        // Ostali operandi ne dolaze u obzir za ST
      }
    } else if (isInstructionCsrrd(s))
    {
      // csrrd %csr, %gpr
      s.erase(0,6);   // brisem csrrd i space nakon njega
      vector<string> args = getArgsFromString(s);
      if (args.size() != 2)
      {
        cout << "ERROR: Sintaksna greska, ocekivana tacno dva operanda uz instrukciju CSRRD!" << endl;
        exit(-1);
      }
        
      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);

        args[0].erase(0,1); // brisem %
        if (!(isRegisterCause(args[0]) || isRegisterHandler(args[0]) || isRegisterStatus(args[0])))
        {
          cout << "ERROR: Prvi argument uz CSRRD instrukciju mora biti jedan od csr registara!" << endl;
          exit(-1);
        }
          
        args[1].erase(0,1); // brisem %
        if (!(isRegisterR0toR13(args[1]) || isRegisterSPorR14(args[1]) || isRegisterPCorR15(args[1])))
        {
          cout << "ERROR: Drugi argument uz CSRRD instrukciju mora biti neki od gpr registara, pc ili sp!" << endl;
          exit(-1);
        }
      } else if (drugiProlaz)
      {
        // csrrd %csr, %gpr
        // gpr <= csr

        vector<unsigned char> data = currentSection->getSectionData();

        // args[0] -> %csr
        // args[1] -> %gpr
        args[0].erase(0,1);
        args[1].erase(0,1);

        int gprA = resolveGeneralPurposeRegister(args[1]);
        int csrB = resolveControlAndStatusRegister(args[0]);        

        unsigned char byte1 = 0b10010000;   // instrCode = 0b1001; instrMode = 0b0000;
        unsigned char byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)csrB) & 0x0F);
        unsigned char byte3 = 0b00000000;   // C = 0;
        unsigned char byte4 = 0b00000000;   // D = 0;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);
        currentSection->incLocationCounterFor(4);      
      }
    } else if (isInstructionCsrwr(s))
    {
      s.erase(0,6);   // brisem csrwr i space nakon njega
      vector<string> args = getArgsFromString(s);
      if (args.size() != 2)
      {
        cout << "ERROR: Sintaksna greska, ocekivana tacno dva operanda uz instrukciju CSRWR!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);

        args[0].erase(0,1); // brisem %
        if (!(isRegisterR0toR13(args[0]) || isRegisterSPorR14(args[0]) || isRegisterPCorR15(args[0])))
        {
          cout << "ERROR: Prvi argument uz CSRWR instrukciju mora biti neki od gpr registara, pc ili sp!" << endl;
          exit(-1);
        }

        args[1].erase(0,1); // brisem %
        if (!(isRegisterCause(args[1]) || isRegisterHandler(args[1]) || isRegisterStatus(args[1])))
        {
          cout << "ERROR: Drugi argument uz CSRWR instrukciju mora biti jedan od csr registara!" << endl;
          exit(-1);
        }          
      } else if (drugiProlaz)
      {
        // csrwr %gpr, %csr
        // csr <= gpr

        vector<unsigned char> data = currentSection->getSectionData();
        
        // args[0] -> %gpr
        // args[1] -> %csr
        args[0].erase(0,1);
        args[1].erase(0,1);

        int csrA = resolveControlAndStatusRegister(args[1]);
        int gprB = resolveGeneralPurposeRegister(args[0]);
        
        unsigned char byte1 = 0b10010100;   // instrCode = 0b1001; instrMode = 0b0100;
        unsigned char byte2 = ((((unsigned char)csrA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);
        unsigned char byte3 = 0b00000000;   // C = 0;
        unsigned char byte4 = 0b00000000;   // D = 0;

        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);

        currentSection->setSectionData(data);
        currentSection->incLocationCounterFor(4);
      }
    }
  }

  // Three Operand Instructions - beq, bne, bgt
  else if (isThreeOpInstruction(s))
  {
    if (isInstructionBeq(s))
    {
      s.erase(0,4); // brisem beq i space iza beq
      vector<string> args = getArgsFromString(s);
      if (args.size() != 3)
      {
        cout << "ERROR: Sintaksna greska! Ocekivana tacno tri operanda uz instrukciju BEQ!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);
        for(int i = 0; i < args.size() - 1; i++)    // Za prva dva argumenta, %gpr, %gpr
        {
          // Proveravam da li su oba elementa u args gpr
          // Ako jesu, sve OK, ERROR ako nisu
          // Posto args[i] pocinje sa %, uklanjam %
          args[i].erase(0,1);
          if (!(isRegisterR0toR13(args[i]) || isRegisterSPorR14(args[i]) || isRegisterPCorR15(args[i])))
          {
            cout << "ERROR: Sintaksna greska! Prva dva operanda uz BEQ instrukciju moraju biti gpr, sp ili pc!" << endl;
            exit(-1);
          }
        }

        // arg[2] je operand -> literal ili simbol, ERROR ako je reg
        if (isRegisterR0toR13(args[2]) || isRegisterSPorR14(args[2]) || isRegisterPCorR15(args[2]))
        {
          cout << "ERROR: Sintaksna greska! Treci operand uz BEQ instrukciju mora biti literal ili simbol!" << endl;
          exit(-1);
        } else if (isLiteral(args[2]))
        {
          // cout << "Literal " << args[2] << endl;
          obradaLiterala(args[2]);
        } else  // labela
        {
          // Ako jeste, proveravam da li se taj simbol nalazi u tabeli simbola
          // Ako se ne nalazi, dodajem ga sa vrednoscu 0 i sekcijom UND
          // Na kraju prvog prolaza, ako ima simbola sam UND sekcijom koji nisu oznaceni kao extern, prijavljujem gresku u asm
          // U drugom prolazu ide u relokacionu tabelu ako je sve OK
          if (!symTable.findSymbolByName(args[2]))  // ako je povratna vrednost nullptr
          {
            // cout << "Simbol " << args[2] << " ne postoji u tabeli simbola. Dodajem ga!" << endl;
            Symbol* newSym = new Symbol(args[2], 0, "UND", false, false, false); // stavljam da je lokalan
            symTable.addSymboltoSymbolTable(newSym);
          }
          // Ako ga ima u tabeli simbola, sve OK, idemo dalje
        }
      } else if (drugiProlaz)
      {
        // beq %gpr1, %gpr2, operand - <literal> ili <simbol>
        // if (gpr[B] == gpr[C]) pc <= gpr[A] + D;

        // args[0] -> %gpr1 -> gpr[B]
        // args[2] -> %gpr2 -> gpr[C]
        // A = 15 (PC); D = operand

        vector<unsigned char> data = currentSection->getSectionData();

        unsigned char byte1 = 0b00000000;
        unsigned char byte2 = 0b00000000;
        unsigned char byte3 = 0b00000000;
        unsigned char byte4 = 0b00000000;

        args[0].erase(0,1);
        args[1].erase(0,1);

        int gprA = 15;
        int gprB = resolveGeneralPurposeRegister(args[0]);
        int gprC = resolveGeneralPurposeRegister(args[1]);

        byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);

        if (isLiteral(args[2]))
        {
          int num = 0;
          
          if (isHexNumber(args[2]))
            num = (int)stol(args[2], nullptr, 16);
          else
            num = stoi(args[2]);

          if (!stajeU12Bita(args[2])) // u D bite smestam pomeraj do literala iz bazena literala
          {
            byte1 = 0b00111001;   // instrCode = 0b0011; instrMode = 0b1001;

            int displacement = 0;

            if (currentSection->entryExistsInLiteralTable(num))
            {
              displacement = currentSection->getLiteralLocation(num) - currentSection->getLocCounter();
            } else
            {
              cout << "ERROR: Literal " << args[2] << " ne postoji u tabeli literala!" << endl;
              exit(-1);
            }

            unsigned int tmp = (displacement >> 8) & 0b1111;

            byte3 = ((((unsigned char)gprC) << 4) & 0xF0) | (((unsigned char)tmp) & 0x0F);
            byte4 = (unsigned char)(displacement & 0xFF);
          } else // literal moze da se smesti u D bite
          {
            byte1 = 0b00110001;   // instrCode = 0b0011; instrMode = 0b0001;
            
            unsigned int tmp = (num >> 8) & 0b1111;

            byte3 = ((((unsigned char)gprC) << 4) & 0xF0) | (((unsigned char)tmp) & 0x0F);
            byte4 = (unsigned char)(num & 0xFF);
          }

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        } else if (isSymbol(args[2]))
        {
          Symbol* sym = symTable.findSymbolByName(args[2]);
          
          // if (currentSection->getName() == sym->getSymbolSection())
          // {
          //   // Simbol se nalazi u istoj sekciji -> LAKSI SLUCAJ
          //   int val = sym->getSymbolValue();

          //   byte1 = 0b00110001;   // instrCode = 0b0011; instrMode = 0b0001;
            
          //   unsigned int tmp = (val >> 8) & 0b1111;

          //   byte3 = ((((unsigned char)gprC) << 4) & 0xF0) | (((unsigned char)tmp) & 0x0F);
          //   byte4 = (unsigned char)(val & 0xFF);
          // } else
          // {

          // PROMENA: Ne interesuje me da li se simbol nalazi u istoj sekciji
          // Simbol se ne nalazi u istoj sekciji -> TEZI SLUCAJ
          vector<pair<int,int>> addrIndex = currentSection->addNewEntryAtTheEndOfLiteralTable(sym); // Ova addr mi treba za relok zapis

          int addr,index;
          for (auto it = addrIndex.begin(); it != addrIndex.end(); ++it)
          {
            addr = it->first;
            index = it->second;
          }

          byte1 = 0b00111001;   // instrCode = 0b0011; instrMode = 0b1001;

          int displacement = addr - currentSection->getLocCounter();

          unsigned int tmp = (displacement >> 8) & 0b1111;

          byte3 = ((((unsigned char)gprC) << 4) & 0xF0) | (((unsigned char)tmp) & 0x0F);
          byte4 = (unsigned char)(displacement & 0xFF);

          RelocSymbol* relSym = new RelocSymbol(sym, addr, index);
          vector<RelocSymbol*> relData = currentSection->getRelocationTable();
          relData.push_back(relSym);
          currentSection->setRelocationData(relData);
          // }

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        }
      }
    } else if (isInstructionBne(s))
    {
      s.erase(0,4); // brisem bne i space iza bne
      vector<string> args = getArgsFromString(s);
      if (args.size() != 3)
      {
        cout << "ERROR: Sintaksna greska! Ocekivana tacno tri operanda uz instrukciju BNE!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);
        for(int i = 0; i < args.size() - 1; i++)    // Za prva dva argumenta, %gpr, %gpr
        {
          // Proveravam da li su oba elementa u args gpr
          // Ako jesu, sve OK, ERROR ako nisu
          // Posto args[i] pocinje sa %, uklanjam %
          args[i].erase(0,1);
          if (!(isRegisterR0toR13(args[i]) || isRegisterSPorR14(args[i]) || isRegisterPCorR15(args[i])))
          {
            cout << "ERROR: Sintaksna greska! Prva dva operanda uz BNE instrukciju moraju biti gpr, sp ili pc!" << endl;
            exit(-1);
          }
        }

        // arg[2] je operand -> literal ili simbol, ERROR ako je reg
        if (isRegisterR0toR13(args[2]) || isRegisterSPorR14(args[2]) || isRegisterPCorR15(args[2]))
        {
          cout << "ERROR: Sintaksna greska! Treci operand uz BNE instrukciju mora biti literal ili simbol!" << endl;
          exit(-1);
        } else if (isLiteral(args[2]))
        {
          // cout << "Literal " << args[2] << endl;
          obradaLiterala(args[2]);
        } else  // labela
        {
          // Ako jeste, proveravam da li se taj simbol nalazi u tabeli simbola
          // Ako se ne nalazi, dodajem ga sa vrednoscu 0 i sekcijom UND
          // Na kraju prvog prolaza, ako ima simbola sam UND sekcijom koji nisu oznaceni kao extern, prijavljujem gresku u asm
          // U drugom prolazu ide u relokacionu tabelu ako je sve OK
          if (!symTable.findSymbolByName(args[2]))  // ako je povratna vrednost nullptr
          {
            // cout << "Simbol " << args[2] << " ne postoji u tabeli simbola. Dodajem ga!" << endl;
            Symbol* newSym = new Symbol(args[2], 0, "UND", false, false, false); // stavljam da je lokalan
            symTable.addSymboltoSymbolTable(newSym);
          } 
          // Ako ga ima u tabeli simbola, sve OK, idemo dalje
        }
      } else if (drugiProlaz)
      {
        // bne %gpr1, %gpr2, operand - <literal> ili <simbol>
        // if (gpr[B] != gpr[C]) pc <= gpr[A] + D;

        // args[0] -> %gpr1 -> gpr[B]
        // args[2] -> %gpr2 -> gpr[C]
        // A = 15 (PC); D = operand

        vector<unsigned char> data = currentSection->getSectionData();

        unsigned char byte1 = 0b00000000;
        unsigned char byte2 = 0b00000000;
        unsigned char byte3 = 0b00000000;
        unsigned char byte4 = 0b00000000;

        args[0].erase(0,1);
        args[1].erase(0,1);

        int gprA = 15;
        int gprB = resolveGeneralPurposeRegister(args[0]);
        int gprC = resolveGeneralPurposeRegister(args[1]);

        byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);

        if (isLiteral(args[2]))
        {
          int num = 0;
          
          if (isHexNumber(args[2]))
            num = (int)stol(args[2], nullptr, 16);
          else
            num = stoi(args[2]);

          if (!stajeU12Bita(args[2])) // u D bite smestam pomeraj do literala iz bazena literala
          {
            byte1 = 0b00111010;   // instrCode = 0b0011; instrMode = 0b1010;

            int displacement = 0;

            if (currentSection->entryExistsInLiteralTable(num))
            {
              displacement = currentSection->getLiteralLocation(num) - currentSection->getLocCounter();
            } else
            {
              cout << "ERROR: Literal " << args[2] << " ne postoji u tabeli literala!" << endl;
              exit(-1);
            }

            unsigned int tmp = (displacement >> 8) & 0b1111;

            byte3 = ((((unsigned char)gprC) << 4) & 0xF0) | (((unsigned char)tmp) & 0x0F);
            byte4 = (unsigned char)(displacement & 0xFF);
          } else // literal moze da se smesti u D bite
          {
            byte1 = 0b00110010;   // instrCode = 0b0011; instrMode = 0b0010;
            
            unsigned int tmp = (num >> 8) & 0b1111;

            byte3 = ((((unsigned char)gprC) << 4) & 0xF0) | (((unsigned char)tmp) & 0x0F);
            byte4 = (unsigned char)(num & 0xFF);
          }

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        } else if (isSymbol(args[2]))
        {
          Symbol* sym = symTable.findSymbolByName(args[2]);
          
          // if (currentSection->getName() == sym->getSymbolSection())
          // {
          //   // Simbol se nalazi u istoj sekciji -> LAKSI SLUCAJ
          //   int val = sym->getSymbolValue();

          //   byte1 = 0b00110010;   // instrCode = 0b0011; instrMode = 0b0010;
            
          //   unsigned int tmp = (val >> 8) & 0b1111;

          //   byte3 = ((((unsigned char)gprC) << 4) & 0xF0) | (((unsigned char)tmp) & 0x0F);
          //   byte4 = (unsigned char)(val & 0xFF);
          // } else
          // {

          // PROMENA: Ne interesuje me da li simbol u istoj sekciji
          // Simbol se ne nalazi u istoj sekciji -> TEZI SLUCAJ
          vector<pair<int,int>> addrIndex = currentSection->addNewEntryAtTheEndOfLiteralTable(sym); // Ova addr mi treba za relok zapis

          int addr,index;
          for (auto it = addrIndex.begin(); it != addrIndex.end(); ++it)
          {
            addr = it->first;
            index = it->second;
          }

          byte1 = 0b00111010;   // instrCode = 0b0011; instrMode = 0b1010;

          int displacement = addr - currentSection->getLocCounter();

          unsigned int tmp = (displacement >> 8) & 0b1111;

          byte3 = ((((unsigned char)gprC) << 4) & 0xF0) | (((unsigned char)tmp) & 0x0F);
          byte4 = (unsigned char)(displacement & 0xFF);

          RelocSymbol* relSym = new RelocSymbol(sym, addr, index);
          vector<RelocSymbol*> relData = currentSection->getRelocationTable();
          relData.push_back(relSym);
          currentSection->setRelocationData(relData);
          // }

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        }
      }
    } else if (isInstructionBgt(s))
    {
      s.erase(0,4); // brisem bgt i space iza bgt
      vector<string> args = getArgsFromString(s);
      if (args.size() != 3)
      {
        cout << "ERROR: Sintaksna greska! Ocekivana tacno tri operanda uz instrukciju BGT!" << endl;
        exit(-1);
      }

      if (prviProlaz)
      {
        currentSection->incLocationCounterFor(4);
        for(int i = 0; i < args.size() - 1; i++)    // Za prva dva argumenta, %gpr, %gpr
        {
          // Proveravam da li su oba elementa u args gpr
          // Ako jesu, sve OK, ERROR ako nisu
          // Posto args[i] pocinje sa %, uklanjam %
          args[i].erase(0,1);
          if (!(isRegisterR0toR13(args[i]) || isRegisterSPorR14(args[i]) || isRegisterPCorR15(args[i])))
          {
            cout << "ERROR: Sintaksna greska! Prva dva operanda uz BGT instrukciju moraju biti gpr, sp ili pc!" << endl;
            exit(-1);
          }
        }

        // arg[2] je operand -> literal ili simbol, ERROR ako je reg
        if (isRegisterR0toR13(args[2]) || isRegisterSPorR14(args[2]) || isRegisterPCorR15(args[2]))
        {
          cout << "ERROR: Sintaksna greska! Treci operand uz BGT instrukciju mora biti literal ili simbol!" << endl;
        } else if (isLiteral(args[2]))
        {
          // cout << "Literal " << args[2] << endl;
          obradaLiterala(args[2]);
        } else  // labela
        {
          // Ako jeste, proveravam da li se taj simbol nalazi u tabeli simbola
          // Ako se ne nalazi, dodajem ga sa vrednoscu 0 i sekcijom UND
          // Na kraju prvog prolaza, ako ima simbola sam UND sekcijom koji nisu oznaceni kao extern, prijavljujem gresku u asm
          // U drugom prolazu ide u relokacionu tabelu ako je sve OK
          if (!symTable.findSymbolByName(args[2]))  // ako je povratna vrednost nullptr
          {
            // cout << "Simbol " << args[2] << " ne postoji u tabeli simbola. Dodajem ga!" << endl;
            Symbol* newSym = new Symbol(args[2], 0, "UND", false, false, false); // stavljam da je lokalan
            symTable.addSymboltoSymbolTable(newSym);
          } 
          // Ako ga ima u tabeli simbola, sve OK, idemo dalje
        }
      } else if (drugiProlaz)
      {
        // bgt %gpr1, %gpr2, operand - <literal> ili <simbol>
        // if (gpr[B] signed > gpr[C]) pc <= gpr[A] + D;

        // args[0] -> %gpr1 -> gpr[B]
        // args[2] -> %gpr2 -> gpr[C]
        // A = 15 (PC); D = operand

        vector<unsigned char> data = currentSection->getSectionData();

        unsigned char byte1 = 0b00000000;
        unsigned char byte2 = 0b00000000;
        unsigned char byte3 = 0b00000000;
        unsigned char byte4 = 0b00000000;

        args[0].erase(0,1);
        args[1].erase(0,1);

        int gprA = 15;
        int gprB = resolveGeneralPurposeRegister(args[0]);
        int gprC = resolveGeneralPurposeRegister(args[1]);

        byte2 = ((((unsigned char)gprA) << 4) & 0xF0) | (((unsigned char)gprB) & 0x0F);

        if (isLiteral(args[2]))
        {
          int num = 0;
          
          if (isHexNumber(args[2]))
            num = (int)stol(args[2], nullptr, 16);
          else
            num = stoi(args[2]);

          if (!stajeU12Bita(args[2])) // u D bite smestam pomeraj do literala iz bazena literala
          {
            byte1 = 0b00111011;   // instrCode = 0b0011; instrMode = 0b1011;

            int displacement = 0;

            if (currentSection->entryExistsInLiteralTable(num))
            {
              displacement = currentSection->getLiteralLocation(num) - currentSection->getLocCounter();
            } else
            {
              cout << "ERROR: Literal " << args[2] << " ne postoji u tabeli literala!" << endl;
              exit(-1);
            }

            unsigned int tmp = (displacement >> 8) & 0b1111;

            byte3 = ((((unsigned char)gprC) << 4) & 0xF0) | (((unsigned char)tmp) & 0x0F);
            byte4 = (unsigned char)(displacement & 0xFF);
          } else // literal moze da se smesti u D bite
          {
            byte1 = 0b00110011;   // instrCode = 0b0011; instrMode = 0b0011;
            
            unsigned int tmp = (num >> 8) & 0b1111;

            byte3 = ((((unsigned char)gprC) << 4) & 0xF0) | (((unsigned char)tmp) & 0x0F);
            byte4 = (unsigned char)(num & 0xFF);
          }

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        } else if (isSymbol(args[2]))
        {
          Symbol* sym = symTable.findSymbolByName(args[2]);
          
          // if (currentSection->getName() == sym->getSymbolSection())
          // {
          //   // Simbol se nalazi u istoj sekciji -> LAKSI SLUCAJ
          //   int val = sym->getSymbolValue();

          //   byte1 = 0b00110011;   // instrCode = 0b0011; instrMode = 0b0011;
            
          //   unsigned int tmp = (val >> 8) & 0b1111;

          //   byte3 = ((((unsigned char)gprC) << 4) & 0xF0) | (((unsigned char)tmp) & 0x0F);
          //   byte4 = (unsigned char)(val & 0xFF);
          // } else
          // {

          // PROMENA: Ne interesuje me da li je simbol u istoj sekciji
          // Simbol se ne nalazi u istoj sekciji -> TEZI SLUCAJ
          vector<pair<int,int>> addrIndex = currentSection->addNewEntryAtTheEndOfLiteralTable(sym); // Ova addr mi treba za relok zapis

          int addr,index;
          for (auto it = addrIndex.begin(); it != addrIndex.end(); ++it)
          {
            addr = it->first;
            index = it->second;
          }

          byte1 = 0b00111011;   // instrCode = 0b0011; instrMode = 0b1011;

          int displacement = addr - currentSection->getLocCounter();

          unsigned int tmp = (displacement >> 8) & 0b1111;

          byte3 = ((((unsigned char)gprC) << 4) & 0xF0) | (((unsigned char)tmp) & 0x0F);
          byte4 = (unsigned char)(displacement & 0xFF);

          RelocSymbol* relSym = new RelocSymbol(sym, addr, index);
          vector<RelocSymbol*> relData = currentSection->getRelocationTable();
          relData.push_back(relSym);
          currentSection->setRelocationData(relData);
          // }

          data.push_back(byte1);
          data.push_back(byte2);
          data.push_back(byte3);
          data.push_back(byte4);

          currentSection->setSectionData(data);
          currentSection->incLocationCounterFor(4);
        }
      }
    }
  }
}

void Assembler::obradaLabele(string s)
{
  if (prviProlaz)
  {
    string substr = s.erase(s.find_first_of(':'));
    Symbol* sym = symTable.findSymbolByName(substr);
    if (sym == nullptr)
    {
      // Nema ga u tabeli simbola, dodajem ga
      Symbol* newSym = new Symbol(substr, currentSection->getLocCounter(), currentSection->getName(), false, false, false);
      symTable.addSymboltoSymbolTable(newSym);
    } else
    {
      if (currentSection->getName().compare("UND") == 0)
      {
        cout << "ERROR: Labela " << s << " definisana je van sekcije!" << endl;
        exit(-1);
      }
      
      if(sym->getSymbolSection().compare("UND") == 0)
        sym->setSymbolSection(currentSection->getName());

      sym->setSymbolValue(currentSection->getLocCounter());
    }
  }
  // U drugom prolazu nije potrebno nista novo da se uradi
  else if (drugiProlaz)
  {
    string substr = s.erase(s.find_first_of(':'));
    Symbol* sym = symTable.findSymbolByName(substr);
    if (sym == nullptr)
    {
      // Nema ga u tabeli simbola, dodajem ga
      Symbol* newSym = new Symbol(substr, currentSection->getLocCounter(), currentSection->getName(), false, false, false);
      symTable.addSymboltoSymbolTable(newSym);
    } else
    {
      if (currentSection->getName().compare("UND") == 0)
      {
        cout << "ERROR: Labela " << s << " definisana je van sekcije!" << endl;
        exit(-1);
      }
      
      if(sym->getSymbolSection().compare("UND") == 0)
        sym->setSymbolSection(currentSection->getName());

      sym->setSymbolValue(currentSection->getLocCounter());
    }
  }
}

void Assembler::obradaLiterala(string s)
{
  int newArg = 0;
  if (isHexNumber(s))
  {
    newArg = (int)stol(s, nullptr, 16);
  } else if (isDecNumber(s))
  {
    newArg = stoi(s);
  }
  
  if (!stajeU12Bita(s))
  {
    // Ako ne staje u 12b, stavljam ga u tabelu literala
    if (!currentSection->entryExistsInLiteralTable(newArg))
    {
      // Dodajem novi ulaz u tabelu literala
      Literal* newLit = new Literal(s, newArg, 4, 0);
      currentSection->getLiteralTable()->push_back(newLit);
    }
  }
}

void Assembler::writeToFile(ostream &os)
{
  os << "SYMBOL_TABLE" << endl;

  // os << std::internal << std::setw(5) << "ID";
  // os << std::internal << std::setw(10) << "Name";
  // os << std::internal << std::setw(10) << "Section";
  // os << std::internal << std::setw(10) << "Bind";
  // os << std::internal << std::setw(10) << "Value";
  // os << std::internal << std::setw(10) << "Type";
  os << endl;

  symTable.printSymbolTable(os);
  os << endl;

  // ------------ SECTION TABLE ---------------

  os << "SECTION_TABLE" << endl;

  // os << std::internal << std::setw(10) << "Name";
  // os << std::internal << std::setw(10) << "Size";
  os << endl;

  secTable.printSectionTable(os);
  os << endl;

  secTable.printLiteralTables(os);
  os << endl;

  secTable.printRelocationTables(os);
  os << endl;

  secTable.printAllSectionData(os);
  
}

void Assembler::obrada(vector<string> fajlZaObradu)
{
  // cout << "prviProlaz: " << prviProlaz << ", " << currentSection->getName() << endl;
  // cout << "drugiProlaz: " << drugiProlaz << ", " << currentSection->getName() << endl;
  for (int i = 0; i < fajlZaObradu.size(); i++) {
    if(!end)
    {
      if(isDirective(fajlZaObradu[i]))            // direktiva
      {
        obradaDirektive(fajlZaObradu[i]);
      } else if (isInstruction(fajlZaObradu[i]))  // instrukcija
      {
        obradaInstrukcije(fajlZaObradu[i]);
      } else if (isLabel(fajlZaObradu[i]))        // labela
      {
        obradaLabele(fajlZaObradu[i]);
      }
    }
  }
}

Assembler::~Assembler() {}