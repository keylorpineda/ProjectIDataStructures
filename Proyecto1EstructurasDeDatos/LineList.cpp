#include "LineList.h"

LineList::LineList() { head = 0; }
LineList::~LineList() { clearAll(); }

LineNode* LineList::getTail() {
    if (head == 0) {
        return 0;
    }
    LineNode* current = head;
    while (current->next != 0) {
        current = current->next;
    }
    return current;
}

void LineList::appendLine(string lineText) {
    LineNode* node = new LineNode(lineText);
    if (head == 0) {
        head = node;
        return;
    }
    LineNode* tail = getTail();
    tail->next = node;
}

void LineList::clearAll() {
    LineNode* current = head;
    while (current != 0) {
        LineNode* nextNode = current->next;
        delete current;
        current = nextNode;
    }
    head = 0;
}

string LineList::joinAll() {
    string output = "";
    LineNode* current = head;
    while (current != 0) {
        output = output + current->textValue;
        if (current->next != 0) {
            output = output + "\n";
        }
        current = current->next;
    }
    return output;
}

bool LineList::isEmpty() { return head == 0; }
