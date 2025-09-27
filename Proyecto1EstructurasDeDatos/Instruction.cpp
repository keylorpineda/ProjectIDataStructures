#include "Instruction.h"

Instruction::Instruction() {
    categoryValue = "";
    operationValue = "";
    parametersValue = "";
    indentValue = 0;
}

Instruction::Instruction(string categoryParam, string operationParam, string parametersParam) {
    categoryValue = categoryParam;
    operationValue = operationParam;
    parametersValue = parametersParam;
    indentValue = 0;
}

Instruction::Instruction(string categoryParam, string operationParam, string parametersParam, int indentParam) {
    categoryValue = categoryParam;
    operationValue = operationParam;
    parametersValue = parametersParam;
    indentValue = indentParam;
}

void Instruction::setCategory(string newCategory) {
    categoryValue = newCategory;
}

void Instruction::setOperation(string newOperation) {
    operationValue = newOperation;
}

void Instruction::setParameters(string newParameters) {
    parametersValue = newParameters;
}

void Instruction::setIndent(int indentParam) {
    indentValue = indentParam;
}

int Instruction::getIndent() {
    return indentValue;
}

string Instruction::getCategory() {
    return categoryValue;
}

string Instruction::getOperation() {
    return operationValue;
}

string Instruction::getParameters() {
    return parametersValue;
}
