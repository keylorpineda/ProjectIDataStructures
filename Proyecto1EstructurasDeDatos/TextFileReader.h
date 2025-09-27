#pragma once
#ifndef TEXTFILEREADER_H
#define TEXTFILEREADER_H

#include <string>
#include <fstream>
using namespace std;

class TextFileReader {
private:
    ifstream inStream;
public:
    bool openFile(string path);
    bool isOpen();
    bool readLine(string& lineRead);
    void closeFile();
};

#endif
