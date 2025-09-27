#pragma once
#ifndef TEXTHELPER_H
#define TEXTHELPER_H

#include <string>
using namespace std;

class TextHelper {
private:
    bool isSpaceSimple(char c);
    bool isDigitSimple(char c);
    char toLowerAscii(char c);
public:
    string toLowerSimple(string text);
    string trimSimple(string text);
    int indexOfText(string text, string key);
    string cutAfter(string base, string key);
    string cutBetweenSimple(string text, string startKey, string endKey);
    string replaceAllSimple(string text, string findWhat, string putWhat);
};

#endif
