#pragma once
#ifndef VARIABLE_H
#define VARIABLE_H

#include <string>
using namespace std;

class Variable {
private:
    string nameValue;
    string typeValue;
    string currentValue;
public:
    Variable();
    Variable(string nameParam, string typeParam, string valueParam);
    void setName(string newName);
    void setType(string newType);
    void setValue(string newValue);
    string getName();
    string getType();
    string getValue();
    void printVariable();
};

#endif
