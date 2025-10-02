#include "CodeGenerator.h"
#include <sstream>

static string replaceUtf8Accents(string text) {
    string result;
    size_t i = 0;
    while (i < text.size()) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        if (c == 0xC3 && i + 1 < text.size()) {
            unsigned char next = static_cast<unsigned char>(text[i + 1]);
            char replacement = 0;
            switch (next) {
            case 0x81:
            case 0xA1:
                replacement = 'a';
                break;
            case 0x89:
            case 0xA9:
                replacement = 'e';
                break;
            case 0x8D:
            case 0xAD:
                replacement = 'i';
                break;
            case 0x93:
            case 0xB3:
                replacement = 'o';
                break;
            case 0x9A:
            case 0xBA:
                replacement = 'u';
                break;
            case 0x9C:
            case 0xBC:
                replacement = 'u';
                break;
            case 0x91:
            case 0xB1:
                replacement = 'n';
                break;
            default:
                break;
            }
            if (replacement != 0) {
                result.push_back(replacement);
                i = i + 2;
                continue;
            }
        }
        if (c == 0xC2 && i + 1 < text.size()) {
            unsigned char next = static_cast<unsigned char>(text[i + 1]);
            if (next == 0xA1 || next == 0xBF) {
                i = i + 2;
                continue;
            }
        }
        result.push_back(text[i]);
        i = i + 1;
    }
    return result;
}

CodeGenerator::CodeGenerator() {
    arrayCounter = 1;
    lastIndent = 0;
    stackCount = 0;
    headerJustEmitted = false;
    nextCloseLine = "";
    activeLines = &bodyLines;
    insideFunction = false;
    functionIndent = 0;
    functionBodyIndent = 0;
    hasLastArray = false;
    lastLoopIndex = "";
    lastLoopArray = "";
    hasSavedSymbolTable = false;
}

void CodeGenerator::appendLine(const string& text) {
    if (activeLines == 0) {
        activeLines = &bodyLines;
    }
    activeLines->appendLine(text);
}

void CodeGenerator::appendToPrelude(const string& text) {
    preludeLines.appendLine(text);
}

string CodeGenerator::toLowerNoAccents(string text) {
    string replaced = replaceUtf8Accents(text);
    string lowered = helper.toLowerSimple(replaced);
    return helper.removeAccents(lowered);
}

string CodeGenerator::normalizeBooleanWord(string text) {
    string lowered = toLowerNoAccents(helper.trimSimple(text));
    if (lowered == "verdadero" || lowered == "cierto" || lowered == "true") {
        return "true";
    }
    if (lowered == "falso" || lowered == "false") {
        return "false";
    }
    return text;
}

vector<string> CodeGenerator::splitPrintSegments(string text) {
    vector<string> parts;
    string current;
    bool inString = false;
    int n = (int)text.length();
    int i = 0;
    while (i < n) {
        char c = text[i];
        if (c == '\"') {
            inString = !inString;
            current.push_back(c);
            i = i + 1;
            continue;
        }
        if (!inString) {
            if (i + 2 < n) {
                string maybe = text.substr(i, 3);
                string lowered = helper.toLowerSimple(maybe);
                if (lowered == " y ") {
                    parts.push_back(helper.trimSimple(current));
                    current.clear();
                    i = i + 3;
                    continue;
                }
            }
            if (c == ',') {
                parts.push_back(helper.trimSimple(current));
                current.clear();
                i = i + 1;
                continue;
            }
        }
        current.push_back(c);
        i = i + 1;
    }
    string lastPart = helper.trimSimple(current);
    if (lastPart != "") {
        parts.push_back(lastPart);
    }
    return parts;
}

string CodeGenerator::detectArrayReference(string text) {
    string lowered = toLowerNoAccents(text);
    for (auto& it : arrayTable) {
        string loweredName = toLowerNoAccents(it.first);
        if ((int)lowered.find(loweredName) >= 0) {
            return it.first;
        }
        string baseName = loweredName;
        while (!baseName.empty() && baseName.back() >= '0' && baseName.back() <= '9') {
            baseName.pop_back();
        }
        if (!baseName.empty()) {
            string bracketPattern = baseName + "[";
            string dotPattern = baseName + ".";
            if ((int)lowered.find(bracketPattern) >= 0 || (int)lowered.find(dotPattern) >= 0) {
                return it.first;
            }
        }
    }
    if (hasLastArray) {
        return lastArrayInfo.name;
    }
    return "";
}

CodeGenerator::ArrayInfo CodeGenerator::resolveArrayForText(string text) {
    string arrayName = detectArrayReference(text);
    if (arrayName != "") {
        auto it = arrayTable.find(arrayName);
        if (it != arrayTable.end()) {
            return it->second;
        }
    }
    if (hasLastArray) {
        return lastArrayInfo;
    }
    ArrayInfo info;
    info.name = "lista1";
    info.elementType = "int";
    info.size = 0;
    return info;
}

string CodeGenerator::arraySizeExpression(const ArrayInfo& info) {
    if (info.size > 0) {
        ostringstream os;
        os << info.size;
        return os.str();
    }
    return "N";
}

string CodeGenerator::selectIndexName(string preferred) {
    if (preferred == "") {
        preferred = "i";
    }
    if (preferred == lastLoopIndex) {
        return preferred;
    }
    lastLoopIndex = preferred;
    return preferred;
}

void CodeGenerator::registerArray(const ArrayInfo& info) {
    arrayTable[info.name] = info;
    lastArrayInfo = info;
    hasLastArray = true;
}

void CodeGenerator::registerStruct(const StructInfo& info) {
    structTable[info.name] = info;
}

int CodeGenerator::effectiveIndent(int rawIndent) {
    if (insideFunction) {
        int adjusted = rawIndent - functionBodyIndent;
        if (adjusted < 0) {
            adjusted = 0;
        }
        return adjusted;
    }
    return rawIndent;
}

void CodeGenerator::beforeEmit(int newIndentRaw) {
    if (insideFunction) {
        if (newIndentRaw <= functionIndent) {
            closeAllBlocks();
            appendLine("}");
            insideFunction = false;
            activeLines = &bodyLines;
            lastIndent = 0;
            stackCount = 0;
            headerJustEmitted = false;
            nextCloseLine = "";
            restoreSymbolTableAfterFunction();
        }
    }
    int newIndent = effectiveIndent(newIndentRaw);
    if (headerJustEmitted) {
        if (newIndent > lastIndent) {
            openBlock(newIndent);
        }
        headerJustEmitted = false;
    }
    closeTo(newIndent);
}

