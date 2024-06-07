#include "../inc/parserLD.h"

using namespace std;

ParserLD::ParserLD(string fileName) : fileName(fileName)
{
  inFile = ifstream(fileName);

  if (!inFile.is_open())
  {
    cout << "ERROR: Fajl sa imenom " << fileName << " nije moguce otvoriti!" << endl;
    exit(-1);
  }
}

string ParserLD::deleteWhitespaceFromString(string s)
{
  s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());

  return s;
}

vector<string> ParserLD::splitLineInTokens(string line)
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

SectionLD *ParserLD::findSectionByName(string name)
{
  for(auto i = secTable.begin(); i != secTable.end(); ++i)
  {
    if((*i)->getName() == name)
    {
      return (*i)->getSection();
    }
  }
  return nullptr;
}

SymbolLD* ParserLD::findSymbolByName(string name)
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

void ParserLD::parse()
{
  // prepareTextForAssembling mi ne treba, jer tacno znam kakav je fajl
  int position = 0;
  string line = "";
  string tmp = "";
  
  // prvi put
  std::getline(inFile, line);

  while (!inFile.eof()) {
    if(regex_match(line, std::regex("^\\s*$"))) continue;
    
    // Brisanje komentara
    while ((position = line.find("#")) != string::npos) line.erase(position);
    
    // brisanje tabova iz stringa
    line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());

    // brisanje space-ova sa pocetka stringa
    while(line.find_first_of(' ') == 0)
      line.erase(0,1);

    if (line.compare("SYMBOL_TABLE") == 0)
    {
      while (true)
      {
        getline(inFile, tmp);
        if(regex_match(tmp, std::regex("^\\s*$"))) continue;

        if ((tmp.find("SECTION_TABLE") != string::npos) || (tmp.find("LiteralTable") != string::npos)
            || (tmp.find("RelocTable") != string::npos)
            || (tmp.find("Data") != string::npos))
            {
              line = tmp;
              break;
            }
          
        vector<string> tokens = splitLineInTokens(tmp);

        // U tabeli simbola tokens uvek sadrzi 6 elemenata
        // tokens[0] - id
        // tokens[1] - name
        // tokens[2] - section
        // tokens[3] - bind
        // tokens[4] - value
        // tokens[5] - type
        SymbolLD* sym =  new SymbolLD(stoi(tokens[0]), tokens[1], tokens[2], tokens[3], stoi(tokens[4]), tokens[5]);
        symTable.push_back(sym);
      }
      // std::cout << "SYMBOL TABLE DONE" << std::endl;
    } else if (line.compare("SECTION_TABLE") == 0)
    {
      while (true)
      {
        getline(inFile, tmp);
        if(regex_match(tmp, std::regex("^\\s*$"))) continue;

        if ((tmp.find("SYMBOL_TABLE") != string::npos) || (tmp.find("LiteralTable") != string::npos)
            || (tmp.find("RelocTable") != string::npos)
            || (tmp.find("Data") != string::npos))
          {
            line = tmp;
            break;
          }
        
        vector<string> tokens = splitLineInTokens(tmp);

        // U tabeli simbola tokens uvek sadrzi 2 elemenata
        // tokens[0] - name
        // tokens[1] - size
        SectionLD* sec = new SectionLD(tokens[0], stoi(tokens[1]));
        secTable.push_back(sec);
      }
      // std::cout << "SECTION TABLE DONE" << std::endl;
    } else if (line.find("LiteralTable") != string::npos)
    {
      vector<string> tokens = splitLineInTokens(line);

      // tokens[0] = LiteralTable
      // tokens[1] = sekcija

      SectionLD* sec = ParserLD::findSectionByName(tokens[1]);

      while (true)
      {
        getline(inFile, tmp);
        if(regex_match(tmp, std::regex("^\\s*$"))) continue;

        if ((tmp.find("SYMBOL_TABLE") != string::npos) || (tmp.find("RelocTable") != string::npos)
            || (tmp.find("SECTION_TABLE") != string::npos)
            || (tmp.find("LiteralTable") != string::npos)
            || (tmp.find("Data") != string::npos))
          {
            line = tmp;
            break;
          }
        
        vector<string> tokens2 = splitLineInTokens(tmp);
        // tokens2[0] -> Value (str)
        // tokens2[1] -> Value (int)
        // tokens2[2] -> Size
        // tokens2[3] -> Location

        SymbolLD* sym = ParserLD::findSymbolByName(tokens2[0]);

        LiteralLD* lit = new LiteralLD(tokens2[0], stoi(tokens2[1]), stoi(tokens2[2]), stoi(tokens2[3]));
        vector<LiteralLD*> litTable = sec->getLiteralTable();
        litTable.push_back(lit);
        sec->setLiteralTable(litTable);
      }
      // std::cout << "LITERAL TABLE FOR SECTION " << tokens[1] << " DONE!" << std::endl;
    } else if (line.find("RelocTable") != string::npos)
    {
      vector<string> tokens = splitLineInTokens(line);

      // tokens[0] = RelocTable
      // tokens[1] = sekcija

      SectionLD* sec = ParserLD::findSectionByName(tokens[1]);

      while (true)
      {
        getline(inFile, tmp);
        if(regex_match(tmp, std::regex("^\\s*$"))) continue;

        if ((tmp.find("SYMBOL_TABLE") != string::npos) || (tmp.find("LiteralTable") != string::npos)
            || (tmp.find("SECTION_TABLE") != string::npos)
            || (tmp.find("RelocTable") != string::npos)
            || (tmp.find("Data") != string::npos))
          {
            line = tmp;
            break;
          }
        
        vector<string> tokens2 = splitLineInTokens(tmp);
        // tokens2[0] -> Symbol
        // tokens2[1] -> offset

        SymbolLD* sym = ParserLD::findSymbolByName(tokens2[0]);

        RelocSymbolLD* relSym = new RelocSymbolLD(sym, stoi(tokens2[1]));
        vector<RelocSymbolLD*> relData = sec->getRelocationTable();
        relData.push_back(relSym);
        sec->setRelocationData(relData);
      }
      // std::cout << "RELOC TABLE FOR SECTION " << tokens[1] << " DONE!" << endl;
    } else if (line.find("Data") != string::npos)
    {
      vector<string> tokens = splitLineInTokens(line);
      // cout << line << endl;

      // tokens[0] = Data
      // tokens[1] = sekcija

      SectionLD* sec = ParserLD::findSectionByName(tokens[1]);

      while (true)
      {
        getline(inFile, tmp);
        if(regex_match(tmp, std::regex("^\\s*$"))) break;   // sa continue ne radi (mora ovako jer posle Data uvek ide EOF)

        if ((tmp.find("SYMBOL_TABLE") != string::npos) || (tmp.find("LiteralTable") != string::npos)
            || (tmp.find("SECTION_TABLE") != string::npos)
            || (tmp.find("RelocTable") != string::npos)
            || (tmp.find("Data") != string::npos))
          {
            line = tmp;
            break;
          }
        
        vector<string> tokens2 = splitLineInTokens(tmp);
        // tokens2[0] -> byte1
        // tokens2[1] -> byte2
        // tokens2[2] -> byte3
        // tokens2[3] -> byte4

        vector<unsigned char> data = sec->getSectionData();

        data.push_back(stoi(tokens2[0], nullptr, 16));
        data.push_back(stoi(tokens2[1], nullptr, 16));
        data.push_back(stoi(tokens2[2], nullptr, 16));
        data.push_back(stoi(tokens2[3], nullptr, 16));

        sec->setSectionData(data);
      }
      // std::cout << "DATA FOR SECTION " << tokens[1] << " DONE!" << std::endl;
    }
	}
  // std::cout << "DONE" << std::endl;
  // cout << "PARSING DONE!" << endl;
}