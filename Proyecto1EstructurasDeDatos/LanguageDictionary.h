#pragma once
#ifndef LANGUAGEDICTIONARY_H
#define LANGUAGEDICTIONARY_H

#include <map>
#include <string>
using namespace std;

class LanguageDictionary {
private:
    map<string, string> mapOp;
    map<string, string> mapIO;
    map<string, string> mapCtrl;
public:
    LanguageDictionary();
    string findOpKey(string text);
    string findIOKey(string text);
    string opCanonical(string key);
    string ioCanonical(string key);

    bool hasIfThen(string text);
    bool hasElse(string text);
    bool hasWhile(string text);
    bool hasForTo(string text);
    bool hasDoUntil(string text);

    bool hasBeginProgram(string text);
    bool hasEndProgram(string text);
    bool hasComment(string text);

    bool hasDefineFunction(string text);
    bool hasCallFunction(string text);

    bool hasCalculate(string text);
    bool hasTraverseList(string text);
    bool hasAddToList(string text);
    bool hasReadInput(string text);
};

#endif
