#include "SystemManager.h"
#include <iostream>
using namespace std;

SystemManager::SystemManager() {
}

void SystemManager::processConsole() {
    cout << "Ingrese instruccion (o 'exit' para salir):" << endl;
    string lineText;
    while (true) {
        cout << "> ";
        if (!getline(cin, lineText)) {
            break;
        }
        if (lineText == "exit") {
            break;
        }
        if (lineText == "") {
            continue;
        }
        Instruction ins = processor.parseInstruction(lineText);
        generator.generate(ins);
        cout << "OK" << endl;
    }
    cout << "=== Codigo Generado ===" << endl;
    cout << generator.buildProgram() << endl;
}

bool SystemManager::processFile(string path) {
    TextFileReader reader;
    if (!reader.openFile(path)) {
        return false;
    }
    string lineText;
    while (reader.readLine(lineText))
    {
        if (lineText == "")
        {
            continue;
        }
        Instruction ins = processor.parseInstruction(lineText);
        generator.generate(ins);
    }
    reader.closeFile();
    return true;
}

string SystemManager::getProgram() {
    return generator.buildProgram();
}

bool SystemManager::exportProgram(string path) {
    string code = generator.buildProgram();
    return writer.writeToFile(path, code);
}

void SystemManager::resetAll() {
    generator.clearAll();
}