void CodeGenerator::afterEmit(int newIndentRaw) {
    int newIndent = effectiveIndent(newIndentRaw);
    if (headerJustEmitted && newIndent > lastIndent) {
        openBlock(newIndent);
        headerJustEmitted = false;
    }
}

void CodeGenerator::openBlock(int indentLevel) {
    appendLine("{");
    if (stackCount < 64) {
        indentStack[stackCount] = indentLevel;
        closeLineStack[stackCount] = nextCloseLine;
        lineStack[stackCount] = activeLines;
        stackCount = stackCount + 1;
    }
    lastIndent = indentLevel;
    nextCloseLine = "";
}

void CodeGenerator::closeOneBlock() {
    if (stackCount == 0) {
        appendLine("}");
        lastIndent = 0;
        return;
    }
    LineList* target = lineStack[stackCount - 1];
    if (target == 0) {
        target = activeLines;
    }
    string extra = closeLineStack[stackCount - 1];
    stackCount = stackCount - 1;
    if ((int)extra.find("do_while|") == 0) {
        string condition = extra.substr(9);
        target->appendLine("} while (" + condition + ");");
    }
    else {
        target->appendLine("}");
        if (extra != "") {
            target->appendLine(extra);
        }
    }
    if (stackCount > 0) {
        lastIndent = indentStack[stackCount - 1];
    }
    else {
        lastIndent = 0;
    }
}

void CodeGenerator::closeTo(int targetIndent) {
    while (lastIndent > targetIndent && stackCount > 0) {
        closeOneBlock();
    }
}

void CodeGenerator::closeAllBlocks() {
    while (stackCount > 0) {
        closeOneBlock();
    }
}

string CodeGenerator::normalizeMathTokens(string text) {
    string t = helper.replaceAllSimple(text, "sumar", "+");
    t = helper.replaceAllSimple(t, "restar", "-");
    t = helper.replaceAllSimple(t, "multiplicar", "*");
    t = helper.replaceAllSimple(t, "por", "*");
    t = helper.replaceAllSimple(t, "dividir", "/");
    t = helper.replaceAllSimple(t, "entre", "/");
    t = helper.replaceAllSimple(t, " y ", " ");
    t = helper.replaceAllSimple(t, ",", " ");
    t = helper.replaceAllSimple(t, "  ", " ");
    return helper.trimSimple(t);
}

string CodeGenerator::normalizeConditionTokens(string text) {
    string t = helper.replaceAllSimple(text, "mayor o igual que", ">=");
    t = helper.replaceAllSimple(t, "menor o igual que", "<=");
    t = helper.replaceAllSimple(t, "igual a", "==");
    t = helper.replaceAllSimple(t, "diferente de", "!=");
    t = helper.replaceAllSimple(t, "mayor que", ">");
    t = helper.replaceAllSimple(t, "menor que", "<");
    t = helper.replaceAllSimple(t, " es ", " == ");
    t = helper.replaceAllSimple(t, " y ", " && ");
    t = helper.replaceAllSimple(t, " o ", " || ");
    t = helper.replaceAllSimple(t, "verdadero", "true");
    t = helper.replaceAllSimple(t, "falso", "false");
    return helper.trimSimple(t);
}

vector<string> CodeGenerator::extractMathOperands(string text) {
    string cleaned = helper.replaceAllSimple(text, "\t", " ");
    cleaned = helper.replaceAllSimple(cleaned, ",", " ");
    cleaned = helper.replaceAllSimple(cleaned, ";", " ");
    vector<string> operands;
    istringstream iss(cleaned);
    string token;
    while (iss >> token) {
        string trimmed = helper.trimSimple(token);
        if (trimmed == "") {
            continue;
        }
        string normalized = toLowerNoAccents(trimmed);
        if (normalized == "sumar" || normalized == "restar" || normalized == "multiplicar" || normalized == "dividir" ||
            normalized == "y" || normalized == "por" || normalized == "entre" || normalized == "mas") {
            continue;
        }
        while (!trimmed.empty() && (trimmed.back() == ',' || trimmed.back() == ';')) {
            trimmed.pop_back();
        }
        while (!trimmed.empty() && (trimmed.front() == ',' || trimmed.front() == ';')) {
            trimmed.erase(trimmed.begin());
        }
        trimmed = helper.trimSimple(trimmed);
        if (trimmed != "") {
            operands.push_back(trimmed);
        }
        if ((int)operands.size() >= 8) {
            break;
        }
    }
    return operands;
}

string CodeGenerator::buildMathExpression(const vector<string>& operands, const string& op) {
    if (operands.empty()) {
        return string("");
    }
    string expression;
    for (size_t i = 0; i < operands.size(); ++i) {
        if (!expression.empty()) {
            expression = expression + " " + op + " ";
        }
        expression = expression + operands[i];
    }
    return expression;
}

string CodeGenerator::removeSpaces(string text) {
    return helper.replaceAllSimple(text, " ", "");
}

bool CodeGenerator::splitAssignRight(string text, string& varName, string& value) {
    int eq = helper.indexOfText(text, "=");
    if (eq < 0) {
        return false;
    }
    string left = helper.trimSimple(text.substr(0, eq));
    string right = helper.trimSimple(text.substr(eq + 1));
    if (left == "" || right == "") {
        return false;
    }
    varName = left;
    value = right;
    return true;
}

bool CodeGenerator::parseForParts(string text, string& varName, string& startVal, string& endVal) {
    string clean = helper.trimSimple(text);
    int eq = helper.indexOfText(clean, "=");
    if (eq < 0) {
        return false;
    }
    varName = helper.trimSimple(clean.substr(0, eq));
    string rightPart = helper.trimSimple(clean.substr(eq + 1));
    istringstream iss(rightPart);
    string tokenStart;
    string tokenEnd;
    if (!(iss >> tokenStart)) {
        return false;
    }
    if (!(iss >> tokenEnd)) {
        return false;
    }
    startVal = tokenStart;
    endVal = tokenEnd;
    return true;
}

