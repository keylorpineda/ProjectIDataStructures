#pragma once
#ifndef INSTRUCTIONHIGHLIGHTER_H
#define INSTRUCTIONHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextDocument>
#include "InstructionProcessor.h"

class InstructionHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    InstructionHighlighter(QTextDocument* doc) : QSyntaxHighlighter(doc) {
        errorFormat.setForeground(QColor("#ff6b6b"));
        errorFormat.setFontWeight(QFont::Bold);
    }

protected:
    void highlightBlock(const QString& text) override {
        QString trimmed = text.trimmed();
        if (trimmed.isEmpty()) {
            return;
        }

        InstructionProcessor ip;
        Instruction ins = ip.parseInstruction(trimmed.toStdString());
        QString op = QString::fromStdString(ins.getOperation());

        if (op == "unknown") {
            int i = 0;
            int n = text.size();
            while (i < n && text.at(i).isSpace()) { i = i + 1; }
            int start = i;
            while (i < n && !text.at(i).isSpace()) { i = i + 1; }
            int len = i - start;
            if (len > 0) {
                setFormat(start, len, errorFormat);
            }
        }
    }

private:
    QTextCharFormat errorFormat;
};

#endif
