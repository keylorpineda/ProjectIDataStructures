#pragma once
#ifndef CODEWRITER_H
#define CODEWRITER_H

#include <string>
using namespace std;

class CodeWriter {
public:
    bool writeToFile(string path, string content);
};

#endif