string CodeGenerator::inferTypeFromToken(string token) {
    string normalized = toLowerNoAccents(token);
    if (normalized == "true" || normalized == "false" || normalized == "verdadero" || normalized == "falso") {
        return "bool";
    }
    int lengthValue = (int)token.length();
    if (lengthValue >= 2 && token[0] == '\"' && token[lengthValue - 1] == '\"') {
        return "string";
    }
    if (lengthValue == 3 && token[0] == '\'' && token[2] == '\'') {
        return "char";
    }
    bool numeric = true;
    bool hasDot = false;
    int i = 0;
    while (i < lengthValue) {
        char ch = token[i];
        if (ch == '.') {
            hasDot = true;
        }
        else {
            if (!(ch >= '0' && ch <= '9')) {
                numeric = false;
            }
        }
        i = i + 1;
    }
    if (numeric) {
        if (hasDot) {
            return "double";
        }
        return "int";
    }
    return "int";
}

string CodeGenerator::nextArrayName() {
    if (arrayCounter == 1) {
        arrayCounter = arrayCounter + 1;
        return "lista";
    }
    ostringstream ss;
    ss << "lista" << arrayCounter;
    arrayCounter = arrayCounter + 1;
    return ss.str();
}

void CodeGenerator::restoreSymbolTableAfterFunction() {
    if (hasSavedSymbolTable) {
        symbolTable.loadFromVector(savedSymbolTable);
        hasSavedSymbolTable = false;
        savedSymbolTable.clear();
    }
}

void CodeGenerator::ensureDeclared(string varName, string typeName, string initValue) {
    if (symbolTable.variableExists(varName)) {
        return;
    }
    if (initValue == "") {
        appendLine(typeName + string(" ") + varName + string(";"));
    }
    else {
        appendLine(typeName + string(" ") + varName + string(" = ") + initValue + string(";"));
    }
    symbolTable.addOrUpdateVariable(Variable(varName, typeName, initValue));
}

void CodeGenerator::genSum(string params) {
    vector<string> operands = extractMathOperands(params);
    string expression = buildMathExpression(operands, "+");
    if (expression == "") {
        return;
    }
    ensureDeclared("total", "int", "");
    appendLine("total = " + expression + ";");
    appendLine("cout << \"El resultado es: \" << total << endl;");
}

void CodeGenerator::genSub(string params) {
    vector<string> operands = extractMathOperands(params);
    string expression = buildMathExpression(operands, "-");
    if (expression == "") {
        return;
    }
    ensureDeclared("resultadoResta", "int", "");
    appendLine("resultadoResta = " + expression + ";");
    appendLine("cout << \"El resultado es: \" << resultadoResta << endl;");
}

void CodeGenerator::genMul(string params) {
    vector<string> operands = extractMathOperands(params);
    string expression = buildMathExpression(operands, "*");
    if (expression == "") {
        return;
    }
    ensureDeclared("resultadoMultiplicacion", "int", "");
    appendLine("resultadoMultiplicacion = " + expression + ";");
    appendLine("cout << \"El resultado es: \" << resultadoMultiplicacion << endl;");
}

void CodeGenerator::genDiv(string params) {
    vector<string> operands = extractMathOperands(params);
    if (operands.empty()) {
        return;
    }
    string expression;
    string first = helper.trimSimple(operands[0]);
    string firstType = inferTypeFromToken(first);
    if (firstType == "int") {
        expression = "static_cast<double>(" + first + ")";
    }
    else {
        expression = first;
    }
    for (size_t i = 1; i < operands.size(); ++i) {
        expression = expression + " / " + operands[i];
    }
    ensureDeclared("resultadoDivision", "double", "");
    appendLine("resultadoDivision = " + expression + ";");
    appendLine("cout << \"El resultado es: \" << resultadoDivision << endl;");
}

void CodeGenerator::genCalc(string params) {
    string lowered = toLowerNoAccents(params);
    int assignPos = (int)lowered.rfind("asignar a");
    string expressionPart = params;
    string targetVar = "resultado";
    if (assignPos >= 0) {
        int adjustedAssignPos = assignPos;
        if (assignPos >= 3) {
            string before = lowered.substr(assignPos - 3, 3);
            if (before == " y " || before == " e ") {
                adjustedAssignPos = assignPos - 3;
            }
        }
        expressionPart = helper.trimSimple(params.substr(0, adjustedAssignPos));
        targetVar = helper.trimSimple(params.substr(assignPos + 9));
    }
    int comoPos = (int)toLowerNoAccents(expressionPart).rfind("como");
    if (comoPos >= 0) {
        expressionPart = helper.trimSimple(expressionPart.substr(comoPos + 4));
    }
    string normalized = normalizeBooleanWord(normalizeMathTokens(expressionPart));
    if (normalized == "") {
        return;
    }
    string inferredType = inferTypeFromToken(normalized);
    if (!symbolTable.variableExists(targetVar)) {
        ensureDeclared(targetVar, inferredType == "int" ? "double" : inferredType, "");
    }
    appendLine(targetVar + " = " + normalized + ";");
}

static pair<string, string> splitDualParams(TextHelper& helper, string params) {
    int pipePos = helper.indexOfText(params, "|");
    if (pipePos < 0) {
        return make_pair(helper.trimSimple(params), string(""));
    }
    string left = helper.trimSimple(params.substr(0, pipePos));
    string right = helper.trimSimple(params.substr(pipePos + 1));
    return make_pair(left, right);
}

void CodeGenerator::genSumAssign(string params) {
    auto parts = splitDualParams(helper, params);
    string varName = parts.first;
    string value = normalizeMathTokens(parts.second);
    if (!symbolTable.variableExists(varName)) {
        ensureDeclared(varName, inferTypeFromToken(value), "");
    }
    appendLine(varName + " = " + varName + " + " + value + ";");
}

void CodeGenerator::genSubAssign(string params) {
    auto parts = splitDualParams(helper, params);
    string varName = parts.first;
    string value = normalizeMathTokens(parts.second);
    if (!symbolTable.variableExists(varName)) {
        ensureDeclared(varName, inferTypeFromToken(value), "");
    }
    appendLine(varName + " = " + varName + " - " + value + ";");
}

void CodeGenerator::genMulAssign(string params) {
    auto parts = splitDualParams(helper, params);
    string varName = parts.first;
    string value = normalizeMathTokens(parts.second);
    if (!symbolTable.variableExists(varName)) {
        ensureDeclared(varName, inferTypeFromToken(value), "");
    }
    appendLine(varName + " = " + varName + " * " + value + ";");
}

void CodeGenerator::genDivAssign(string params) {
    auto parts = splitDualParams(helper, params);
    string varName = parts.first;
    string value = normalizeMathTokens(parts.second);
    if (!symbolTable.variableExists(varName)) {
        ensureDeclared(varName, "double", "");
    }
    appendLine(varName + " = " + varName + " / " + value + ";");
}

