#include "VariableNode.h"

VariableNode::VariableNode() {
    data = Variable();
    next = 0;
}

VariableNode::VariableNode(Variable v) {
    data = v;
    next = 0;
}
