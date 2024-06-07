#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <regex>

using namespace std;

void openFile(string filename);
vector<string> prepareTextForAssembling(ifstream& file);
void trim (string& s);

bool isWhitespace(unsigned char c);

string deleteWhitespaceFromString(string s);
string removeWhitespacesAfterString(const string& str);

vector<string> getArgsFromString(string s);
string getOneArgFromString(string s);

int ldAndStOperand(string s);
vector<string> trimStringByPlus(string s);  // used for ld and st instructions

bool stajeU12Bita(string s);

/* ********** DIRECTIVES ********** */

inline bool isDirective(string s)
{
  if(regex_match(s, std::regex("(.*)(\\.)(.*)"))) return true;
  else return false;
}

inline bool isDirectiveGlobal(string s)
{
  if(std::regex_match(s, std::regex("(.global)(.*)"))) return true;
  else return false;
}

inline bool isDirectiveExtern(string s)
{
  if(std::regex_match(s, std::regex("(.extern)(.*)"))) return true;
  else return false;
}

inline bool isDirectiveSection(string s)
{
  if(std::regex_match(s, std::regex("(.section)(.*)"))) return true;
  else return false;
}

inline bool isDirectiveWord(string s)
{
  if(std::regex_match(s, std::regex("(.word)(.*)"))) return true;
  else return false;
}

inline bool isDirectiveSkip(string s)
{
  if(std::regex_match(s, std::regex("(.skip)(.*)"))) return true;
  else return false;
}

inline bool isDirectiveEnd(string s)
{
  if(std::regex_match(s, std::regex("(.end)(.*)"))) return true;
  else return false;
}

/* ********** NUMBERS ********** */

inline bool isHexNumber(string s)
{
  if(std::regex_match(s, std::regex("^0[xX][0-9a-fA-F]+$"))) return true;
  else return false;
}

inline bool isDecNumber(string s)
{
  if(std::regex_match(s, std::regex("^(([1-9][0-9]*)|(0))$"))) return true;
  else return false;
}

inline bool isLiteral(string s)   // kao bool number kod Duleta
{
  if(isHexNumber(s) || isDecNumber(s)) return true;
  else return false;
}

inline bool isLabel(string s)
{
  if(std::regex_match(s, std::regex("[a-zA-Z][a-zA-Z0-9_]*:$")))
  {
    return true; 
  } 
  else
  {
    return false;
  }
}

inline bool isSymbol(string s)
{
  if(std::regex_match(s, std::regex("[a-zA-Z][a-zA-Z0-9_]*$"))) return true;
  else return false;
}

/* ********** REGISTERS ********** */

int resolveGeneralPurposeRegister(string s);
int resolveControlAndStatusRegister(string s);

inline bool isRegisterR0toR13(string s)
{
  if(std::regex_match(s, std::regex("^r[0-9]|r10|r11|r12|r13$"))) return true;
  else return false;
}

inline bool isRegisterSPorR14(string s)
{
  if(std::regex_match(s, std::regex("^sp|r14$"))) return true;
  else return false;
}

inline bool isRegisterPCorR15(string s)
{
  if(std::regex_match(s, std::regex("^pc|r15$"))) return true;
  else return false;
}

inline bool isRegisterStatus(string s)
{
  if(std::regex_match(s, std::regex("^status$"))) return true;
  else return false;
}

inline bool isRegisterHandler(string s)
{
  if(std::regex_match(s, std::regex("^handler$"))) return true;
  else return false;
}

inline bool isRegisterCause(string s)
{
  if(std::regex_match(s, std::regex("^cause$"))) return true;
  else return false;
}

/* ********** INSTRUCTIONS ********** */

bool isInstruction(string s);

// halt
inline bool isInstructionHalt(string s)
{
  if(std::regex_match(s, std::regex("^halt$"))) return true;
  else return false;
}

// int
inline bool isInstructionInt(string s)
{
  if(std::regex_match(s, std::regex("^int$"))) return true;
  else return false;
}

// iret
inline bool isInstructionIret(string s)
{
  if(std::regex_match(s, std::regex("^iret$"))) return true;
  else return false;
}

// call operand
inline bool isInstructionCall(string s)
{
  if(std::regex_match(s, std::regex("^\\s*call\\s+(\\w+)\\s*$"))) return true;  // "^\\s*call\\s*$"
  else return false;
}

// ret
inline bool isInstructionRet(string s)
{
  if(std::regex_match(s, std::regex("^ret$"))) return true;
  else return false;
}

// jmp operand
inline bool isInstructionJmp(string s)
{
  if(std::regex_match(s, std::regex("^\\s*jmp\\s+(\\w+)\\s*$"))) return true;
  else return false;
}

// beq %gpr1, %gpr2, operand
inline bool isInstructionBeq(string s)
{
  string regexx = "^beq\\s+%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*,\\s*%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*,\\s*(\\w+)\\s*$";
  if(std::regex_match(s, std::regex(regexx))) return true;
  else return false;
}

