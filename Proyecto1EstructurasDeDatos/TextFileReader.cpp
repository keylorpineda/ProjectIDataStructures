#include "TextFileReader.h"

bool TextFileReader::openFile(string path) {
    inStream.open(path.c_str());
    if (inStream.is_open()) { return true; }
    return false;
}

bool TextFileReader::isOpen() {
    return inStream.is_open();
}

bool TextFileReader::readLine(string& lineRead) {
    if (!inStream.is_open()) { return false; }
    if (!getline(inStream, lineRead)) { return false; }
    return true;
}

void TextFileReader::closeFile() {
    if (inStream.is_open()) { inStream.close(); }
}