void CodeGenerator::genPrint(string params) {
    vector<string> segments = splitPrintSegments(params);
    if (segments.empty()) {
        appendLine("cout << endl;");
        return;
    }
    string expression = "";
    for (auto& seg : segments) {
        if (seg == "") {
            continue;
        }
        string normalized = normalizeBooleanWord(seg);
        if (expression != "") {
            expression = expression + " << ";
        }
        expression = expression + normalized;
    }
    appendLine("cout << " + expression + " << endl;");
}

void CodeGenerator::genMessage(string params) {
    string textValue = helper.trimSimple(params);
    if (textValue == "") {
        appendLine("cout << \"\" << endl;");
        return;
    }
    int n = (int)textValue.length();
    if (n >= 2 && textValue[0] == '\"' && textValue[n - 1] == '\"') {
        appendLine("cout << " + textValue + " << endl;");
        return;
    }
    appendLine("cout << \"" + textValue + "\" << endl;");
}

void CodeGenerator::genRead(string params) {
    string trimmed = helper.trimSimple(params);
    string lowered = toLowerNoAccents(trimmed);
    if (lowered.find("cada") != string::npos || lowered.find("todos") != string::npos || lowered.find("lista") != string::npos || lowered.find("elemento") != string::npos) {
        ArrayInfo info = resolveArrayForText(trimmed);
        string indexName = selectIndexName("i");
        string sizeExpr = arraySizeExpression(info);
        appendLine("for (int " + indexName + " = 0; " + indexName + " < " + sizeExpr + "; " + indexName + "++)");
        appendLine("{");
        if (structTable.find(info.elementType) != structTable.end()) {
            StructInfo sinfo = structTable[info.elementType];
            for (auto& field : sinfo.fields) {
                string promptLine = string("    cout << \\\"Ingrese ") + field.first + " del " + info.elementType +
                    " \\\" << " + indexName + " << \": \\\";";
                appendLine(promptLine);
                appendLine("    cin >> " + info.name + "[" + indexName + "]." + field.first + ";");
            }
        }
        else {
            appendLine("    cin >> " + info.name + "[" + indexName + "];");
        }
        appendLine("}");
        lastLoopArray = info.name;
        return;
    }
    string varName = trimmed;
    if (varName == "") {
        return;
    }
    string normalizedVar = normalizeBooleanWord(varName);
    if (symbolTable.variableExists(normalizedVar)) {
        Variable var = symbolTable.getVariable(normalizedVar);
        appendLine("cin >> " + var.getName() + ";");
        return;
    }
    ensureDeclared(normalizedVar, "int", "0");
    appendLine("cin >> " + normalizedVar + ";");
}

void CodeGenerator::genIf(string params) {
    string condition = normalizeConditionTokens(params);
    appendLine("if (" + condition + ")");
    headerJustEmitted = true;
    nextCloseLine = "";
}

void CodeGenerator::genElse() {
    appendLine("else");
    headerJustEmitted = true;
    nextCloseLine = "";
}

void CodeGenerator::genWhile(string params) {
    string condition = normalizeConditionTokens(params);
    appendLine("while (" + condition + ")");
    headerJustEmitted = true;
    nextCloseLine = "";
}

void CodeGenerator::genDoUntil(string params) {
    string condition = normalizeConditionTokens(params);
    appendLine("do");
    headerJustEmitted = true;
    nextCloseLine = "do_while|!(" + condition + ")";
}

void CodeGenerator::genForTo(string params) {
    string text = helper.trimSimple(params);
    int h = helper.indexOfText(text, "hasta");
    if (h >= 0) {
        string left = helper.trimSimple(text.substr(0, h));
        string right = helper.trimSimple(text.substr(h + 5));
        text = left + string(" ") + right;
    }
    string varName = "";
    string startVal = "";
    string endVal = "";
    bool ok = parseForParts(text, varName, startVal, endVal);
    if (!ok) {
        return;
    }
    appendLine("for (int " + varName + " = " + startVal + "; " + varName + " <= " + endVal + "; " + varName + "++)");
    headerJustEmitted = true;
    nextCloseLine = "";
}

static string normalizeInitialValue(TextHelper& helper, const string& value) {
    string trimmed = helper.trimSimple(value);
    string lowered = helper.removeAccents(helper.toLowerSimple(trimmed));
    if (lowered == "falso") { return "false"; }
    if (lowered == "verdadero") { return "true"; }
    if (lowered == "nulo" || lowered == "null") { return "0"; }
    return trimmed;
}

static string cleanIdentifierToken(TextHelper& helper, string token) {
    string cleaned = helper.trimSimple(token);
    while (!cleaned.empty() && (cleaned.back() == ',' || cleaned.back() == ';' || cleaned.back() == ')')) {
        cleaned.pop_back();
    }
    while (!cleaned.empty() && cleaned.front() == '(') {
        cleaned.erase(cleaned.begin());
    }
    cleaned = helper.trimSimple(cleaned);
    while (!cleaned.empty() && (cleaned.front() == ',')) {
        cleaned.erase(cleaned.begin());
    }
    cleaned = helper.trimSimple(cleaned);
    return cleaned;
}

static string detectTypeFromTokens(TextHelper& helper, const vector<string>& tokens, const vector<string>& normalized, int startIndex, int& nextIndex) {
    int total = (int)normalized.size();
    if (startIndex < 0 || startIndex >= total) {
        nextIndex = startIndex;
        return string("");
    }
    string current = normalized[startIndex];
    auto setNext = [&](int value) {
        nextIndex = value;
    };
    if (current == "numero") {
        if (startIndex + 1 < total) {
            string second = normalized[startIndex + 1];
            if (second == "entero" || second == "integer") {
                setNext(startIndex + 2);
                return "int";
            }
            if (second == "decimal" || second == "real" || second == "doble" || second == "double" || second == "float") {
                setNext(startIndex + 2);
                return "double";
            }
            if (second == "booleano" || second == "bool" || second == "logico") {
                setNext(startIndex + 2);
                return "bool";
            }
            if (second == "texto" || second == "string" || second == "cadena") {
                setNext(startIndex + 2);
                return "string";
            }
            if (second == "caracter" || second == "char") {
                setNext(startIndex + 2);
                return "char";
            }
        }
        setNext(startIndex + 1);
        return "int";
    }
    if (current == "entero" || current == "int") {
        setNext(startIndex + 1);
        return "int";
    }
    if (current == "decimal" || current == "double" || current == "doble" || current == "real" || current == "float") {
        setNext(startIndex + 1);
        return "double";
    }
    if (current == "booleano" || current == "bool" || current == "logico") {
        setNext(startIndex + 1);
        return "bool";
    }
    if (current == "texto" || current == "string" || current == "palabra") {
        setNext(startIndex + 1);
        return "string";
    }
    if (current == "cadena") {
        if (startIndex + 2 < total && normalized[startIndex + 1] == "de" && normalized[startIndex + 2] == "texto") {
            setNext(startIndex + 3);
            return "string";
        }
        setNext(startIndex + 1);
        return "string";
    }
    if (current == "caracter" || current == "char") {
        setNext(startIndex + 1);
        return "char";
    }
    if (current == "void" || current == "vacio" || current == "vacia" || current == "nada" || current == "procedimiento") {
        setNext(startIndex + 1);
        return "void";
    }
    nextIndex = startIndex;
    return string("");
}

