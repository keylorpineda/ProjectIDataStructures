#pragma once
#ifndef VARIABLENODE_H
#define VARIABLENODE_H

#include "Variable.h"

class VariableNode {
public:
    Variable data;
    VariableNode* next;
    VariableNode();
    VariableNode(Variable v);
};

#endif
