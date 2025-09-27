#include "Variable.h"
#include <iostream>
using namespace std;

Variable::Variable() {
    nameValue = "";
    typeValue = "";
    currentValue = "";
}

Variable::Variable(string nameParam, string typeParam, string valueParam) {
    nameValue = nameParam;
    typeValue = typeParam;
    currentValue = valueParam;
}

void Variable::setName(string newName) {
    nameValue = newName;
}
void Variable::setType(string newType) {
    typeValue = newType;
}
void Variable::setValue(string newValue) {
    currentValue = newValue;
}
string Variable::getName() {
    return nameValue;
}
string Variable::getType() {
    return typeValue;
}
string Variable::getValue() {
    return currentValue;
}

void Variable::printVariable() {
    cout << "Variable: " << nameValue << " | Tipo: " << typeValue << " | Valor: " << currentValue << endl;
}