void CodeGenerator::genCreateVar(string params) {
    string text = replaceUtf8Accents(helper.trimSimple(params));
    string typeName = "int";
    string varName = "";
    string initValue = "";
    int withPos = helper.indexOfText(text, "con valor");
    if (withPos >= 0) {
        string left = helper.trimSimple(text.substr(0, withPos));
        string right = helper.trimSimple(text.substr(withPos + 9));
        text = left;
        initValue = normalizeInitialValue(helper, right);
        if (initValue != "") {
            string initNormalized = helper.removeAccents(helper.toLowerSimple(initValue));
            if ((int)initNormalized.find("inicial") == 0) {
                int offset = 7;
                if ((int)initValue.length() > offset) {
                    initValue = helper.trimSimple(initValue.substr(offset));
                }
                else {
                    initValue = "";
                }
            }
        }
    }
    vector<string> tokens;
    vector<string> normalized;
    istringstream iss(text);
    string token;
    while (iss >> token) {
        tokens.push_back(token);
        string lowered = helper.toLowerSimple(token);
        normalized.push_back(helper.removeAccents(lowered));
    }
    int nameIndex = -1;
    if (!tokens.empty()) {
        string firstNorm = normalized[0];
        if (firstNorm == "entero" || firstNorm == "int") {
            typeName = "int";
            nameIndex = 1;
        }
        else if (firstNorm == "numero") {
            if ((int)normalized.size() >= 2) {
                string secondNorm = normalized[1];
                if (secondNorm == "entero") {
                    typeName = "int";
                    nameIndex = 2;
                }
                else if (secondNorm == "decimal" || secondNorm == "real" || secondNorm == "doble") {
                    typeName = "double";
                    nameIndex = 2;
                }
                else if (secondNorm == "booleano" || secondNorm == "bool") {
                    typeName = "bool";
                    nameIndex = 2;
                }
                else if (secondNorm == "texto") {
                    typeName = "string";
                    nameIndex = 2;
                }
                else if (secondNorm == "caracter") {
                    typeName = "char";
                    nameIndex = 2;
                }
                else {
                    typeName = "int";
                    nameIndex = 1;
                }
            }
            else {
                typeName = "int";
                nameIndex = 1;
            }
        }
        else if (firstNorm == "decimal" || firstNorm == "double" || firstNorm == "float" || firstNorm == "real") {
            typeName = "double";
            nameIndex = 1;
        }
        else if (firstNorm == "booleano" || firstNorm == "bool") {
            typeName = "bool";
            nameIndex = 1;
        }
        else if (firstNorm == "texto" || firstNorm == "string" || firstNorm == "palabra") {
            typeName = "string";
            nameIndex = 1;
        }
        else if (firstNorm == "cadena") {
            if ((int)normalized.size() >= 3 && normalized[1] == "de" && normalized[2] == "texto") {
                typeName = "string";
                nameIndex = 3;
            }
            else {
                typeName = "string";
                nameIndex = 1;
            }
        }
        else if (firstNorm == "caracter" || firstNorm == "char") {
            typeName = "char";
            nameIndex = 1;
        }
        else {
            nameIndex = 0;
        }
    }
    if (nameIndex >= 0 && nameIndex < (int)tokens.size()) {
        varName = tokens[nameIndex];
    }
    if (varName == "" && !tokens.empty()) {
        varName = tokens.back();
    }
    if (varName == "") {
        varName = "var1";
    }
    if (initValue != "") {
        initValue = normalizeBooleanWord(initValue);
    }
    ensureDeclared(varName, typeName, initValue);
}

void CodeGenerator::genAssign(string params) {
    string text = replaceUtf8Accents(helper.trimSimple(params));
    string varName = "";
    string value = "";
    string normalized = toLowerNoAccents(text);
    if ((int)normalized.rfind("a ", 0) == 0) {
        string afterA = helper.trimSimple(text.substr(1));
        int posCon = helper.indexOfText(afterA, " con ");
        if (posCon >= 0) {
            varName = helper.trimSimple(afterA.substr(0, posCon));
            value = helper.trimSimple(afterA.substr(posCon + 5));
        }
        else {
            int eq = helper.indexOfText(afterA, "=");
            if (eq >= 0) {
                varName = helper.trimSimple(afterA.substr(0, eq));
                value = helper.trimSimple(afterA.substr(eq + 1));
            }
        }
    }
    if (varName == "") {
        int posA = helper.indexOfText(text, " a ");
        if (posA >= 0) {
            value = helper.trimSimple(text.substr(0, posA));
            varName = helper.trimSimple(text.substr(posA + 3));
        }
    }
    if (varName == "" || value == "") {
        int posCon = helper.indexOfText(text, " con ");
        if (posCon >= 0) {
            string left = helper.trimSimple(text.substr(0, posCon));
            string right = helper.trimSimple(text.substr(posCon + 5));
            string normalizedLeft = toLowerNoAccents(left);
            if ((int)normalizedLeft.rfind("a ", 0) == 0) {
                varName = helper.trimSimple(left.substr(1));
                value = right;
            }
        }
    }
    if (varName == "" || value == "") {
        bool ok = splitAssignRight(text, varName, value);
        if (!ok) {
            return;
        }
    }
    string normalizedValue = toLowerNoAccents(value);
    if ((int)normalizedValue.rfind("llamar funcion", 0) == 0) {
        int offset = (int)string("llamar funcion").length();
        if ((int)value.length() >= offset) {
            value = helper.trimSimple(value.substr(offset));
        }
    }
    value = normalizeBooleanWord(normalizeMathTokens(value));
    if (!symbolTable.variableExists(varName)) {
        string inferred = inferTypeFromToken(value);
        ensureDeclared(varName, inferred, value);
        return;
    }
    appendLine(varName + " = " + value + ";");
    symbolTable.assignValue(varName, value);
}

