#include "../inc/parserEM.h"

ParserEM::ParserEM(string fileName) : fileName(fileName)
{
  inFile = ifstream(fileName);

  if (!inFile.is_open())
  {
    cout << "ERROR: Fajl sa imenom " << fileName << " nije moguce otvoriti!" << endl;
    exit(-1);
  }
}

vector<string> ParserEM::splitLineInTokens(string line)
{
  vector<string> args = vector<string>();

  // string new_string = deleteWhitespaceFromString(line);
  std::stringstream ss(line);

  while( ss.good() )
  {
    string substr;
    getline( ss, substr, ' ' );
    if(regex_match(substr, std::regex("^\\s*$"))) continue;
    else args.push_back( substr );
  }

  return args;
}

vector<pair<unsigned int, unsigned char>> ParserEM::parse()
{
  int position = 0;
  string line = "";
  
  while (!inFile.eof()) {
    std::getline(inFile, line);
    // cout << "Line: " << line << endl;

    if(regex_match(line, std::regex("^\\s*$"))) continue;
    
    // Brisanje komentara
    while ((position = line.find("#")) != string::npos) line.erase(position);
    
    // brisanje tabova iz stringa
    line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());

    // brisanje space-ova sa pocetka stringa
    while(line.find_first_of(' ') == 0)
      line.erase(0,1);

    vector<string> tokens = splitLineInTokens(line);

    // for (int i = 0; i < tokens.size(); i++)
    //   cout << tokens[i] << endl;

    // cout << hex << addr << endl;
    
    // Svaka linija ima ili 5 ili 9 tokena
    // tokens[0] -> adresa
    // tokens[1]-tokens[size] -> data
    unsigned int addr = stol(tokens[0], nullptr, 16);
    
    if (tokens.size() == 9)
    {
      unsigned char byte1 = (unsigned char)stoi(tokens[1], nullptr, 16);
      unsigned char byte2 = (unsigned char)stoi(tokens[2], nullptr, 16);
      unsigned char byte3 = (unsigned char)stoi(tokens[3], nullptr, 16);
      unsigned char byte4 = (unsigned char)stoi(tokens[4], nullptr, 16);
      unsigned char byte5 = (unsigned char)stoi(tokens[5], nullptr, 16);
      unsigned char byte6 = (unsigned char)stoi(tokens[6], nullptr, 16);
      unsigned char byte7 = (unsigned char)stoi(tokens[7], nullptr, 16);
      unsigned char byte8 = (unsigned char)stoi(tokens[8], nullptr, 16);

      data.emplace_back(std::make_pair(addr, byte1));
      data.emplace_back(std::make_pair(addr+1, byte2));
      data.emplace_back(std::make_pair(addr+2, byte3));
      data.emplace_back(std::make_pair(addr+3, byte4));
      data.emplace_back(std::make_pair(addr+4, byte5));
      data.emplace_back(std::make_pair(addr+5, byte6));
      data.emplace_back(std::make_pair(addr+6, byte7));
      data.emplace_back(std::make_pair(addr+7, byte8));
    } else if (tokens.size() == 5)
    {
      unsigned char byte1 = (unsigned char)stoi(tokens[1], nullptr, 16);
      unsigned char byte2 = (unsigned char)stoi(tokens[2], nullptr, 16);
      unsigned char byte3 = (unsigned char)stoi(tokens[3], nullptr, 16);
      unsigned char byte4 = (unsigned char)stoi(tokens[4], nullptr, 16);

      data.emplace_back(std::make_pair(addr, byte1));
      data.emplace_back(std::make_pair(addr+1, byte2));
      data.emplace_back(std::make_pair(addr+2, byte3));
      data.emplace_back(std::make_pair(addr+3, byte4));
    }
  }

  return data;

  // for (auto it = data.begin(); it != data.end(); ++it)
  // {
  //   cout << hex << it->first << " : " << hex << (int)it->second << endl;
  // }
}