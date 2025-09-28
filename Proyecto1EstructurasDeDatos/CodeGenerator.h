#pragma once
#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <map>
#include <string>
#include <utility>
#include <vector>
#include "Instruction.h"
#include "VariableList.h"
#include "LineList.h"
#include "TextHelper.h"
using namespace std;

class CodeGenerator {
private:
    struct ArrayInfo {
        string name;
        string elementType;
        int size;
    };

    struct StructInfo {
        string name;
        vector<pair<string, string>> fields;
    };

    VariableList symbolTable;
    LineList preludeLines;
    LineList bodyLines;
    TextHelper helper;
    map<string, ArrayInfo> arrayTable;
    map<string, StructInfo> structTable;
    ArrayInfo lastArrayInfo;
    bool hasLastArray;
    string lastLoopIndex;
    string lastLoopArray;
    LineList* activeLines;
    bool insideFunction;
    int functionIndent;
    int functionBodyIndent;

    int arrayCounter;
    int lastIndent;
    int stackCount;
    int indentStack[64];
    string closeLineStack[64];
    LineList* lineStack[64];
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
    int effectiveIndent(int rawIndent);
    string toLowerNoAccents(string text);
    string normalizeBooleanWord(string text);
    vector<string> splitPrintSegments(string text);
    string detectArrayReference(string text);
    ArrayInfo resolveArrayForText(string text);
    string arraySizeExpression(const ArrayInfo& info);
    string selectIndexName(string preferred);
    void appendLine(const string& text);
    void appendToPrelude(const string& text);
    void registerArray(const ArrayInfo& info);
    void registerStruct(const StructInfo& info);
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
    void genSumAssign(string params);
    void genSubAssign(string params);
    void genMulAssign(string params);
    void genDivAssign(string params);

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
    void genCreateStruct(string params);
    void genReturn(string params);

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