void CodeGenerator::genCreateArray(string params) {
    string text = helper.trimSimple(params);
    string arrayName = "";
    string typeName = "int";
    int sizeValue = 0;
    for (auto& entry : structTable) {
        string loweredName = toLowerNoAccents(entry.first);
        if ((int)toLowerNoAccents(text).find(loweredName) >= 0) {
            typeName = entry.first;
            break;
        }
    }
    if ((int)text.find("string") >= 0 || (int)text.find("texto") >= 0 || (int)text.find("cadena") >= 0) {
        typeName = "string";
    }
    else if ((int)text.find("double") >= 0 || (int)text.find("decimal") >= 0 || (int)text.find("float") >= 0) {
        typeName = "double";
    }
    else if ((int)text.find("char") >= 0 || (int)text.find("caracter") >= 0) {
        typeName = "char";
    }
    else if ((int)text.find("bool") >= 0 || (int)text.find("booleano") >= 0) {
        typeName = "bool";
    }
    int posNamed = helper.indexOfText(text, "llamada");
    if (posNamed >= 0) {
        string after = helper.trimSimple(text.substr(posNamed + 7));
        istringstream ns(after);
        ns >> arrayName;
    }
    int posCon = helper.indexOfText(text, "con");
    int posElem = helper.indexOfText(text, "elemento");
    if (posCon >= 0 && posElem > posCon) {
        string mid = helper.trimSimple(text.substr(posCon + 3, posElem - (posCon + 3)));
        istringstream s(mid);
        int n = 0;
        s >> n;
        if (n > 0) {
            sizeValue = n;
        }
    }
    if (arrayName == "") {
        arrayName = nextArrayName();
    }
    if (sizeValue <= 0) {
        sizeValue = 1;
    }
    ostringstream os;
    os << sizeValue;
    appendLine(typeName + string(" ") + arrayName + string("[") + os.str() + string("] = {};"));
    ArrayInfo info;
    info.name = arrayName;
    info.elementType = typeName;
    info.size = sizeValue;
    registerArray(info);
}

void CodeGenerator::genFillArray(string params) {
    string trimmed = helper.trimSimple(params);
    ArrayInfo info = resolveArrayForText(trimmed);
    string sizeExpr = arraySizeExpression(info);
    string indexName = selectIndexName("i");
    appendLine("for (int " + indexName + " = 0; " + indexName + " < " + sizeExpr + "; " + indexName + "++)");
    appendLine("{");
    auto structIt = structTable.find(info.elementType);
    if (structIt != structTable.end()) {
        const StructInfo& sinfo = structIt->second;
        for (size_t k = 0; k < sinfo.fields.size(); ++k) {
            appendLine("    cin >> " + info.name + "[" + indexName + "]." + sinfo.fields[k].first + ";");
        }
    }
    else {
        appendLine("    cin >> " + info.name + "[" + indexName + "];");
    }
    appendLine("}");
    lastLoopArray = info.name;
}

void CodeGenerator::genTraverseList(string params) {
    string trimmed = helper.trimSimple(params);
    ArrayInfo info = resolveArrayForText(trimmed);
    string sizeExpr = arraySizeExpression(info);
    string indexName = selectIndexName("i");
    string lowered = toLowerNoAccents(trimmed);
    if ((int)lowered.find("sumar") >= 0 && (int)lowered.find("total") >= 0) {
        if (!symbolTable.variableExists("total")) {
            ensureDeclared("total", info.elementType == "double" ? "double" : "int", "0");
        }
        appendLine("for (int " + indexName + " = 0; " + indexName + " < " + sizeExpr + "; " + indexName + "++)");
        appendLine("{");
        appendLine("    total += " + info.name + "[" + indexName + "];");
        appendLine("}");
        lastLoopArray = info.name;
        return;
    }
    if ((int)lowered.find("mostrar") >= 0 && structTable.find(info.elementType) != structTable.end()) {
        StructInfo sinfo = structTable[info.elementType];
        vector<string> fieldsToShow;
        for (auto& field : sinfo.fields) {
            if ((int)lowered.find(toLowerNoAccents(field.first)) >= 0) {
                fieldsToShow.push_back(field.first);
            }
        }
        if (fieldsToShow.empty()) {
            for (auto& field : sinfo.fields) {
                fieldsToShow.push_back(field.first);
            }
        }
        appendLine("for (int " + indexName + " = 0; " + indexName + " < " + sizeExpr + "; " + indexName + "++)");
        appendLine("{");
        string expr = "";
        for (auto& field : fieldsToShow) {
            if (expr != "") {
                expr = expr + " << \" \" << ";
            }
            expr = expr + info.name + "[" + indexName + "]." + field;
        }
        appendLine("    cout << " + expr + " << endl;");
        appendLine("}");
        return;
    }
    appendLine("for (int " + indexName + " = 0; " + indexName + " < " + sizeExpr + "; " + indexName + "++)");
    headerJustEmitted = true;
    nextCloseLine = "";
    lastLoopArray = info.name;
}

void CodeGenerator::genAddToList(string params) {
    (void)params;
}

