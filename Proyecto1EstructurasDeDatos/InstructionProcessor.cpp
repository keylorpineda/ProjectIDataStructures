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
    string original = content;

    if (dict.hasComment(lowered)) {
        string rest = helper.cutAfter(original, "comentario");
        Instruction ins("Meta", "comment", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasBeginProgram(lowered)) {
        Instruction ins("Meta", "begin_program", "");
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasEndProgram(lowered)) {
        Instruction ins("Meta", "end_program", "");
        ins.setIndent(indentSpaces);
        return ins;
    }

    if (dict.hasDefineFunction(lowered)) {
        string rest = helper.cutAfter(original, "definir funcion");
        Instruction ins("Func", "def_func", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasCallFunction(lowered)) {
        string rest = helper.cutAfter(original, "llamar funcion");
        Instruction ins("Func", "call_func", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }

    if (dict.hasIfThen(lowered)) {
        string cond = helper.cutBetweenSimple(original, "si", " entonces");
        if (cond == "") { cond = helper.cutBetweenSimple(original, "si", "entonces"); }
        Instruction ins("Control", "if", helper.trimSimple(cond));
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasElse(lowered)) {
        Instruction ins("Control", "else", "");
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasWhile(lowered)) {
        string rest = helper.cutAfter(original, "mientras");
        Instruction ins("Control", "while", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasDoUntil(lowered)) {
        string rest = helper.cutAfter(original, "repetir hasta");
        Instruction ins("Control", "do_until", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasForTo(lowered)) {
        string left = helper.cutBetweenSimple(original, "para", " hasta");
        if (left == "") { left = helper.cutBetweenSimple(original, "para", "hasta"); }
        string right = helper.trimSimple(helper.cutAfter(original, "hasta"));
        string joined = helper.trimSimple(left) + string(" ") + right; // "i = 1 5"
        Instruction ins("Control", "for_to", helper.trimSimple(joined));
        ins.setIndent(indentSpaces);
        return ins;
    }

    if (dict.hasTraverseList(lowered)) {
        string rest = helper.cutAfter(original, "recorrer la lista");
        Instruction ins("Array", "traverse_list", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasAddToList(lowered)) {
        string rest = helper.cutAfter(original, "agregar a la lista");
        Instruction ins("Array", "add_to_list", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }
    if (dict.hasReadInput(lowered)) {
        string rest = helper.cutAfter(original, "leer");
        Instruction ins("IO", "read", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }

    if (dict.hasCalculate(lowered)) {
        string rest = helper.cutAfter(original, "calcular");
        Instruction ins("Op", "calc", helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }

    string opKey = dict.findOpKey(lowered);
    if (opKey != "") {
        string rest = helper.cutAfter(original, opKey);
        Instruction ins("Op", dict.opCanonical(opKey), helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }

    string ioKey = dict.findIOKey(lowered);
    if (ioKey != "") {
        string rest = helper.cutAfter(original, ioKey);
        Instruction ins("IO", dict.ioCanonical(ioKey), helper.trimSimple(rest));
        ins.setIndent(indentSpaces);
        return ins;
    }

    Instruction ins("Unknown", "unknown", content);
    ins.setIndent(indentSpaces);
    return ins;
}
