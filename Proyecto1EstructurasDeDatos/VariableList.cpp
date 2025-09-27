#include "VariableList.h"

VariableList::VariableList() { head = 0; }
VariableList::~VariableList() { clearList(); }

VariableNode* VariableList::findNodeByName(string nameParam) {
    VariableNode* current = head;
    while (current != 0) {
        if (current->data.getName() == nameParam) {
            return current;
        }
        current = current->next;
    }
    return 0;
}

bool VariableList::variableExists(string nameParam) {
    return findNodeByName(nameParam) != 0;
}

void VariableList::addOrUpdateVariable(Variable varParam) {
    if (head == 0) {
        head = new VariableNode(varParam);
        return;
    }
    VariableNode* existing = findNodeByName(varParam.getName());
    if (existing != 0) {
        existing->data = varParam;
        return;
    }
    VariableNode* current = head;
    while (current->next != 0) {
        current = current->next;
    }
    current->next = new VariableNode(varParam);
}

void VariableList::assignValue(string nameParam, string newValue) {
    VariableNode* node = findNodeByName(nameParam);
    if (node != 0) {
        Variable temp = node->data;
        temp.setValue(newValue);
        node->data = temp;
    }
}

Variable VariableList::getVariable(string nameParam) {
    VariableNode* node = findNodeByName(nameParam);
    if (node != 0) {
        return node->data;
    }
    return Variable();
}

void VariableList::removeVariable(string nameParam) {
    if (head == 0) {
        return;
    }
    if (head->data.getName() == nameParam) {
        VariableNode* tmp = head;
        head = head->next;
        delete tmp;
        return;
    }
    VariableNode* current = head;
    while (current->next != 0) {
        if (current->next->data.getName() == nameParam) {
            VariableNode* tmp = current->next;
            current->next = tmp->next;
            delete tmp;
            return;
        }
        current = current->next;
    }
}

int VariableList::getSize() {
    int countValue = 0;
    VariableNode* current = head;
    while (current != 0) {
        countValue = countValue + 1;
        current = current->next;
    }
    return countValue;
}

bool VariableList::isEmpty() { return head == 0; }

void VariableList::clearList() {
    VariableNode* current = head;
    while (current != 0) {
        VariableNode* nextNode = current->next;
        delete current;
        current = nextNode;
    }
    head = 0;
}
