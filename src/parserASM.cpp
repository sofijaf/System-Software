#include "../inc/parserASM.h"

#include <iostream>
#include <string>
#include <cctype>

// fstream file;
// std::regex blankLine = std::regex("^\\s*$");

// void openFile(string path)
// {
//   file.open(path);
//   if (!file.is_open())
//   {
//     cout << "Fajl nije pronadjen!" << endl;
//   } else
//   {
//     cout << "Fajl uspesno otvoren!" << endl;
//   }
// }

regex blankLine = std::regex("^\\s*$");

vector<string> prepareTextForAssembling(ifstream& file)
{
  vector<string> fajlZaObradu;
  int position = 0;
  std::string line;
  while (std::getline(file, line)) {
    // Brisanje komentara
    while ((position = line.find("#")) != string::npos) line.erase(position);
    // Brisanje praznih linija
    if(regex_match(line, blankLine)) continue;

    // brisanje tabova iz stringa
    line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());

    // brisanje space-ova sa pocetka stringa
    while(line.find_first_of(' ') == 0)
      line.erase(0,1);

    line = removeWhitespacesAfterString(line);
    
		fajlZaObradu.push_back(line);
	}
  return fajlZaObradu;
}

vector<string> Instructions = { "halt",
                                "int",
                                "iret",
                                "call",
                                "ret",
                                "jmp",
                                "beq",
                                "bne",
                                "bgt",
                                "push",
                                "pop",
                                "xchg",
                                "add",
                                "sub",
                                "mul",
                                "div",
                                "not",
                                "and",
                                "or",
                                "xor",
                                "shl",
                                "shr",
                                "ld",
                                "st",
                                "csrrd",
                                "csrwr"};

// Function that returns true if a character is a whitespace.
bool isWhitespace(unsigned char c) {
    if (c == ' ' || c == '\t' || c == '\n' ||
        c == '\r' || c == '\f' || c == '\v') {
        return true;
    } else {
        return false;
    }
}

string removeWhitespacesAfterString(const string& str) {
    std::string s = str;
    int i = s.size() - 1;

    // Starting from the end of the string, remove whitespace characters
    while (i >= 0 && std::isspace(s[i])) {
        s.pop_back();
        i--;
    }

    return s;
}

string deleteWhitespaceFromString(string s)
{
  s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());

  return s;
}

vector<string> getArgsFromString(string s)
{
  vector<string> args = vector<string>();

  string new_string = deleteWhitespaceFromString(s);
  std::stringstream ss(new_string);

  while( ss.good() )
  {
    string substr;
    getline( ss, substr, ',' );
    args.push_back( substr );
  }

  return args;
}

string getOneArgFromString(string s)
{
  std::stringstream ss(s);
  string ret = "";

  while( ss.good() )
  {
    string substr;
    getline( ss, substr, ' ' );
    ret = substr;
    // args.push_back( substr );
  }

  return ret;
}

bool stajeU12Bita(string s)
{
  bool ret = false;
  unsigned int newArg = 0;
  if (isHexNumber(s))
  {
    newArg = (int)stoul(s, nullptr, 16);

    int bitCount = 0;

    // Count the number of bits
    while (newArg != 0) {
      newArg >>= 1;
      bitCount++;
    }

    if (bitCount <= 12)
      ret = true;
  } else if (isDecNumber(s))
  {
    newArg = stoi(s);

    int maxValue = 2047;  // Maximum value representable in 12 bits (signed)  -> 2^11 - 1

    if (newArg <= maxValue) ret = true;
  }

  return ret;
}

bool isInstruction(string s)
{
  bool ret = false;
  for (size_t i = 0; i < Instructions.size(); i++)
  {
    string str = Instructions[i];   // ovde je stajalo Instructions[i] + " "; i to je pravilo problem za instrukcije bez operanada
    if (s.rfind(str, 0) == 0)
    {
      ret = true;
      break;
    }
  }
  return ret;
}

bool isNoOpInstruction(string s)
{
  if (isInstructionHalt(s) || 
      isInstructionInt(s) ||
      isInstructionRet(s) ||
      isInstructionIret(s)) return true;
  else return false;
}

bool isOneOpInstruction(string s)
{
  if (isInstructionCall(s) || 
      isInstructionJmp(s) ||
      isInstructionPush(s) ||
      isInstructionPop(s) ||
      isInstructionNot(s)) 
      {
        return true;
      } else return false;
}

