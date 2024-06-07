#include "../inc/emulator.h"

Emulator::Emulator(string fileName) : fileName(fileName)
{
  end = false;

  for (int i = 0; i < 15; i++)
    gpr[i] = 0;

  string pcStart = "0x40000000";
  
  gpr[15] = stol(pcStart, nullptr, 16);

  // cout << hex << gpr[15] << endl;

  for (int i = 0; i < 3; i++)
    csr[i] = 0;
}

Emulator::~Emulator() {}

vector<pair<unsigned int, unsigned char>> Emulator::parseInFile()
{
  ParserEM* parser = new ParserEM(fileName);
  vector<pair<unsigned int, unsigned char>> data = parser->parse();
  // cout << "PARSING FILE " << fileName << " DONE" << endl;

  return data;
}

void Emulator::fillMemory()
{
  memory = parseInFile();

  // for (auto it = memory.begin(); it != memory.end(); ++it)
  // {
  //   cout << hex << it->first << " : " << hex << (int)it->second << endl;
  // }
}

pair<unsigned int, unsigned char> Emulator::searchMemoryForAddress(unsigned int addr)
{
  pair<unsigned int, unsigned char> ret;

  for (auto it = memory.begin(); it != memory.end(); ++it)
  {
    if (it->first == addr)
    {
      ret = make_pair(it->first, it->second);
      break;
    }
  }

  return ret;
}

void Emulator::writeByteInMemory(unsigned int addr, unsigned char data)
{
  // cout << "ovde sam, proslednjena adresa je " << addr << ", prosledjen data je " << hex << (int)data << endl;
  for (auto it = memory.begin(); it != memory.end(); ++it)
  {
    // cout << "it->first=" << it->first << endl;
    if (it->first == addr)
    {
      it->second = data;
      // cout << "ovde sam, it->first = " << it->first << ", it->second posle upisa = " << it->second << endl;
      break;
    }
  }
  // cout << "adresa " << addr << " ne postoji u mapi" << endl;
  // Ako je dosao do ovde, to znaci da adresa sigurno ne postoji u mapi i to je neka nova adresa vezana za sp
  memory.emplace_back(std::make_pair(addr, data));
}

void Emulator::work()
{
  while (!end)
  {
    executeInstruction();

    // PROVERA: Ispis stanja posle svake instrukcije
    // for (int i = 0; i < 16; i++)
    // {
    //   if (i != 0 && i % 4 == 0) std::cout<<endl;
    //   std::cout << "r" << i << "=" << std::hex << "0x" << std::setw(8) << gpr[i] << std::setw(2) << " ";
    // }
    // std::cout << std::endl;
    // for (int i = 0; i < 3; i++)
    // {
    //   std::cout << std::setw(10) << "csr" << i << "=" << std::hex << "0x" << csr[i] <<std::dec;
    // }
  }
}

