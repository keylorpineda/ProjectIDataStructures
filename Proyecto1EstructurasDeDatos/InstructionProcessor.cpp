#include "InstructionProcessor.h"

static int countLeadingIndentSpaces(string text) {
    int i = 0;
    int n = (int)text.length();
    int spaces = 0;
    while (i < n) {
        char c = text[i];
        if (c == ' ') { spaces = spaces + 1; i = i + 1; }
        else if (c == '\t') { spaces = spaces + 4; i = i + 1; }
        else { break; }
    }
    return spaces;
}

InstructionProcessor::InstructionProcessor() {
}

Instruction InstructionProcessor::parseInstruction(string lineText) {
    int indentSpaces = countLeadingIndentSpaces(lineText);
    string content = lineText.substr(indentSpaces);    
    string lowered = helper.toLowerSimple(content);
    string normalized = helper.removeAccents(lowered);
    string original = content;
    string trimmedOriginal = helper.trimSimple(original);
    string trimmedNormalized = helper.removeAccents(helper.toLowerSimple(trimmedOriginal));

    auto cutAfterInsensitive = [&](const string& pattern) -> string {
        int pos = (int)lowered.find(pattern);
        if (pos < 0) { return original; }
        int begin = pos + (int)pattern.length();
        if (begin < 0) { begin = 0; }
        if (begin > (int)original.length()) { return string(""); }
        return original.substr(begin);
    };

    auto cutBetweenInsensitive = [&](const string& start, const string& end) -> string {
        int startPos = (int)lowered.find(start);
        if (startPos < 0) { return string(""); }
        int begin = startPos + (int)start.length();
        if (begin < 0) { begin = 0; }
        if (begin > (int)original.length()) { return string(""); }
        int endPos = -1;
        if (end != "") {
            endPos = (int)lowered.find(end, begin);
        }
        if (endPos < 0) {
            return helper.trimSimple(original.substr(begin));
        }
        return helper.trimSimple(original.substr(begin, endPos - begin));
    };

    if (dict.hasComment(normalized)) {
        string rest = cutAfterInsensitive("comentario");
        Instruction ins("Meta", "comment", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasBeginProgram(normalized)) {
        Instruction ins("Meta", "begin_program", "");
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasEndProgram(normalized)) {
        Instruction ins("Meta", "end_program", "");
        ins.setIndent(indentSpaces);
        return ins;
    }

    if (dict.hasDefineFunction(normalized)) {
        string rest = cutAfterInsensitive("definir funcion");
        if (rest == original) {
            rest = cutAfterInsensitive("definir función");
        }
        Instruction ins("Func", "def_func", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasCallFunction(normalized)) {
        string rest = cutAfterInsensitive("llamar funcion");
        if (rest == original) {
            rest = cutAfterInsensitive("llamar función");
        }
        Instruction ins("Func", "call_func", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }

    bool startsWithSimpleIf = false;
    if ((int)trimmedNormalized.length() >= 3) {
        if (trimmedNormalized.rfind("si ", 0) == 0 && (int)trimmedNormalized.find(" entonces") < 0) {
            startsWithSimpleIf = true;
        }
        else if (trimmedNormalized.rfind("si(", 0) == 0 && (int)trimmedNormalized.find(" entonces") < 0) {
            startsWithSimpleIf = true;
        }
    }

    if (dict.hasIfThen(normalized) || startsWithSimpleIf) {
        string cond = cutBetweenInsensitive("si", " entonces");
        if (cond == "") { cond = cutBetweenInsensitive("si", "entonces"); }
        if (cond == "" && startsWithSimpleIf) {
            if ((int)trimmedOriginal.length() > 2) {
                cond = helper.trimSimple(trimmedOriginal.substr(2));
            }
        }
        if (cond == "") {
            cond = helper.trimSimple(cutAfterInsensitive("si"));
        }
        Instruction ins("Control", "if", helper.trimSimple(cond));
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasElse(normalized)) {
        Instruction ins("Control", "else", "");
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasWhile(normalized)) {
        string rest = cutAfterInsensitive("mientras");
        Instruction ins("Control", "while", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasDoUntil(normalized)) {
        string rest = cutAfterInsensitive("repetir hasta");
        Instruction ins("Control", "do_until", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasForTo(normalized)) {
        string left = cutBetweenInsensitive("para", " hasta");
        if (left == "") { left = cutBetweenInsensitive("para", "hasta"); }
        string right = helper.trimSimple(cutAfterInsensitive("hasta"));
        string joined = helper.trimSimple(left) + string(" ") + right; // "i = 1 5"
        Instruction ins("Control", "for_to", helper.trimSimple(joined));
        ins.setIndent(indentSpaces);
        return ins;
    }

    if (dict.hasTraverseList(normalized)) {
        string rest = cutAfterInsensitive("recorrer la lista");
        Instruction ins("Array", "traverse_list", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasAddToList(normalized)) {
        string rest = cutAfterInsensitive("agregar a la lista");
        Instruction ins("Array", "add_to_list", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasReadInput(normalized)) {
        string rest = original;
        if ((int)normalized.find("ingresar valor") >= 0) {
            rest = cutAfterInsensitive("ingresar valor");
        }
        else {
            rest = cutAfterInsensitive("leer");
        }
        Instruction ins("IO", "read", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }

    if (dict.hasCalculate(normalized)) {
        string rest = cutAfterInsensitive("calcular");
        Instruction ins("Op", "calc", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }

    string opKey = dict.findOpKey(normalized);
    if (opKey != "") {
        string rest = cutAfterInsensitive(opKey);
        Instruction ins("Op", dict.opCanonical(opKey), helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }

    string ioKey = dict.findIOKey(normalized);
    if (ioKey != "") {
        string rest = cutAfterInsensitive(ioKey);
        Instruction ins("IO", dict.ioCanonical(ioKey), helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }

    Instruction ins("Unknown", "unknown", content);
    ins.setIndent(indentSpaces);
    return ins;
}
