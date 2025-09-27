#include "LineNode.h"

LineNode::LineNode() {
    textValue = "";
    next = 0;
}

LineNode::LineNode(string lineParam) {
    textValue = lineParam;
    next = 0;
}