void Emulator::executeInstruction()
{
  unsigned char instrCode = searchMemoryForAddress(gpr[15]).second;

  // cout << "addr: " << hex << searchMemoryForAddress(gpr[15]).first << ", ";
  // cout << "instrCode: " << hex << (int)instrCode << endl;
  // end = true;

  if (instrCode == 0x00)
  {
    // halt - radi
    // cout << "HALT!!!!" << endl;
    end = true;
    gpr[15] += 4;
  }

  else if (instrCode == 0x10)
  {
    // int - instrukcija softverskog prekida
    // push status; push pc; cause <= 4; status <= status & (~0x1); pc <= handle;
    
    // Stek raste ka nizim adresama i ukazuje na poslednju zauzetu lokaciju

    // push pc
    gpr[15] += 4;
    // gpr[14] = gpr[14] - 4;

    unsigned char byte1PC = (unsigned char)(gpr[15] & 0xFF);
    unsigned char byte2PC = (unsigned char)((gpr[15] >> 8) & 0xFF);
    unsigned char byte3PC = (unsigned char)((gpr[15] >> 16) & 0xFF);
    unsigned char byte4PC = (unsigned char)((gpr[15] >> 24) & 0xFF);

    writeByteInMemory(gpr[14]-1, byte1PC);
    writeByteInMemory(gpr[14]-2, byte2PC);
    writeByteInMemory(gpr[14]-3, byte3PC);
    writeByteInMemory(gpr[14]-4, byte4PC);

    gpr[14] = gpr[14] - 4;

    // push status
    // gpr[14] = gpr[14] - 4;

    unsigned char byte1status = (unsigned char)(csr[0] & 0xFF);
    unsigned char byte2status = (unsigned char)((csr[0] >> 8) & 0xFF);
    unsigned char byte3status = (unsigned char)((csr[0] >> 16) & 0xFF);
    unsigned char byte4status = (unsigned char)((csr[0] >> 24) & 0xFF);

    writeByteInMemory(gpr[14]-1, byte1status);
    writeByteInMemory(gpr[14]-2, byte2status);
    writeByteInMemory(gpr[14]-3, byte3status);
    writeByteInMemory(gpr[14]-4, byte4status);

    gpr[14] = gpr[14] - 4;

    // cause <= 4;
    csr[2] = 4;

    // status <= status & (~0x1);
    csr[0] = csr[0] & (~0x1);

    // pc <= handle
    gpr[15] = csr[1];   
  }

  else if (instrCode == 0x20)
  {
    // call operand
    
    // push pc; pc <= gpr[A] + gpr[B] + D;

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;  // treba da ispadne PC
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int DizC = (searchMemoryForAddress(gpr[15]+2).second) & 0x0F;
    int DizD = (searchMemoryForAddress(gpr[15]+3).second);
    
    int D = ((DizC << 12) & 0xF00) | (DizD & 0x0FF);

    // push pc
    gpr[15] += 4;
    // gpr[14] = gpr[14] - 4;

    unsigned char byte1PC = (unsigned char)(gpr[15] & 0xFF);
    unsigned char byte2PC = (unsigned char)((gpr[15] >> 8) & 0xFF);
    unsigned char byte3PC = (unsigned char)((gpr[15] >> 16) & 0xFF);
    unsigned char byte4PC = (unsigned char)((gpr[15] >> 24) & 0xFF);

    writeByteInMemory(gpr[14]-1, byte1PC);
    writeByteInMemory(gpr[14]-2, byte2PC);
    writeByteInMemory(gpr[14]-3, byte3PC);
    writeByteInMemory(gpr[14]-4, byte4PC);

    gpr[14] = gpr[14] - 4;

    // pc <= gpr[A] + gpr[B] + D;
    gpr[15] = gpr[gprA] + gpr[gprB] + D;
  }

  else if (instrCode == 0x21)
  {
    // call operand

    // push pc; pc <= mem32[gpr[A] + gpr[B] + D]

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;  // treba da ispadne PC
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int DizC = (searchMemoryForAddress(gpr[15]+2).second) & 0x0F;
    int DizD = (searchMemoryForAddress(gpr[15]+3).second);
    
    int D = ((DizC << 12) & 0xF00) | (DizD & 0x0FF);

    // cout << "A=" << gprA << ", B=" << gprB << ", DizC=" << DizC << ", DizD=" << DizD << endl;

    // cout << "D="<<D<<", gpr[gprA]="<<gpr[gprA]<<endl;

    // push pc
    gpr[15] += 4;
    // gpr[14] = gpr[14] - 4;

    unsigned char byte1PC = (unsigned char)(gpr[15] & 0xFF);
    unsigned char byte2PC = (unsigned char)((gpr[15] >> 8) & 0xFF);
    unsigned char byte3PC = (unsigned char)((gpr[15] >> 16) & 0xFF);
    unsigned char byte4PC = (unsigned char)((gpr[15] >> 24) & 0xFF);

    unsigned int pcSP = ((byte4PC << 24) & 0xFF000000) | ((byte3PC << 16) & 0x00FF0000) | ((byte2PC << 8) & 0x0000FF00) | (byte1PC & 0x000000FF);

    // cout << "pcSP=" << pcSP<<endl;

    writeByteInMemory(gpr[14]-1, byte1PC);
    writeByteInMemory(gpr[14]-2, byte2PC);
    writeByteInMemory(gpr[14]-3, byte3PC);
    writeByteInMemory(gpr[14]-4, byte4PC);

    gpr[14] = gpr[14] - 4;

    // pc <= mem32[gpr[A] + gpr[B] + D]
    int addr = gpr[gprA] + gpr[gprB] + D;
    // cout << "addr=" << addr << endl;

    unsigned char byte1 = searchMemoryForAddress(addr).second;
    unsigned char byte2 = searchMemoryForAddress(addr+1).second;
    unsigned char byte3 = searchMemoryForAddress(addr+2).second;
    unsigned char byte4 = searchMemoryForAddress(addr+3).second;

    unsigned int pcData = ((byte4 << 24) & 0xFF000000) | ((byte3 << 16) & 0x00FF0000) | ((byte2 << 8) & 0x0000FF00) | (byte1 & 0x000000FF);

    // cout << "pcData="<<pcData<<endl;

    // cout << "gprB=" << gpr[gprB] << endl;

    gpr[15] = pcData;

    // cout << "pc=" << gpr[15]<<endl;
  }

  else if (instrCode == 0x30)
  {
    // jmp operand

    // pc <= gpr[A] + D

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;  // treba da ispadne PC
    int DizC = (searchMemoryForAddress(gpr[15]+2).second) & 0x0F;
    int DizD = (searchMemoryForAddress(gpr[15]+3).second);
    
    int D = ((DizC << 12) & 0xF00) | (DizD & 0x0FF);

    gpr[15] = gpr[gprA] + D;
  }
  
  else if (instrCode == 0x38)
  {
    // jmp operand

    // pc <= mem32[gpr[A] + D]

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;  // treba da ispadne PC
    int DizC = (searchMemoryForAddress(gpr[15]+2).second) & 0x0F;
    int DizD = (searchMemoryForAddress(gpr[15]+3).second);
    
    int D = ((DizC << 12) & 0xF00) | (DizD & 0x0FF);

    // cout << "DizC=" << DizC << ", DizD=" << DizD << endl;

    // cout << "D="<<D<<", gpr[gprA]="<<gpr[gprA]<<endl;

    int addr = gpr[gprA] + D;

    // cout << "addr= " << addr << endl;

    unsigned char byte1 = searchMemoryForAddress(addr).second;
    unsigned char byte2 = searchMemoryForAddress(addr+1).second;
    unsigned char byte3 = searchMemoryForAddress(addr+2).second;
    unsigned char byte4 = searchMemoryForAddress(addr+3).second;

    unsigned int pcData = ((byte4 << 24) & 0xFF000000) | ((byte3 << 16) & 0x00FF0000) | ((byte2 << 8) & 0x0000FF00) | (byte1 & 0x000000FF);

    // cout << "pcData=" << pcData << endl;
    gpr[15] = pcData;
  }

  else if (instrCode == 0x31)
  {
    // beq %gpr1, %gpr2, operand

    // if (gpr[B] == gpr[C]) pc <= gpr[A] + D

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;  // trebalo bi da bude PC
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;

    int DizC = (searchMemoryForAddress(gpr[15]+2).second) & 0x0F;
    int DizD = (searchMemoryForAddress(gpr[15]+3).second);
    
    int D = ((DizC << 12) & 0xF00) | (DizD & 0x0FF);

    if (gpr[gprB] == gpr[gprC])
    {
      gpr[15] = gpr[gprA] + D;
    } else
    {
      gpr[15] += 4;
    }
  }

  else if (instrCode == 0x39)
  {
    // beq %gpr1, %gpr2, operand

    // if (gpr[B] == gpr[C]) pc <= mem32[gpr[A] + D]

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;  // trebalo bi da bude PC
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;

    int DizC = (searchMemoryForAddress(gpr[15]+2).second) & 0x0F;
    int DizD = (searchMemoryForAddress(gpr[15]+3).second);
    
    int D = ((DizC << 12) & 0xF00) | (DizD & 0x0FF);

    // cout << "DizC=" << DizC << ", DizD=" << DizD << endl;

    // cout << "D="<<D<<", gpr[gprA]="<<gpr[gprA]<<endl;

    int addr = gpr[gprA] + D;

    unsigned char byte1 = searchMemoryForAddress(addr).second;
    unsigned char byte2 = searchMemoryForAddress(addr+1).second;
    unsigned char byte3 = searchMemoryForAddress(addr+2).second;
    unsigned char byte4 = searchMemoryForAddress(addr+3).second;

    unsigned int pcData = ((byte4 << 24) & 0xFF000000) | ((byte3 << 16) & 0x00FF0000) | ((byte2 << 8) & 0x0000FF00) | (byte1 & 0x000000FF);

    // cout << "pcData="<<pcData<<endl;

    // cout << "gprB=" << gpr[gprB] << ", gprC="<<gpr[gprC]<<endl;

    if (gpr[gprB] == gpr[gprC])
    {
      gpr[15] = pcData;
    } else
    {
      gpr[15] += 4;
    }

    // cout << "pc = " << gpr[15] << endl;
  }

  else if (instrCode == 0x32)
  {
    // bne %gpr1, %gpr2, operand

    // if (gpr[B] != gpr[C]) pc <= gpr[A] + D

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;  // trebalo bi da bude PC
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;

    int DizC = (searchMemoryForAddress(gpr[15]+2).second) & 0x0F;
    int DizD = (searchMemoryForAddress(gpr[15]+3).second);
    
    int D = ((DizC << 12) & 0xF00) | (DizD & 0x0FF);

    if (gpr[gprB] != gpr[gprC])
    {
      gpr[15] = gpr[gprA] + D;
    } else
    {
      gpr[15] += 4;
    }
  }

  else if (instrCode == 0x3A)
  {
    // bne %gpr1, %gpr2, operand

    // if (gpr[B] != gpr[C]) pc <= mem32[gpr[A] + D]

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;  // trebalo bi da bude PC
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;

    int DizC = (searchMemoryForAddress(gpr[15]+2).second) & 0x0F;
    int DizD = (searchMemoryForAddress(gpr[15]+3).second);
    
    int D = ((DizC << 12) & 0xF00) | (DizD & 0x0FF);

    int addr = gpr[gprA] + D;

    unsigned char byte1 = searchMemoryForAddress(addr).second;
    unsigned char byte2 = searchMemoryForAddress(addr+1).second;
    unsigned char byte3 = searchMemoryForAddress(addr+2).second;
    unsigned char byte4 = searchMemoryForAddress(addr+3).second;

    unsigned int pcData = ((byte4 << 24) & 0xFF000000) | ((byte3 << 16) & 0x00FF0000) | ((byte2 << 8) & 0x0000FF00) | (byte1 & 0x000000FF);

    if (gpr[gprB] != gpr[gprC])
    {
      gpr[15] = pcData;
    } else
    {
      gpr[15] += 4;
    }
  }

  else if (instrCode == 0x33)
  {
    // bgt %gpr1, %gpr2, operand

    // if (gpr[B] signed > gpr[C]) pc <= gpr[A] + D

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;  // trebalo bi da bude PC
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;

    int DizC = (searchMemoryForAddress(gpr[15]+2).second) & 0x0F;
    int DizD = (searchMemoryForAddress(gpr[15]+3).second);
    
    int D = ((DizC << 12) & 0xF00) | (DizD & 0x0FF);

    if ((signed int)gpr[gprB] > gpr[gprC])    // TODO - MOZE LI OVAKO SA SIGNED?????
    {
      gpr[15] = gpr[gprA] + D;
    } else
    {
      gpr[15] += 4;
    }
  }

  else if (instrCode == 0x3B)
  {
    // bgt %gpr1, %gpr2, operand

    // if (gpr[B] signed > gpr[C]) pc <= mem32[gpr[A] + D]

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;  // trebalo bi da bude PC
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;

    int DizC = (searchMemoryForAddress(gpr[15]+2).second) & 0x0F;
    int DizD = (searchMemoryForAddress(gpr[15]+3).second);
    
    int D = ((DizC << 12) & 0xF00) | (DizD & 0x0FF);

    int addr = gpr[gprA] + D;

    unsigned char byte1 = searchMemoryForAddress(addr).second;
    unsigned char byte2 = searchMemoryForAddress(addr+1).second;
    unsigned char byte3 = searchMemoryForAddress(addr+2).second;
    unsigned char byte4 = searchMemoryForAddress(addr+3).second;

    unsigned int pcData = ((byte4 << 24) & 0xFF000000) | ((byte3 << 16) & 0x00FF0000) | ((byte2 << 8) & 0x0000FF00) | (byte1 & 0x000000FF);

    if ((signed int)gpr[gprB] > gpr[gprC])    // TODO - MOZE LI OVAKO SA SIGNED?????
    {
      gpr[15] = pcData;
    } else
    {
      gpr[15] += 4;
    }
  }

  else if (instrCode == 0x40)
  {
    // xchg %gprS, %gprD

    // gprD = C, gprS = B

    int gprS = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprD = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;

    unsigned int gprTmp = gpr[gprD];
    gpr[gprD] = gpr[gprS];
    gpr[gprS] = gprTmp;

    gpr[15] += 4;
  }

  else if (instrCode == 0x50)
  {
    // add %gprS, %gprD

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;

    gpr[gprA] = gpr[gprB] + gpr[gprC];

    gpr[15] += 4;
  }
  
  else if (instrCode == 0x51)
  {
    // sub %gprS, %gprD

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;

    gpr[gprA] = gpr[gprB] - gpr[gprC];

    gpr[15] += 4;
  }

  else if (instrCode == 0x52)
  {
    // mul %gprS, %gprD

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;

    gpr[gprA] = gpr[gprB] * gpr[gprC];

    gpr[15] += 4;
  }

  else if (instrCode == 0x53)
  {
    // div %gprS, %gprD

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;

    if (gpr[gprC] != 0)
    {
      gpr[gprA] = gpr[gprB] / gpr[gprC];
    } else
    {
      cout << "ERROR: Deljenje sa nulom!" << endl;
      exit(-1);
    }    

    gpr[15] += 4;
  }

  else if (instrCode == 0x60)
  {
    // not %gpr

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;

    gpr[gprA] = ~gpr[gprB];

    gpr[15] += 4;
  }

  else if (instrCode == 0x61)
  {
    // and %gprS, %gprD

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;

    gpr[gprA] = gpr[gprB] & gpr[gprC];

    gpr[15] += 4;
  }

  else if (instrCode == 0x62)
  {
    // or %gprS, %gprD

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;

    gpr[gprA] = gpr[gprB] | gpr[gprC];

    gpr[15] += 4;
  }

  else if (instrCode == 0x63)
  {
    // xor %gprS, %gprD

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;

    gpr[gprA] = gpr[gprB] ^ gpr[gprC];

    gpr[15] += 4;
  }

  else if (instrCode == 0x70)
  {
    // shl %gprS, %gprD

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;

    gpr[gprA] = gpr[gprB] << gpr[gprC];

    gpr[15] += 4;
  }

  else if (instrCode == 0x71)
  {
    // shr %gprS, %gprD

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;

    gpr[gprA] = gpr[gprB] >> gpr[gprC];

    gpr[15] += 4;
  }

  else if (instrCode == 0x80)
  {
    // st %gpr, operand

    // mem32[gpr[A] + gpr[B] + D] <= gpr[C]

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;
    int DizC = (searchMemoryForAddress(gpr[15]+2).second) & 0x0F;
    int DizD = (searchMemoryForAddress(gpr[15]+3).second);
    
    int D = ((DizC << 12) & 0xF00) | (DizD & 0x0FF);

    int addr = gpr[gprA] + gpr[gprB] + D;

    unsigned char byte1 = (unsigned char)(gpr[gprC] & 0xFF);
    unsigned char byte2 = (unsigned char)((gpr[gprC] >> 8) & 0xFF);
    unsigned char byte3 = (unsigned char)((gpr[gprC] >> 16) & 0xFF);
    unsigned char byte4 = (unsigned char)((gpr[gprC] >> 24) & 0xFF);

    writeByteInMemory(addr, byte1);
    writeByteInMemory(addr+1, byte2);
    writeByteInMemory(addr+2, byte3);
    writeByteInMemory(addr+3, byte4);

    gpr[15] += 4;
  }

  else if (instrCode == 0x81)
  {
    // st %gpr, operand

    // gpr[A] <= gpr[A] - D; mem32[gpr[A]] <= gpr[C];

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int DizC = (searchMemoryForAddress(gpr[15]+2).second) & 0x0F;
    int DizD = (searchMemoryForAddress(gpr[15]+3).second);
    
    int D = ((DizC << 12) & 0xF00) | (DizD & 0x0FF);

    // cout << "gprA=" << gprA << ", gprB=" << gprB << ", D=" << D << endl;

    // gpr[A] <= gpr[A] - D;
    gpr[gprA] = gpr[gprA] - D;

    // mem32[gpr[A]] <= gpr[C];
    unsigned char byte1gprC = (unsigned char)(gpr[gprB] & 0xFF);
    unsigned char byte2gprC = (unsigned char)((gpr[gprB] >> 8) & 0xFF);
    unsigned char byte3gprC = (unsigned char)((gpr[gprB] >> 16) & 0xFF);
    unsigned char byte4gprC = (unsigned char)((gpr[gprB] >> 24) & 0xFF);

    // cout << hex << (int)byte1gprC << ", " << hex << (int)byte2gprC << ", " << hex << (int)byte3gprC << ", " << hex << (int)byte4gprC << endl;

    // cout << "gpr[gprA] " << gpr[gprA] << endl;
    // cout << "gpr[gprB] " << gpr[gprB] << " "<< endl;    

    writeByteInMemory(gpr[gprA], byte1gprC);
    writeByteInMemory(gpr[gprA]+1, byte2gprC);
    writeByteInMemory(gpr[gprA]+2, byte3gprC);
    writeByteInMemory(gpr[gprA]+3, byte4gprC);

    // auto i = searchMemoryForAddress(gpr[gprA]);
    // auto j = searchMemoryForAddress(gpr[gprA]+1);
    // auto k = searchMemoryForAddress(gpr[gprA]+2);
    // auto l = searchMemoryForAddress(gpr[gprA]+3);

    // cout << i.first << ":" << hex << (int)i.second << endl;
    // cout << j.first << ":" << hex << (int)j.second << endl;
    // cout << k.first << ":" << hex << (int)k.second << endl;
    // cout << l.first << ":" << hex << (int)l.second << endl;

    gpr[15] += 4;
  }

  else if (instrCode == 0x82)
  {
    // st %gpr, operand

    // mem32[mem32[gpr[A] + gpr[B] + D]] <= gprC;

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;
    int DizC = (searchMemoryForAddress(gpr[15]+2).second) & 0x0F;
    int DizD = (searchMemoryForAddress(gpr[15]+3).second);
    
    int D = ((DizC << 12) & 0xF00) | (DizD & 0x0FF);

    int addrUnutrasnja = gpr[gprA] + gpr[gprB] + D;

    unsigned char byte1Spoljasnja = searchMemoryForAddress(addrUnutrasnja).second;
    unsigned char byte2Spoljasnja = searchMemoryForAddress(addrUnutrasnja+1).second;
    unsigned char byte3Spoljasnja = searchMemoryForAddress(addrUnutrasnja+2).second;
    unsigned char byte4Spoljasnja = searchMemoryForAddress(addrUnutrasnja+3).second;

    int addrSpoljasnja = ((byte4Spoljasnja << 24) & 0xFF000000) | ((byte3Spoljasnja << 16) & 0x00FF0000) | ((byte2Spoljasnja << 8) & 0x0000FF00) | (byte1Spoljasnja & 0x000000FF);

    unsigned char byte1gprC = (unsigned char)(gpr[gprC] & 0xFF);
    unsigned char byte2gprC = (unsigned char)((gpr[gprC] >> 8) & 0xFF);
    unsigned char byte3gprC = (unsigned char)((gpr[gprC] >> 16) & 0xFF);
    unsigned char byte4gprC = (unsigned char)((gpr[gprC] >> 24) & 0xFF);

    writeByteInMemory(addrSpoljasnja, byte1gprC);
    writeByteInMemory(addrSpoljasnja+1, byte2gprC);
    writeByteInMemory(addrSpoljasnja+2, byte3gprC);
    writeByteInMemory(addrSpoljasnja+3, byte4gprC);

    gpr[15] += 4;
  }

  else if (instrCode == 0x90)
  {
    // ld operand, %gpr
    // csrrd %csr, %gpr

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int csrB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;

    gpr[gprA] = csr[csrB];

    gpr[15] += 4;
  }

  else if (instrCode == 0x91)
  {
    // ld operand, %gpr

    // gpr[A] <= gpr[B] + D;

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;  // treba da ispadne PC
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int DizC = (searchMemoryForAddress(gpr[15]+2).second) & 0x0F;
    int DizD = (searchMemoryForAddress(gpr[15]+3).second);
    
    int D = ((DizC << 12) & 0xF00) | (DizD & 0x0FF);

    // cout << "gprA = " << gprA << ", gprB = " << gprB << ", DizC = " << hex << DizC << ", DizD = " << hex << DizD << ", D = " << hex << D << endl;

    gpr[gprA] = gpr[gprB] + D;

    gpr[15] += 4;
  }

  else if (instrCode == 0x92)
  {
    // ld operand, %gpr
    // cout << "evo me!" << endl;

    // gpr[A] <= mem32[gpr[B] + gpr[C] + D];

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int gprC = (searchMemoryForAddress(gpr[15]+2).second >> 4) & 0x0F;
    int DizC = (searchMemoryForAddress(gpr[15]+2).second) & 0x0F;
    int DizD = (searchMemoryForAddress(gpr[15]+3).second);

    int D = ((DizC << 12) & 0xF00) | (DizD & 0x0FF);

    unsigned int addr = gpr[gprB] + gpr[gprC] + D;

    // cout << "D: " << D << ", addr: " << addr << endl;
    
    unsigned char byte4 = searchMemoryForAddress(addr).second;
    unsigned char byte3 = searchMemoryForAddress(addr+1).second;
    unsigned char byte2 = searchMemoryForAddress(addr+2).second;
    unsigned char byte1 = searchMemoryForAddress(addr+3).second;

    // OVAKO BI TREBALO ZBOG LITTLE ENDIAN
    // gpr[gprA] = ((byte4 << 24) & 0xFF000000) | ((byte3 << 16) & 0x00FF0000) | ((byte2 << 8) & 0x0000FF00) | (byte1 & 0x000000FF);
    
    gpr[gprA] = ((byte1 << 24) & 0xFF000000) | ((byte2 << 16) & 0x00FF0000) | ((byte3 << 8) & 0x0000FF00) | (byte4 & 0x000000FF);

    gpr[15] += 4;
  }

  else if (instrCode == 0x93)
  {
    // ld operand, %gpr

    // gpr[A] <= mem32[gpr[B]]; gpr[B] <= gpr[B] + D;

    int gprA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;
    int DizC = (searchMemoryForAddress(gpr[15]+2).second) & 0x0F;
    int DizD = (searchMemoryForAddress(gpr[15]+3).second);
    
    int D = ((DizC << 12) & 0xF00) | (DizD & 0x0FF);

    unsigned char byte4 = (unsigned char)searchMemoryForAddress(gpr[gprB]).second;
    unsigned char byte3 = (unsigned char)searchMemoryForAddress(gpr[gprB]+1).second;
    unsigned char byte2 = (unsigned char)searchMemoryForAddress(gpr[gprB]+2).second;
    unsigned char byte1 = (unsigned char)searchMemoryForAddress(gpr[gprB]+3).second;

    // cout << hex << (int)byte1 << ", " << hex << (int)byte2 << ", " << hex << (int)byte3 << ", " << hex << (int)byte4 << endl;

    gpr[gprA] = ((byte4 << 24) & 0xFF000000) | ((byte3 << 16) & 0x00FF0000) | ((byte2 << 8) & 0x0000FF00) | (byte1 & 0x000000FF);
    
    // gpr[gprA] = ((byte1 << 24) & 0xFF000000) | ((byte2 << 16) & 0x00FF0000) | ((byte3 << 8) & 0x0000FF00) | (byte4 & 0x000000FF);

    // cout << "gpr[gprA] " << gpr[gprA] << endl;
    // cout << "gpr[gprB] " << gpr[gprB] << " " << gpr[gprB]+1<< endl;

    // cout << "gprA=" << gprA << ", gprB=" << gprB << ", D=" << D << endl;

    gpr[gprB] = gpr[gprB] + D;

    // cout << "gpr[gprA]: " << gpr[gprA] << endl;

    if (gprA != 15)
      gpr[15] += 4;
  }

  else if (instrCode == 0x94)
  {
    // ld operand, %gpr
    // csrwr %gpr, %csr

    int csrA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;

    csr[csrA] = gpr[gprB];

    gpr[15] += 4;
  }

  // else if (instrCode == 0x95)
  // {
  //   // ld operand, %gpr

  //   // trebalo bi da se ne koristi
  //   cout << "ups! koristi se " << instrCode << endl;
  // }

  // else if (instrCode == 0x96)
  // {
  //   // ld operand, %gpr

  //   // trebalo bi da se ne koristi
  //   cout << "ups! koristi se " << instrCode << endl;
  // }

  else if (instrCode == 0x97)
  {
    // ld operand, %gpr

    // csr[A] <= mem32[gpr[B]]; gpr[B] = gpr[B] + D;

    int csrA = (searchMemoryForAddress(gpr[15]+1).second >> 4) & 0x0F;
    int gprB = (searchMemoryForAddress(gpr[15]+1).second) & 0x0F;   // treba da bude sp
    int DizC = (searchMemoryForAddress(gpr[15]+2).second) & 0x0F;
    int DizD = (searchMemoryForAddress(gpr[15]+3).second);
    
    int D = ((DizC << 12) & 0xF00) | (DizD & 0x0FF);

    unsigned char byte1 = searchMemoryForAddress(gpr[gprB]).second;
    unsigned char byte2 = searchMemoryForAddress(gpr[gprB]+1).second;
    unsigned char byte3 = searchMemoryForAddress(gpr[gprB]+2).second;
    unsigned char byte4 = searchMemoryForAddress(gpr[gprB]+3).second;

    csr[csrA] = ((byte4 << 24) & 0xFF000000) | ((byte3 << 16) & 0x00FF0000) | ((byte2 << 8) & 0x0000FF00) | (byte1 & 0x000000FF);
    gpr[gprB] = gpr[gprB] + D;

    gpr[15] += 4;
  } else
  {
    cout << "Nepoznat instrCode = " << hex << (int)instrCode << " na adresi " << gpr[15] << endl;
    
    end = true;
  }

  // cout << hex << (int)instrCode << endl;

  // Samo za potrebe testiranja ovde stavljam end na true - OBRISI POSLE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // end = true;
}

void Emulator::write()
{
  std::cout << "Emulated processor executed halt instruction" <<std::endl;
  std::cout << "Emulated processor state:" << endl;

  for (int i = 0; i < 16; i++)
  {
    if (i != 0 && i % 4 == 0) std::cout<<endl;
    std::cout << "r" << dec << i << "=" << "0x" << std::setw(8) << std::setfill('0') << hex << gpr[i] << std::setw(1) << "\t";
  }
  std::cout << std::endl;
  // for (int i = 0; i < 3; i++)
  // {
  //   std::cout << std::setw(10) << "csr" << i << "=" << std::hex << "0x" << csr[i] <<std::dec;
  // }
}
