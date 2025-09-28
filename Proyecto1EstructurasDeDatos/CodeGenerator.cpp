#include "CodeGenerator.h"
#include <sstream>
#include <vector>

CodeGenerator::CodeGenerator() {
    arrayCounter = 1;
    lastIndent = 0;
    stackCount = 0;
    headerJustEmitted = false;
    nextCloseLine = "";
}

void CodeGenerator::beforeEmit(int newIndent) {
    closeTo(newIndent);
}

void CodeGenerator::afterEmit(int newIndent) {
    if (headerJustEmitted && newIndent > lastIndent) {
        openBlock(newIndent);
        headerJustEmitted = false;
        return;
    }
    headerJustEmitted = false;
}

void CodeGenerator::openBlock(int indentLevel) {
    bodyLines.appendLine("{");
    if (stackCount < 64) {
        indentStack[stackCount] = indentLevel;
        closeLineStack[stackCount] = nextCloseLine;
        stackCount = stackCount + 1;
    }
    lastIndent = indentLevel;
    nextCloseLine = "";
}

void CodeGenerator::closeOneBlock() {
    bodyLines.appendLine("}");
    if (stackCount > 0) {
        string extra = closeLineStack[stackCount - 1];
        stackCount = stackCount - 1;
        if (extra != "") {
            bodyLines.appendLine(extra);
        }
        if (stackCount > 0) {
            lastIndent = indentStack[stackCount - 1];
        }
        else {
            lastIndent = 0;
        }
    }
    else {
        lastIndent = 0;
    }
}

