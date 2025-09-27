#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QClipboard>
#include <QGuiApplication>
#include <QProgressDialog>
#include <QTimer>

#include "InstructionProcessor.h"
#include "CodeGenerator.h"
#include "CodeWriter.h"
#include "InstructionHighlighter.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);

private:
    QLabel* titleLabel;
    QPlainTextEdit* inputEdit;
    QPlainTextEdit* codeEdit;
    QPushButton* loadButton;
    QPushButton* translateButton;
    QPushButton* exportButton;
    QPushButton* copyButton;
    InstructionProcessor processor;
    CodeGenerator generator;
    InstructionHighlighter* highlighter;
    QProgressDialog* progress;
    void buildUi();
    void applyDarkTheme();
    void connectSignals();
    void translateAllLines();
    void showBusy();
    void hideBusy();

private slots:
    void onLoadClicked();
    void onTranslateClicked();
    void onExportClicked();
    void onCopyClicked();
};

#endif
