#pragma once
#ifndef LINENODE_H
#define LINENODE_H

#include <string>
using namespace std;

class LineNode {
public:
    string textValue;
    LineNode* next;
    LineNode();
    LineNode(string lineParam);
};

#endif
