#include "LanguageDictionary.h"
#include <cctype>

LanguageDictionary::LanguageDictionary() {
    mapOp["sumar"] = "sum";
    mapOp["suma"] = "sum";
    mapOp["restar"] = "sub";
    mapOp["resta"] = "sub";
    mapOp["multiplicar"] = "mul";
    mapOp["multiplica"] = "mul";
    mapOp["por"] = "mul";
    mapOp["dividir"] = "div";
    mapOp["divide"] = "div";
    mapOp["entre"] = "div";
    mapOp["calcular"] = "calc";

    mapOp["crear variable"] = "create_var";
    mapOp["asignar valor"] = "assign";
    mapOp["asignar"] = "assign";

    mapOp["crear lista"] = "create_array";
    mapOp["crear arreglo"] = "create_array";
    mapOp["lista de"] = "array_of";
    mapOp["recorrer la lista"] = "traverse_list";
    mapOp["ingresar los valores de la lista"] = "fill_array";
    mapOp["ingresar los valores a la lista"] = "fill_array";
    mapOp["ingresar los valores"] = "fill_array";
    mapOp["ingresar valor de cada"] = "fill_array";
    mapOp["ingresar los datos de cada"] = "fill_array";
    mapOp["agregar a la lista"] = "add_to_list";
    mapOp["crear estructura"] = "create_struct";

    mapIO["imprimir"] = "print";
    mapIO["mostrar"] = "print";
    mapIO["mensaje"] = "message";
    mapIO["leer"] = "read";
    mapIO["ingresar valor"] = "read";

    mapCtrl["si"] = "if";
    mapCtrl["sino"] = "else";
    mapCtrl["mientras"] = "while";
    mapCtrl["repetir hasta"] = "do_until";
    mapCtrl["para"] = "for";
    mapCtrl["comenzar programa"] = "begin_program";
    mapCtrl["terminar programa"] = "end_program";
    mapCtrl["fin"] = "end_program";
    mapCtrl["comentario"] = "comment";
    mapCtrl["definir funcion"] = "def_func";
    mapCtrl["llamar funcion"] = "call_func";
    mapCtrl["retornar"] = "return";
    mapCtrl["retorne"] = "return";
}

static bool isWordBoundaryMatch(const string& text, size_t pos, size_t length) {
    if (pos > 0) {
        unsigned char prev = static_cast<unsigned char>(text[pos - 1]);
        if (std::isalnum(prev) || prev == '_') {
            return false;
        }
    }
    size_t end = pos + length;
    if (end < text.length()) {
        unsigned char next = static_cast<unsigned char>(text[end]);
        if (std::isalnum(next) || next == '_') {
            return false;
        }
    }
    return true;
}

string LanguageDictionary::findOpKey(string text) {
    string bestKey = "";
    size_t bestLength = 0;
    for (auto& entry : mapOp) {
        size_t pos = text.find(entry.first);
        if (pos != string::npos && isWordBoundaryMatch(text, pos, entry.first.length())) {
            size_t currentLength = entry.first.length();
            if (currentLength > bestLength) {
                bestKey = entry.first;
                bestLength = currentLength;
            }
        }
    }
    return bestKey;
}

string LanguageDictionary::findIOKey(string text) {
    string bestKey = "";
    size_t bestLength = 0;
    for (auto& entry : mapIO) {
        size_t pos = text.find(entry.first);
        if (pos != string::npos && isWordBoundaryMatch(text, pos, entry.first.length())) {
            size_t currentLength = entry.first.length();
            if (currentLength > bestLength) {
                bestKey = entry.first;
                bestLength = currentLength;
            }
        }
    }
    return bestKey;
}

string LanguageDictionary::opCanonical(string key) {
    map<string, string>::iterator it = mapOp.find(key);
    if (it != mapOp.end()) {
        return it->second;
    }
    return "";
}

string LanguageDictionary::ioCanonical(string key) {
    map<string, string>::iterator it = mapIO.find(key);
    if (it != mapIO.end()) {
        return it->second;
    }
    return "";
}

bool LanguageDictionary::hasIfThen(string text) {
    if ((int)text.find("si") >= 0 && (int)text.find("entonces") >= 0) {
        return true;
    }
    return false;
}
bool LanguageDictionary::hasElse(string text) { return (int)text.find("sino") >= 0; }
bool LanguageDictionary::hasWhile(string text) { return (int)text.find("mientras") >= 0; }
bool LanguageDictionary::hasForTo(string text) { return (int)text.find("para") >= 0 && (int)text.find("hasta") >= 0; }
bool LanguageDictionary::hasDoUntil(string text) { return (int)text.find("repetir hasta") >= 0; }

bool LanguageDictionary::hasBeginProgram(string text) { return (int)text.find("comenzar programa") >= 0; }
bool LanguageDictionary::hasEndProgram(string text) {
    if ((int)text.find("terminar programa") >= 0) {
        return true;
    }
    size_t first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        return false;
    }
    size_t last = text.find_last_not_of(" \t\r\n");
    string trimmed = text.substr(first, last - first + 1);
    return trimmed == "fin";
}
bool LanguageDictionary::hasComment(string text) { return (int)text.find("comentario") >= 0; }

bool LanguageDictionary::hasDefineFunction(string text) { return (int)text.find("definir funcion") >= 0; }
bool LanguageDictionary::hasCallFunction(string text) { return (int)text.find("llamar funcion") >= 0; }

bool LanguageDictionary::hasCalculate(string text) { return (int)text.find("calcular") >= 0; }
bool LanguageDictionary::hasTraverseList(string text) { return (int)text.find("recorrer la lista") >= 0; }
bool LanguageDictionary::hasAddToList(string text) { return (int)text.find("agregar a la lista") >= 0; }
bool LanguageDictionary::hasReadInput(string text) { return (int)text.find("leer") >= 0 || (int)text.find("ingresar valor") >= 0; }