// bne %gpr1, %gpr2, operand
inline bool isInstructionBne(string s)
{
  string regexx = "^bne\\s+%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*,\\s*%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*,\\s*(\\w+)\\s*$";
  if(std::regex_match(s, std::regex(regexx))) return true;
  else return false;
}

// bgt %gpr1, %gpr2, operand
inline bool isInstructionBgt(string s)
{
  string regexx = "^bgt\\s+%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*,\\s*%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*,\\s*(\\w+)\\s*$";
  if(std::regex_match(s, std::regex(regexx))) return true;
  else return false;
}

// push %gpr
inline bool isInstructionPush(string s)
{
  if(std::regex_match(s, std::regex("^push\\s+%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)$"))) 
  {
    //cout << "Jeste push" << endl;
    return true;
  }
  else
  {
    //cout << "Nije push" << endl;
    return false;
  }
}

// pop %gpr
inline bool isInstructionPop(string s)
{
  if(std::regex_match(s, std::regex("^pop\\s+%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)$"))) return true;
  else return false;
}

// xchg %gprS, %gprD
inline bool isInstructionXchg(string s)
{
  if(std::regex_match(s, std::regex("^xchg\\s+%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*,\\s*%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*$"))) return true;
  else return false;
}

// add %gprS, %gprD
inline bool isInstructionAdd(string s)
{
  if(std::regex_match(s, std::regex("^add\\s+%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*,\\s*%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*$"))) // ^add\\s+((r[0-15])|sp|pc)\\s*,\\s*((r[0-15])|sp|pc)$
  {
    // cout << "Jeste ADD" << endl;
    return true;
  } else 
  {
    // cout << "Nije ADD" << endl;
    return false;
  }
}

// sub %gprS, %gprD
inline bool isInstructionSub(string s)
{
  if(std::regex_match(s, std::regex("^sub\\s+%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*,\\s*%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*$"))) return true;
  else return false;
}

// mul %gprS, %gprD
inline bool isInstructionMul(string s)
{
  if(std::regex_match(s, std::regex("^mul\\s+%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*,\\s*%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*$"))) return true;
  else return false;
}

// div %gprS, %gprD
inline bool isInstructionDiv(string s)
{
  if(std::regex_match(s, std::regex("^div\\s+%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*,\\s*%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*$"))) return true;
  else return false;
}

// not %gpr
inline bool isInstructionNot(string s)
{
  if(std::regex_match(s, std::regex("^not\\s+%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)$"))) return true;
  else return false;
}

// and %gprS, %gprD
inline bool isInstructionAnd(string s)
{
  if(std::regex_match(s, std::regex("^and\\s+%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*,\\s*%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*$"))) return true;
  else return false;
}

// or %gprS, %gprD
inline bool isInstructionOr(string s)
{
  if(std::regex_match(s, std::regex("^or\\s+%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*,\\s*%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*$"))) return true;
  else return false;
}

// xor %gprS, %gprD
inline bool isInstructionXor(string s)
{
  if(std::regex_match(s, std::regex("^xor\\s+%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*,\\s*%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*$"))) return true;
  else return false;
}

// shl %gprS, %gprD
inline bool isInstructionShl(string s)
{
  if(std::regex_match(s, std::regex("^shl\\s+%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*,\\s*%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*$"))) return true;
  else return false;
}

// shr %gprS, %gprD
inline bool isInstructionShr(string s)
{
  if(std::regex_match(s, std::regex("^shr\\s+%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*,\\s*%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*$"))) return true;
  else return false;
}

// ld operand, %gpr
inline bool isInstructionLd(string s)
{
  if(std::regex_match(s, std::regex("^\\s*ld\\s+.*,\\s+.*$")))
  {
    // cout << s << " jeste LD instrukcija" << endl;
    return true;
  }
  else return false;
}

// st %gpr, operand
inline bool isInstructionSt(string s)
{
  if(std::regex_match(s, std::regex("^\\s*st\\s+.*,\\s+.*$")))
  {
    // cout << s << " jeste ST instrukcija" << endl;
    return true;
  } 
  else return false;
}

// csrrd %csr, %gpr
inline bool isInstructionCsrrd(string s)
{
  if(std::regex_match(s, std::regex("^csrrd\\s+%(status|handler|cause)\\s*,\\s*%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*$")))
  {
    // cout << s << " jeste csrrd instrukcija" << endl;
    return true;
  }
  else
  {
    // cout << s << " nije csrrd instrukcija" << endl;
    return false;
  }
}

// csrwr %grp, %csr
inline bool isInstructionCsrwr(string s)
{
  if(std::regex_match(s, std::regex("^csrwr\\s+%(r[0-9]|r10|r11|r12|r13|r14|r15|pc|sp)\\s*,\\s*%(status|handler|cause)\\s*$")))
  {
    // cout << s << " jeste csrwr instrukcija" << endl;
    return true;
  }
  else
  {
    // cout << s << " nije csrwr instrukcija" << endl;
    return false;
  }
}

bool isNoOpInstruction(string s);
bool isOneOpInstruction(string s);
bool isTwoOpInstruction(string s);
bool isThreeOpInstruction(string s);

#endif