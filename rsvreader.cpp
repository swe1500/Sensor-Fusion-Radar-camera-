#include "rsvreader.h"


string RsvReader::Trim(string &str){
    str.erase(0,str.find_first_not_of(" \t\r\n"));
    str.erase(str.find_last_not_of(" \t\r\n") + 1);
    return str;
}

float RsvReader::String2Num(const string &str){
    istringstream iss(str);
    float num;
    iss >> num;
    return num;
}

vector<vector<float> > RsvReader::CsvConverter(){

    vector<vector<float> > fields;
    while(getline(file,line)){
       istringstream sin(line);
       vector<float> field;
       string str;
       while(getline(sin, str, ',')){
           field.push_back(String2Num(str));
       }
       fields.push_back(field);
    }
    return fields;
}

