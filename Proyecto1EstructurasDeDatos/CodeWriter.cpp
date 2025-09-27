#include "CodeWriter.h"
#include <fstream>

bool CodeWriter::writeToFile(string path, string content) {
    ofstream out(path.c_str());
    if (!out.is_open()) { return false; }
    out << content;
    out.close();
    return true;
}
