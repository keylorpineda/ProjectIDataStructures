#pragma once
#ifndef SYSTEMMANAGER_H
#define SYSTEMMANAGER_H

#include <string>
#include "InstructionProcessor.h"
#include "CodeGenerator.h"
#include "TextFileReader.h"
#include "CodeWriter.h"
using namespace std;

class SystemManager {
private:
    InstructionProcessor processor;
    CodeGenerator generator;
    CodeWriter writer;
public:
    SystemManager();
    void processConsole();
    bool processFile(string path);
    string getProgram();
    bool exportProgram(string path);
    void resetAll();
};

#endif
