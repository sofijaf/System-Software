#ifndef PARSER_EM_H
#define PARSER_EM_H

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <regex>

using namespace std;

class ParserEM
{
private:
  vector<pair<unsigned int, unsigned char>> data;

  string fileName;
  ifstream inFile;

  vector<string> splitLineInTokens(string line);
public:
  ParserEM(string inFile);
  vector<pair<unsigned int, unsigned char>> parse();

  vector<pair<unsigned int, unsigned char>> getData() { return this->data; }
};

#endif