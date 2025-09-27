#pragma once
#ifndef LINELIST_H
#define LINELIST_H

#include "LineNode.h"
#include <string>
using namespace std;

class LineList {
private:
    LineNode* head;
    LineNode* getTail();
public:
    LineList();
    ~LineList();
    void appendLine(string lineText);
    void clearAll();
    string joinAll();
    bool isEmpty();
};

#endif
