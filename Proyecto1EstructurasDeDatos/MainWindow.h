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
#include <QTimer>
#include <QString>

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
    QLabel* subtitleLabel;
    QPlainTextEdit* inputEdit;
    QPlainTextEdit* codeEdit;
    QPushButton* loadButton;
    QPushButton* translateButton;
    QPushButton* exportButton;
    QPushButton* copyButton;
    QLabel* statusChip;
    InstructionProcessor processor;
    CodeGenerator generator;
    InstructionHighlighter* highlighter;
    QString metricsSummary;
    QString statusStateText;
    bool isTranslating;
    void buildUi();
    void applyDarkTheme();
    void connectSignals();
    void translateAllLines();
    void updateInputMetrics();
    void setTranslatingState(bool active);
    void updateStatusLabel(const QString& stateText);
    void setStatusState(const QString& stateName);
    void scheduleStatusReset(int durationMs = 2200);

private slots:
    void onLoadClicked();
    void onTranslateClicked();
    void onExportClicked();
    void onCopyClicked();
};

#endif