void CodeGenerator::genCreateStruct(string params) {
    string text = helper.trimSimple(params);
    int posCon = helper.indexOfText(text, "con");
    string structName = helper.trimSimple(posCon >= 0 ? text.substr(0, posCon) : text);
    string fieldsPart = posCon >= 0 ? helper.trimSimple(text.substr(posCon + 3)) : "";
    StructInfo info;
    info.name = structName;
    string replaced = helper.replaceAllSimple(fieldsPart, " y ", ",");
    istringstream items(replaced);
    string item;
    while (getline(items, item, ',')) {
        string trimmedItem = helper.trimSimple(item);
        if (trimmedItem == "") { continue; }
        string fieldName = item;
        string typeText = "int";
        int open = helper.indexOfText(trimmedItem, "(");
        int close = helper.indexOfText(trimmedItem, ")");
        if (open >= 0 && close > open) {
            fieldName = helper.trimSimple(trimmedItem.substr(0, open));
            typeText = helper.trimSimple(trimmedItem.substr(open + 1, close - open - 1));
        }
        else {
            fieldName = helper.trimSimple(trimmedItem);
        }
        string normalizedType = toLowerNoAccents(typeText);
        string cppType = "int";
        if (normalizedType.find("decimal") != string::npos || normalizedType.find("real") != string::npos || normalizedType.find("double") != string::npos) {
            cppType = "double";
        }
        else if (normalizedType.find("texto") != string::npos || normalizedType.find("cadena") != string::npos || normalizedType.find("string") != string::npos) {
            cppType = "string";
        }
        else if (normalizedType.find("bool") != string::npos || normalizedType.find("booleano") != string::npos) {
            cppType = "bool";
        }
        else if (normalizedType.find("char") != string::npos || normalizedType.find("caracter") != string::npos) {
            cppType = "char";
        }
        info.fields.push_back(make_pair(helper.trimSimple(fieldName), cppType));
    }
    appendToPrelude("struct " + structName);
    appendToPrelude("{");
    for (auto& field : info.fields) {
        appendToPrelude("    " + field.second + " " + field.first + ";");
    }
    appendToPrelude("};");
    registerStruct(info);
}

void CodeGenerator::genReturn(string params) {
    string expr = helper.trimSimple(params);
    expr = normalizeBooleanWord(normalizeMathTokens(expr));
    if (expr == "") {
        appendLine("return;");
        return;
    }
    appendLine("return " + expr + ";");
}

void CodeGenerator::genBeginProgram() {
}

void CodeGenerator::genEndProgram() {
}

void CodeGenerator::genComment(string params) {
    appendLine("// " + params);
}

void CodeGenerator::genDefineFunction(string params) {
    string rawSignature = helper.trimSimple(params);
    string asciiSignature = replaceUtf8Accents(rawSignature);
    string normalizedSignature = helper.toLowerSimple(asciiSignature);
    vector<string> paramMarkers = {
        "con parametro", "con parametros", "con el parametro", "con los parametros",
        "con argumento", "con argumentos", "con el argumento", "con los argumentos"
    };
    int paramsPos = -1;
    int markerLength = 0;
    for (const auto& marker : paramMarkers) {
        int pos = helper.indexOfText(normalizedSignature, marker);
        if (pos >= 0) {
            paramsPos = pos;
            markerLength = (int)marker.length();
            break;
        }
    }
    string beforePart = asciiSignature;
    string paramsPart = "";
    if (paramsPos >= 0) {
        beforePart = helper.trimSimple(asciiSignature.substr(0, paramsPos));
        paramsPart = helper.trimSimple(asciiSignature.substr(paramsPos + markerLength));
    }
    vector<string> tokens;
    vector<string> normalized;
    istringstream iss(beforePart);
    string token;
    while (iss >> token) {
        string cleaned = cleanIdentifierToken(helper, token);
        if (cleaned == "") {
            continue;
        }
        tokens.push_back(cleaned);
        string lowered = helper.toLowerSimple(cleaned);
        normalized.push_back(helper.removeAccents(lowered));
    }

    string returnType = "void";
    int indexName = 0;
    if (!tokens.empty()) {
        int nextIndex = 0;
        string typeCandidate = detectTypeFromTokens(helper, tokens, normalized, 0, nextIndex);
        if (typeCandidate != "") {
            returnType = typeCandidate;
            indexName = nextIndex;
        }
    }

    while (indexName < (int)normalized.size() && (normalized[indexName] == "funcion" || normalized[indexName] == "function")) {
        indexName = indexName + 1;
    }

    string functionName = "funcion";
    if (indexName < (int)tokens.size()) {
        functionName = cleanIdentifierToken(helper, tokens[indexName]);
        if (functionName == "") {
            functionName = "funcion";
        }
    }

    vector<pair<string, string>> parameters;
    string normalizedParams = toLowerNoAccents(paramsPart);
    if (normalizedParams != "" && normalizedParams.find("sin parametro") == string::npos && normalizedParams.find("sin argumento") == string::npos) {
        vector<string> paramTokens;
        vector<string> paramNormalized;
        istringstream pss(paramsPart);
        string ptoken;
        while (pss >> ptoken) {
            string cleaned = cleanIdentifierToken(helper, ptoken);
            if (cleaned == "") {
                continue;
            }
            paramTokens.push_back(cleaned);
            string lowered = helper.toLowerSimple(cleaned);
            paramNormalized.push_back(helper.removeAccents(lowered));
        }
        int i = 0;
        int paramIndex = 1;
        auto findStructTypeForToken = [&](const string& token) -> string {
            string normalizedToken = helper.removeAccents(helper.toLowerSimple(token));
            for (auto& entry : structTable) {
                string normalizedName = helper.removeAccents(helper.toLowerSimple(entry.first));
                if (normalizedToken == normalizedName) {
                    return entry.first;
                }
            }
            return string("");
        };
        while (i < (int)paramTokens.size()) {
            string norm = paramNormalized[i];
            if (norm == "" || norm == "y" || norm == "e" || norm == "con" || norm == "parametro" || norm == "parametros" ||
                norm == "argumento" || norm == "argumentos" || norm == "el" || norm == "la" || norm == "los" || norm == "las" ||
                norm == "de" || norm == "del") {
                i = i + 1;
                continue;
            }
            int nextIndex = i;
            string typeCandidate = detectTypeFromTokens(helper, paramTokens, paramNormalized, i, nextIndex);
            if (typeCandidate == "") {
                string structType = findStructTypeForToken(paramTokens[i]);
                if (structType != "") {
                    typeCandidate = structType;
                    nextIndex = i + 1;
                }
            }
            if (typeCandidate != "") {
                string paramName = "";
                if (nextIndex < (int)paramTokens.size()) {
                    paramName = cleanIdentifierToken(helper, paramTokens[nextIndex]);
                    nextIndex = nextIndex + 1;
                }
                if (paramName == "") {
                    ostringstream generated;
                    generated << "param" << paramIndex;
                    paramName = generated.str();
                }
                parameters.push_back(make_pair(typeCandidate, paramName));
                i = nextIndex;
                paramIndex = paramIndex + 1;
                continue;
            }
            string structType = findStructTypeForToken(paramTokens[i]);
            if (structType != "") {
                string paramName = "";
                if (i + 1 < (int)paramTokens.size()) {
                    paramName = cleanIdentifierToken(helper, paramTokens[i + 1]);
                    i = i + 2;
                }
                else {
                    ostringstream generated;
                    generated << "param" << paramIndex;
                    paramName = generated.str();
                    i = i + 1;
                }
                if (paramName == "") {
                    ostringstream generated;
                    generated << "param" << paramIndex;
                    paramName = generated.str();
                }
                parameters.push_back(make_pair(structType, paramName));
                paramIndex = paramIndex + 1;
                continue;
            }
            string paramName = cleanIdentifierToken(helper, paramTokens[i]);
            if (paramName == "") {
                ostringstream generated;
                generated << "param" << paramIndex;
                paramName = generated.str();
            }
            parameters.push_back(make_pair("int", paramName));
            i = i + 1;
            paramIndex = paramIndex + 1;
        }
    }

    ostringstream signature;
    signature << returnType << " " << functionName << "(";
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) {
            signature << ", ";
        }
        signature << parameters[i].first << " " << parameters[i].second;
    }
    signature << ")";

    if (!hasSavedSymbolTable) {
        savedSymbolTable = symbolTable.toVector();
        hasSavedSymbolTable = true;
    }
    symbolTable.clearList();
    for (auto& param : parameters) {
        symbolTable.addOrUpdateVariable(Variable(param.second, param.first, ""));
    }

    appendToPrelude(signature.str() + " {");
    activeLines = &preludeLines;
    insideFunction = true;
    lastIndent = 0;
    stackCount = 0;
    headerJustEmitted = false;
    nextCloseLine = "";
}

