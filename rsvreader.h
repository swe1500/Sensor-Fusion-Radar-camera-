#ifndef RSVREADER_H
#define RSVREADER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;

class RsvReader{
private:
    ifstream file;
    string line;

    string Trim(string& str);
    float  String2Num(const string & str);

public:
    inline RsvReader(const char* f):file(f){}
    ~RsvReader(){file.close();}

    vector<vector<float> > CsvConverter();
};
#endif // RSVREADER_H