void CodeGenerator::closeTo(int targetIndent) {
    while (lastIndent > targetIndent) {
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
    string t = helper.replaceAllSimple(text, "igual a", "==");
    t = helper.replaceAllSimple(t, "diferente de", "!=");
    t = helper.replaceAllSimple(t, "mayor que", ">");
    t = helper.replaceAllSimple(t, "menor que", "<");
    t = helper.replaceAllSimple(t, " y ", " && ");
    t = helper.replaceAllSimple(t, " o ", " || ");
    return helper.trimSimple(t);
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
    int lengthValue = (int)token.length();
    if (token == "true" || token == "false") {
        return "bool";
    }
    if (lengthValue >= 2 && token[0] == '"' && token[lengthValue - 1] == '"') {
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
    ostringstream ss;
    ss << "lista" << arrayCounter;
    arrayCounter = arrayCounter + 1;
    return ss.str();
}

void CodeGenerator::ensureDeclared(string varName, string typeName, string initValue) {
    if (symbolTable.variableExists(varName)) {
        return;
    }
    if (initValue == "") {
        bodyLines.appendLine(typeName + string(" ") + varName + string(";"));
    }
    else {
        bodyLines.appendLine(typeName + string(" ") + varName + string(" = ") + initValue + string(";"));
    }
    symbolTable.addOrUpdateVariable(Variable(varName, typeName, initValue));
}

void CodeGenerator::genSum(string params) {
    string normalized = normalizeMathTokens(params);
    istringstream iss(normalized);
    string token;
    string expression = "";
    int countTokens = 0;

    while (iss >> token) {
        if (expression != "") {
            expression = expression + " + ";
        }
        expression = expression + token;
        countTokens = countTokens + 1;
        if (countTokens >= 6) {
            break;
        }
    }

    ensureDeclared("total", "int", "");
    bodyLines.appendLine("total = " + expression + ";");
    bodyLines.appendLine("cout << \"El resultado es: \" << total << endl;");
}

void CodeGenerator::genSub(string params) {
    string normalized = normalizeMathTokens(params);
    istringstream iss(normalized);
    string token;
    string expression = "";
    int countTokens = 0;

    while (iss >> token) {
        if (expression != "") {
            expression = expression + " - ";
        }
        expression = expression + token;
        countTokens = countTokens + 1;
        if (countTokens >= 6) {
            break;
        }
    }

    bodyLines.appendLine("int resultado = " + expression + ";");
    bodyLines.appendLine("cout << \"El resultado es: \" << resultado << endl;");
}

void CodeGenerator::genMul(string params) {
    string normalized = normalizeMathTokens(params);
    istringstream iss(normalized);
    string token;
    string expression = "";
    int countTokens = 0;

    while (iss >> token) {
        if (expression != "") {
            expression = expression + " * ";
        }
        expression = expression + token;
        countTokens = countTokens + 1;
        if (countTokens >= 6) {
            break;
        }
    }

    bodyLines.appendLine("int resultado = " + expression + ";");
    bodyLines.appendLine("cout << \"El resultado es: \" << resultado << endl;");
}

void CodeGenerator::genDiv(string params) {
    string normalized = normalizeMathTokens(params);
    istringstream iss(normalized);
    string token;
    string expression = "";
    int countTokens = 0;

    while (iss >> token) {
        if (expression != "") {
            expression = expression + " / ";
        }
        expression = expression + token;
        countTokens = countTokens + 1;
        if (countTokens >= 6) {
            break;
        }
    }

    bodyLines.appendLine("double resultado = " + expression + ";");
    bodyLines.appendLine("cout << \"El resultado es: \" << resultado << endl;");
}

void CodeGenerator::genCalc(string params) {
    string normalized = normalizeMathTokens(params);
    if (normalized == "") {
        bodyLines.appendLine("// calcular: sin expresion");
        return;
    }
    bodyLines.appendLine("int resultado = " + normalized + ";");
    bodyLines.appendLine("cout << \"El resultado es: \" << resultado << endl;");
}

void CodeGenerator::genPrint(string params) {
    bodyLines.appendLine("cout << " + params + " << endl;");
}

void CodeGenerator::genMessage(string params) {
    string textValue = helper.trimSimple(params);
    if (textValue == "") {
        bodyLines.appendLine("cout << \"\" << endl;");
        return;
    }
    int n = (int)textValue.length();
    if (textValue[0] == '"' && textValue[n - 1] == '"') {
        bodyLines.appendLine("cout << " + textValue + " << endl;");
        return;
    }
    bodyLines.appendLine("cout << \"" + textValue + "\" << endl;");
}

void CodeGenerator::genRead(string params) {
    string varName = helper.trimSimple(params);
    if (varName == "") {
        bodyLines.appendLine("// leer: falta variable");
        return;
    }
    ensureDeclared(varName, "int", "0");
    bodyLines.appendLine("cin >> " + varName + ";");
}

void CodeGenerator::genIf(string params) {
    string condition = normalizeConditionTokens(params);
    bodyLines.appendLine("if (" + condition + ")");
    headerJustEmitted = true;
    nextCloseLine = "";
}

void CodeGenerator::genElse() {
    bodyLines.appendLine("else");
    headerJustEmitted = true;
    nextCloseLine = "";
}

void CodeGenerator::genWhile(string params) {
    string condition = normalizeConditionTokens(params);
    bodyLines.appendLine("while (" + condition + ")");
    headerJustEmitted = true;
    nextCloseLine = "";
}

void CodeGenerator::genDoUntil(string params) {
    string condition = normalizeConditionTokens(params);
    bodyLines.appendLine("do");
    headerJustEmitted = true;
    nextCloseLine = "while (" + condition + ");";
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
        bodyLines.appendLine("// para/hasta: formato no reconocido");
        return;
    }
    bodyLines.appendLine("for (int " + varName + " = " + startVal + "; " + varName + " <= " + endVal + "; " + varName + "++)");
    headerJustEmitted = true;
    nextCloseLine = "";
}

void CodeGenerator::genDefineFunction(string params) {
    string signature = helper.trimSimple(params);
    if (signature == "") {
        preludeLines.appendLine("void funcion()");
        preludeLines.appendLine("{");
        preludeLines.appendLine("}");
        return;
    }
    int open = helper.indexOfText(signature, "(");
    if (open < 0) {
        signature = signature + "()";
    }
    preludeLines.appendLine(signature);
    preludeLines.appendLine("{");
    preludeLines.appendLine("}");
}

void CodeGenerator::genCreateVar(string params) {
    string text = helper.trimSimple(params);
    string typeName = "int";
    string varName = "";
    string initValue = "";

    int withPos = helper.indexOfText(text, "con valor");
    if (withPos >= 0) {
        string left = helper.trimSimple(text.substr(0, withPos));
        string right = helper.trimSimple(text.substr(withPos + 9));
        text = left;
        initValue = right;

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

    ensureDeclared(varName, typeName, initValue);
}

void CodeGenerator::genAssign(string params) {
    string text = helper.trimSimple(params);
    string varName = "";
    string value = "";

    int posA = helper.indexOfText(text, " a ");
    if (posA >= 0) {
        value = helper.trimSimple(text.substr(0, posA));
        varName = helper.trimSimple(text.substr(posA + 3));
    }
    else {
        bool ok = splitAssignRight(text, varName, value);
        if (!ok) {
            bodyLines.appendLine("// asignar valor: formato no reconocido");
            return;
        }
    }

    if (!symbolTable.variableExists(varName)) {
        string inferred = inferTypeFromToken(value);
        ensureDeclared(varName, inferred, value);
        return;
    }
    bodyLines.appendLine(varName + " = " + value + ";");
    symbolTable.assignValue(varName, value);
}

void CodeGenerator::genCreateArray(string params) {
    string text = helper.trimSimple(params);
    string arrayName = "";
    string typeName = "int";
    int sizeValue = 0;

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
    bodyLines.appendLine(typeName + string(" ") + arrayName + string("[") + os.str() + string("];"));
}

void CodeGenerator::genTraverseList(string params) {
    string arrayName = helper.trimSimple(params);
    if (arrayName == "") {
        arrayName = "lista1";
    }
    bodyLines.appendLine("for (int i = 0; i < N; i++)");
    bodyLines.appendLine("{");
    bodyLines.appendLine("    // usar " + arrayName + "[i]");
    bodyLines.appendLine("}");
}

void CodeGenerator::genAddToList(string params) {
    bodyLines.appendLine("// agregar a la lista requiere estructura dinamica (vector) no permitida");
}

void CodeGenerator::genBeginProgram() {
    bodyLines.appendLine("// comenzar programa");
}

void CodeGenerator::genEndProgram() {
    bodyLines.appendLine("// terminar programa");
}

void CodeGenerator::genComment(string params) {
    bodyLines.appendLine("// " + params);
}

void CodeGenerator::genCallFunction(string params) {
    string callName = helper.trimSimple(params);
    if (callName == "") {
        bodyLines.appendLine("funcion();");
        return;
    }
    int open = helper.indexOfText(callName, "(");
    if (open < 0) {
        callName = callName + "()";
    }
    bodyLines.appendLine(callName + ";");
}

void CodeGenerator::generate(Instruction ins) {
    string op = ins.getOperation();
    string params = helper.trimSimple(ins.getParameters());
    int indentLevel = ins.getIndent();

    beforeEmit(indentLevel);

    if (op == "sum") { genSum(params); afterEmit(indentLevel); return; }
    if (op == "sub") { genSub(params); afterEmit(indentLevel); return; }
    if (op == "mul") { genMul(params); afterEmit(indentLevel); return; }
    if (op == "div") { genDiv(params); afterEmit(indentLevel); return; }
    if (op == "calc") { genCalc(params); afterEmit(indentLevel); return; }

    if (op == "print") { genPrint(params); afterEmit(indentLevel); return; }
    if (op == "message") { genMessage(params); afterEmit(indentLevel); return; }
    if (op == "read") { genRead(params); afterEmit(indentLevel); return; }

    if (op == "if") { genIf(params); afterEmit(indentLevel); return; }
    if (op == "else") { genElse(); afterEmit(indentLevel); return; }
    if (op == "while") { genWhile(params); afterEmit(indentLevel); return; }
    if (op == "do_until") { genDoUntil(params); afterEmit(indentLevel); return; }
    if (op == "for_to") { genForTo(params); afterEmit(indentLevel); return; }

    if (op == "create_var") { genCreateVar(params); afterEmit(indentLevel); return; }
    if (op == "assign") { genAssign(params); afterEmit(indentLevel); return; }

    if (op == "create_array") { genCreateArray(params); afterEmit(indentLevel); return; }
    if (op == "traverse_list") { genTraverseList(params); afterEmit(indentLevel); return; }
    if (op == "add_to_list") { genAddToList(params); afterEmit(indentLevel); return; }

    if (op == "begin_program") { genBeginProgram(); afterEmit(indentLevel); return; }
    if (op == "end_program") { genEndProgram(); afterEmit(indentLevel); return; }
    if (op == "comment") { genComment(params); afterEmit(indentLevel); return; }
    if (op == "def_func") { genDefineFunction(params); afterEmit(indentLevel); return; }
    if (op == "call_func") { genCallFunction(params); afterEmit(indentLevel); return; }

    bodyLines.appendLine("// unknown: " + params);
    afterEmit(indentLevel);
}


string CodeGenerator::buildProgram() {
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

    closeAllBlocks();                      
    if (body != "" && body[(int)body.length() - 1] != '\n') { out = out + "\n"; }

    out = out + "return 0;\n";
    out = out + "}\n";
    return out;
}

void CodeGenerator::clearAll() {
    symbolTable.clearList();
    preludeLines.clearAll();
    bodyLines.clearAll();
    arrayCounter = 1;
    lastIndent = 0;
    stackCount = 0;
    headerJustEmitted = false;
    nextCloseLine = "";
}