void CodeGenerator::genCallFunction(string params) {
    string callName = helper.trimSimple(params);
    if (callName == "") {
        appendLine("funcion();");
        return;
    }
    int open = helper.indexOfText(callName, "(");
    if (open < 0) {
        callName = callName + "()";
    }
    appendLine(callName + ";");
}

void CodeGenerator::generate(Instruction ins) {
    int indentLevelRaw = ins.getIndent();
    beforeEmit(indentLevelRaw);
    string op = ins.getOperation();
    string params = helper.trimSimple(ins.getParameters());
    if (op == "sum") { genSum(params); afterEmit(indentLevelRaw); return; }
    if (op == "sub") { genSub(params); afterEmit(indentLevelRaw); return; }
    if (op == "mul") { genMul(params); afterEmit(indentLevelRaw); return; }
    if (op == "div") { genDiv(params); afterEmit(indentLevelRaw); return; }
    if (op == "calc") { genCalc(params); afterEmit(indentLevelRaw); return; }
    if (op == "sum_assign") { genSumAssign(params); afterEmit(indentLevelRaw); return; }
    if (op == "sub_assign") { genSubAssign(params); afterEmit(indentLevelRaw); return; }
    if (op == "mul_assign") { genMulAssign(params); afterEmit(indentLevelRaw); return; }
    if (op == "div_assign") { genDivAssign(params); afterEmit(indentLevelRaw); return; }
    if (op == "print") { genPrint(params); afterEmit(indentLevelRaw); return; }
    if (op == "message") { genMessage(params); afterEmit(indentLevelRaw); return; }
    if (op == "read") { genRead(params); afterEmit(indentLevelRaw); return; }
    if (op == "if") { genIf(params); afterEmit(indentLevelRaw); return; }
    if (op == "else") { genElse(); afterEmit(indentLevelRaw); return; }
    if (op == "while") { genWhile(params); afterEmit(indentLevelRaw); return; }
    if (op == "do_until") { genDoUntil(params); afterEmit(indentLevelRaw); return; }
    if (op == "for_to") { genForTo(params); afterEmit(indentLevelRaw); return; }
    if (op == "create_var") { genCreateVar(params); afterEmit(indentLevelRaw); return; }
    if (op == "assign") { genAssign(params); afterEmit(indentLevelRaw); return; }
    if (op == "create_array") { genCreateArray(params); afterEmit(indentLevelRaw); return; }
    if (op == "fill_array") { genFillArray(params); afterEmit(indentLevelRaw); return; }
    if (op == "traverse_list") { genTraverseList(params); afterEmit(indentLevelRaw); return; }
    if (op == "add_to_list") { genAddToList(params); afterEmit(indentLevelRaw); return; }
    if (op == "begin_program") { genBeginProgram(); afterEmit(indentLevelRaw); return; }
    if (op == "end_program") { genEndProgram(); afterEmit(indentLevelRaw); return; }
    if (op == "comment") { genComment(params); afterEmit(indentLevelRaw); return; }
    if (op == "def_func") {
        functionIndent = indentLevelRaw;
        functionBodyIndent = indentLevelRaw + 4;
        genDefineFunction(params);
        afterEmit(indentLevelRaw);
        return;
    }
    if (op == "call_func") { genCallFunction(params); afterEmit(indentLevelRaw); return; }
    if (op == "create_struct") { genCreateStruct(params); afterEmit(indentLevelRaw); return; }
    if (op == "return") { genReturn(params); afterEmit(indentLevelRaw); return; }
    appendLine("cout << \"Instruccion no reconocida\" << endl;");
    afterEmit(indentLevelRaw);
}

string CodeGenerator::buildProgram() {
    closeAllBlocks();
    if (insideFunction) {
        appendLine("}");
        insideFunction = false;
        restoreSymbolTableAfterFunction();
    }
    string out = "";
    out = out + "#include <iostream>\n";
    out = out + "#include <string>\n";
    out = out + "using namespace std;\n\n";
    string pre = preludeLines.joinAll();
    if (pre != "") { out = out + pre + "\n\n"; }
    out = out + "int main()\n";
    out = out + "{\n";
    string body = bodyLines.joinAll();
    out = out + body;
    if (body != "" && body[(int)body.length() - 1] != '\n') { out = out + "\n"; }
    out = out + "return 0;\n";
    out = out + "}\n";
    return out;
}

void CodeGenerator::clearAll() {
    symbolTable.clearList();
    preludeLines.clearAll();
    bodyLines.clearAll();
    arrayTable.clear();
    structTable.clear();
    hasLastArray = false;
    lastLoopIndex = "";
    lastLoopArray = "";
    arrayCounter = 1;
    lastIndent = 0;
    stackCount = 0;
    headerJustEmitted = false;
    nextCloseLine = "";
    activeLines = &bodyLines;
    insideFunction = false;
    functionIndent = 0;
    functionBodyIndent = 0;
    savedSymbolTable.clear();
    hasSavedSymbolTable = false;
}
