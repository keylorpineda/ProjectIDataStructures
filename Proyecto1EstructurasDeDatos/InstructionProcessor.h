#pragma once
#ifndef INSTRUCTIONPROCESSOR_H
#define INSTRUCTIONPROCESSOR_H

#include <string>
#include "Instruction.h"
#include "LanguageDictionary.h"
#include "TextHelper.h"
using namespace std;

class InstructionProcessor {
private:
    LanguageDictionary dict;
    TextHelper helper;
public:
    InstructionProcessor();
    Instruction parseInstruction(string lineText);
};

#endif