bool isTwoOpInstruction(string s)
{
  bool ret = false;
  if (isInstructionXchg(s)) ret = true;
  else if (isInstructionAdd(s)) ret = true;
  else if (isInstructionSub(s)) ret = true;
  else if (isInstructionMul(s)) ret = true;
  else if (isInstructionDiv(s)) ret = true;
  else if (isInstructionAnd(s)) ret = true;
  else if (isInstructionOr(s)) ret = true;
  else if (isInstructionXor(s)) ret = true;
  else if (isInstructionShl(s)) ret = true;
  else if (isInstructionShr(s)) ret = true;
  else if (isInstructionLd(s)) ret = true;
  else if (isInstructionSt(s)) ret = true;
  else if (isInstructionCsrrd(s)) ret = true;
  else if (isInstructionCsrwr(s)) ret = true;
  else ret = false;
  
  return ret;
}

bool isThreeOpInstruction(string s)
{
  if (isInstructionBeq(s) ||
      isInstructionBne(s) ||
      isInstructionBgt(s)) 
      {
        return true;
      } else return false;
}

int resolveGeneralPurposeRegister(string s)
{
  int ret = -1;
  if (s.compare("r0") == 0)
    ret = 0;
  if (s.compare("r1") == 0)
    ret = 1;
  if (s.compare("r2") == 0)
    ret = 2;
  if (s.compare("r3") == 0)
    ret = 3;
  if (s.compare("r4") == 0)
    ret = 4;
  if (s.compare("r5") == 0)
    ret = 5;
  if (s.compare("r6") == 0)
    ret = 6;
  if (s.compare("r7") == 0)
    ret = 7;
  if (s.compare("r8") == 0)
    ret = 8;
  if (s.compare("r9") == 0)
    ret = 9;
  if (s.compare("r10") == 0)
    ret = 10;
  if (s.compare("r11") == 0)
    ret = 11;
  if (s.compare("r12") == 0)
    ret = 12;
  if (s.compare("r13") == 0)
    ret = 13;
  if (s.compare("r14") == 0)
    ret = 14;
  if (s.compare("r15") == 0)
    ret = 15;
  if (s.compare("pc") == 0)
    ret = 15;
  if (s.compare("sp") == 0)
    ret = 14;
  return ret;
}

int resolveControlAndStatusRegister(string s)
{
  int ret = -1;
  if (s.compare("status") == 0)
    ret = 0;
  if (s.compare("handler") == 0)
    ret = 1;
  if (s.compare("cause") == 0)
    ret = 2;

  return ret;
}

int ldAndStOperand(string s)
{
  int ret = -1;
  // ret = 1 -> $<literal> - vrednost <literal>
  // ret = 2 -> $<simbol> - vrednost <simbol>
  // ret = 3 -> <literal> - vrednost iz memorije na adresi <literal>
  // ret = 4 -> <simbol> - vrednost iz memorije na adresi <simbol>
  // ret = 5 -> %<reg> - vrednost u registru <reg>
  // ret = 6 -> [%<reg>] - vrednost iz memorije na adresi <reg>
  // ret = 7 -> [%<reg> + <literal>] - vrednost iz memorije na adresi <reg> + <literal>
  // ret = 8 -> [%<reg> + <simbol>] - vrednost iz memorije na adresi <reg> + <simbol>

  if (s.find_first_of("$") == 0)
  {
    // $<literal> ili $<simbol>
    s.erase(0,1);

    if (isLiteral(s))     ret = 1;
    else if (isSymbol(s))  ret = 2;
  } else if (isLiteral(s))  ret = 3;
  else if (isSymbol(s))      ret = 4;
  else if (s.find_first_of("%") == 0)
  {
    // %<reg>
    s.erase(0,1);

    if (resolveGeneralPurposeRegister(s) != -1) ret = 5;
  } else if (s.find_first_of("[") == 0)
  {
    // [%<reg>]
    int pos = s.find_last_of("]");
    if (pos == 4 || pos == 5)
    {
      s.erase(0,1);
      s.erase(std::prev(s.end()));
      
      if (s.find_first_of("%") == 0)
        s.erase(0,1);

      if (resolveGeneralPurposeRegister(s) != -1) ret = 6;
    } else
    {
      // U suprotnom je ili [%<reg> + <literal>] ili [%<reg> + <simbol>]
      ret = 7;
    }
  }

  return ret;
}

vector<string> trimStringByPlus(string s)
{
  // string moze da bude oblika
  // 1. [%<reg>+<literal>]
  // 2. [%<reg>+<simbol]
  vector<string> args;

  s.erase(remove(s.begin(), s.end(), '['), s.end());
  s.erase(remove(s.begin(), s.end(), ']'), s.end());

  s.erase(s.find("%"),1);

  int pos = s.find("+");
  
  string str1 = s.substr(0,pos);
  string str2 = s.substr(pos + 1);

  args.push_back(str1);
  args.push_back(str2);

  return args;
}