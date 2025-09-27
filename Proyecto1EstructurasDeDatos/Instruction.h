#pragma once
#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <string>
using namespace std;

class Instruction {
private:
    string categoryValue;
    string operationValue;
    string parametersValue;
    int indentValue;        

public:
    Instruction();
    Instruction(string categoryParam, string operationParam, string parametersParam);
    Instruction(string categoryParam, string operationParam, string parametersParam, int indentParam);

    void setCategory(string newCategory);
    void setOperation(string newOperation);
    void setParameters(string newParameters);

    void setIndent(int indentParam);   
    int getIndent();                   

    string getCategory();
    string getOperation();
    string getParameters();
};

#endif
