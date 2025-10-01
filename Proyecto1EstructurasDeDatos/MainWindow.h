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
#include <QFrame>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QResizeEvent>

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
    QLabel* dropOverlayLabel;
    QPushButton* loadButton;
    QPushButton* translateButton;
    QPushButton* exportButton;
    QPushButton* copyButton;
    QLabel* statusChip;
    QSplitter* editorSplitter;
    QFrame* leftCard;
    QFrame* rightCard;
    InstructionProcessor processor;
    CodeGenerator generator;
    InstructionHighlighter* highlighter;
    QString metricsSummary;
    QString statusStateText;
    bool isTranslating;
    void buildUi();
    void applyDarkTheme();
    void connectSignals();
    void initEntryAnimations();
    void pulseButton(QPushButton* button);
    void loadInstructionsFromFile(const QString& path);
    void translateAllLines();
    void updateInputMetrics();
    void setTranslatingState(bool active);
    void updateStatusLabel(const QString& stateText);
    void setStatusState(const QString& stateName);
    void scheduleStatusReset(int durationMs = 2200);
    void updateDropOverlayGeometry();

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onLoadClicked();
    void onTranslateClicked();
    void onExportClicked();
    void onCopyClicked();
};

#endif
