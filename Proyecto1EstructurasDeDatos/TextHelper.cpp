#include "TextHelper.h"

bool TextHelper::isSpaceSimple(char c) {
    unsigned char uc = static_cast<unsigned char>(c);
    if (c == ' ') {
        return true;
    }
    if (c == '\n') {
        return true;
    }
    if (c == '\r') {
        return true;
    }
    if (c == '\t') {
        return true;
    }
    if (c == '\v') {
        return true;
    }
    if (c == '\f') {
        return true;
    }
    if (uc == 0xC2 || uc == 0xA0) {
        return true;
    }
    return false;
}

bool TextHelper::isDigitSimple(char c) {
    if (c >= '0' && c <= '9') {
        return true;
    }
    return false;
}

char TextHelper::toLowerAscii(char c) {
    if (c >= 'A' && c <= 'Z') {
        return char(c + 32);
    }
    return c;
}

char TextHelper::removeAccentChar(char c) {
    switch (c) {
    case '\xc1':
    case '\xe1':
        return 'a';
    case '\xc9':
    case '\xe9':
        return 'e';
    case '\xcd':
    case '\xed':
        return 'i';
    case '\xd3':
    case '\xf3':
        return 'o';
    case '\xda':
    case '\xfa':
    case '\xdc':
    case '\xfc':
        return 'u';
    case '\xd1':
    case '\xf1':
        return 'n';
    default:
        return c;
    }
}

string TextHelper::toLowerSimple(string text) {
    int i = 0;
    int n = (int)text.length();
    while (i < n) {
        text[i] = toLowerAscii(text[i]);
        i = i + 1;
    }
    return text;
}

string TextHelper::removeAccents(string text) {
    int i = 0;
    int n = (int)text.length();
    while (i < n) {
        text[i] = removeAccentChar(text[i]);
        i = i + 1;
    }
    return text;
}

string TextHelper::trimSimple(string text) {
    int left = 0;
    int right = (int)text.length() - 1;

    while (left <= right && isSpaceSimple(text[left])) {
        left = left + 1;
    }
    while (right >= left && isSpaceSimple(text[right])) {
        right = right - 1;
    }
    if (right < left) {
        return "";
    }
    return text.substr(left, right - left + 1);
}

int TextHelper::indexOfText(string text, string key) {
    int pos = (int)text.find(key);
    if (pos < 0) {
        return -1;
    }
    return pos;
}

string TextHelper::cutAfter(string text, string key) {
    int pos = indexOfText(text, key);
    if (pos < 0) { return text; }
    int begin = pos + (int)key.length();
    if (begin < 0) { begin = 0; }
    if (begin > (int)text.length()) { return ""; }
    return text.substr(begin);
}

string TextHelper::cutBetweenSimple(string text, string startKey, string endKey) {
    int a = indexOfText(text, startKey);
    if (a < 0) { return ""; }
    int begin = a + (int)startKey.length();
    if (begin < 0) { begin = 0; }
    if (begin > (int)text.length()) { return ""; }
    int rel = indexOfText(text.substr(begin), endKey);
    if (rel < 0) { return trimSimple(text.substr(begin)); }
    int end = begin + rel;
    return trimSimple(text.substr(begin, end - begin));
}


string TextHelper::replaceAllSimple(string text, string findWhat, string putWhat) {
    int pos = indexOfText(text, findWhat);
    while (pos >= 0) {
        string leftPart = text.substr(0, pos);
        string rightPart = text.substr(pos + (int)findWhat.length());
        text = leftPart + putWhat + rightPart;
        int nextStart = pos + (int)putWhat.length();
        int nextPos = (int)text.find(findWhat, nextStart);
        if (nextPos < 0) {
            return text;
        }
        pos = nextPos;
    }
    return text;
}
