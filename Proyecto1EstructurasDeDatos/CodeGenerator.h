#pragma once
#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <string>
#include "Instruction.h"
#include "VariableList.h"
#include "LineList.h"
#include "TextHelper.h"
using namespace std;

class CodeGenerator {
private:
    VariableList symbolTable;
    LineList preludeLines;
    LineList bodyLines;
    TextHelper helper;

    int arrayCounter;
    int lastIndent;
    int stackCount;
    int indentStack[64];
    string closeLineStack[64];
    bool headerJustEmitted;
    string nextCloseLine;

    void beforeEmit(int newIndent);
    void afterEmit(int newIndent);
    void openBlock(int indentLevel);
    void closeOneBlock();
    void closeTo(int targetIndent);
    void closeAllBlocks();

    string normalizeMathTokens(string text);
    string normalizeConditionTokens(string text);
    string removeSpaces(string text);

    bool splitAssignRight(string text, string& varName, string& value);
    bool parseForParts(string text, string& varName, string& startVal, string& endVal);
    string inferTypeFromToken(string token);
    string nextArrayName();

    void ensureDeclared(string varName, string typeName, string initValue);

    void genSum(string params);
    void genSub(string params);
    void genMul(string params);
    void genDiv(string params);
    void genCalc(string params);

    void genPrint(string params);
    void genMessage(string params);
    void genRead(string params);

    void genIf(string params);
    void genElse();
    void genWhile(string params);
    void genDoUntil(string params);
    void genForTo(string params);

    void genCreateVar(string params);
    void genAssign(string params);

    void genCreateArray(string params);
    void genTraverseList(string params);
    void genAddToList(string params);

    void genBeginProgram();
    void genEndProgram();
    void genComment(string params);
    void genDefineFunction(string params);
    void genCallFunction(string params);

public:
    CodeGenerator();
    void generate(Instruction ins);
    string buildProgram();
    void clearAll();
};

#endif
