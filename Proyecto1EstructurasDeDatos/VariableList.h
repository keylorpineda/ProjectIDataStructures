#pragma once
#ifndef VARIABLELIST_H
#define VARIABLELIST_H

#include <string>
#include "VariableNode.h"
using namespace std;

class VariableList {
private:
    VariableNode* head;
    VariableNode* findNodeByName(string nameParam);
public:
    VariableList();
    ~VariableList();
    bool variableExists(string nameParam);
    void addOrUpdateVariable(Variable varParam);
    void assignValue(string nameParam, string newValue);
    Variable getVariable(string nameParam);
    void removeVariable(string nameParam);
    int getSize();
    bool isEmpty();
    void clearList();
};

#endif
