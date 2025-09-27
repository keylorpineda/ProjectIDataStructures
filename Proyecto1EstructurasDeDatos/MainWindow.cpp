#include "MainWindow.h"
#include <QScreen>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QTextBlockFormat>
#include <QTextCursor>
#include <limits>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    applyDarkTheme();
    buildUi();
    connectSignals();
}

void MainWindow::applyDarkTheme() {
    QPalette pal = qApp->palette();
    pal.setColor(QPalette::Window, QColor("#202124"));
    pal.setColor(QPalette::Base, QColor("#1e1f22"));
    pal.setColor(QPalette::AlternateBase, QColor("#2b2d31"));
    pal.setColor(QPalette::Text, QColor("#e8eaed"));
    pal.setColor(QPalette::Button, QColor("#2b2d31"));
    pal.setColor(QPalette::ButtonText, QColor("#e8eaed"));
    pal.setColor(QPalette::Highlight, QColor("#4f7cff"));
    pal.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    qApp->setPalette(pal);

    setStyleSheet(
        "QPlainTextEdit {"
        "  font-family: Consolas, 'Courier New', monospace;"
        "  font-size: 14pt;"              /* texto mas grande */
        "  color: #e8eaed;"
        "  background: #1e1f22;"
        "  border: 1px solid #2f3136;"
        "}"
        "QPushButton {"
        "  background: #3c4043;"
        "  border: 1px solid #5f6368;"
        "  border-radius: 8px;"
        "  padding: 6px 14px;"
        "}"
        "QPushButton:hover { background: #4a4f54; }"
        "QLabel#titleLabel {"
        "  color: #e8eaed; font-weight: 600; font-size: 18pt;"
        "  padding: 4px 0;"               /* encabezado mas compacto */
        "  margin: 0;"
        "}"
    );
}

void MainWindow::buildUi() {
    QWidget* central = new QWidget(this);
    QVBoxLayout* root = new QVBoxLayout(central);
    root->setContentsMargins(12, 8, 12, 12);  /* margenes moderados */
    root->setSpacing(8);

    /* encabezado */
    titleLabel = new QLabel("Traductor a C++", central);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    root->addWidget(titleLabel);

    /* divisor izquierda/derecha */
    QSplitter* split = new QSplitter(Qt::Horizontal, central);
    split->setHandleWidth(6);

    /* izquierda */
    QWidget* left = new QWidget(split);
    QVBoxLayout* leftLay = new QVBoxLayout(left);
    leftLay->setContentsMargins(0, 0, 6, 0);  /* simetria: 6px separacion al centro */
    inputEdit = new QPlainTextEdit(left);
    inputEdit->setPlaceholderText("Escribe instrucciones o carga un archivo .txt...");
    inputEdit->setWordWrapMode(QTextOption::NoWrap);
    leftLay->addWidget(inputEdit);

    /* resalte de errores (palabras desconocidas) */
    highlighter = new InstructionHighlighter(inputEdit->document());

    /* derecha */
    QWidget* right = new QWidget(split);
    QVBoxLayout* rightLay = new QVBoxLayout(right);
    rightLay->setContentsMargins(6, 0, 0, 0); /* simetria: 6px separacion al centro */

    QHBoxLayout* copyBar = new QHBoxLayout();
    copyBar->setContentsMargins(0, 0, 0, 0);
    copyButton = new QPushButton("Copiar", right);
    copyBar->addStretch();
    copyBar->addWidget(copyButton);

    codeEdit = new QPlainTextEdit(right);
    codeEdit->setReadOnly(true);
    codeEdit->setWordWrapMode(QTextOption::NoWrap);

    rightLay->addLayout(copyBar);
    rightLay->addWidget(codeEdit);

    split->addWidget(left);
    split->addWidget(right);
    split->setStretchFactor(0, 1);
    split->setStretchFactor(1, 1);

    /* forzar tamaos iniciales simetricos */
    QList<int> sizes;
    sizes << 1 << 1;
    split->setSizes(sizes);

    root->addWidget(split);

    /* barra inferior */
    QHBoxLayout* bottom = new QHBoxLayout();
    bottom->setContentsMargins(0, 0, 0, 0);
    loadButton = new QPushButton("Cargar", central);
    translateButton = new QPushButton("Traducir", central);
    exportButton = new QPushButton("A .cpp", central);

    bottom->addWidget(loadButton);
    bottom->addStretch();
    bottom->addWidget(translateButton);
    bottom->addStretch();
    bottom->addWidget(exportButton);

    root->addLayout(bottom);

    setCentralWidget(central);
    setWindowTitle("Traductor a C++");

    /* animacion de carga */
    progress = new QProgressDialog("Traduciendo...", QString(), 0, 0, this);
    progress->setWindowModality(Qt::ApplicationModal);
    progress->setCancelButton(nullptr);
    progress->setMinimumDuration(std::numeric_limits<int>::max());
    progress->setAutoClose(false);
    progress->setAutoReset(false);
    progress->setWindowTitle("Espere");
    progress->hide();

    /* tipografia: asegurar misma fuente y altura de linea en ambos editores */
    QFont mono("Consolas");
    mono.setPointSize(14);
    inputEdit->setFont(mono);
    codeEdit->setFont(mono);

    auto setLineHeight = [](QPlainTextEdit* edit, int percent) {
        QTextCursor cur(edit->document());
        cur.select(QTextCursor::Document);
        QTextBlockFormat fmt;
        fmt.setLineHeight(percent, QTextBlockFormat::ProportionalHeight); // 120 = 120%
        cur.mergeBlockFormat(fmt);
        };
    setLineHeight(inputEdit, 120);
    setLineHeight(codeEdit, 120);

    /* ventana mas compacta y centrada */
    resize(1080, 700);
    QTimer::singleShot(0, this, [this]() {
        if (QScreen* screen = QGuiApplication::primaryScreen()) {
            const QRect available = screen->availableGeometry();
            const QSize winSize = size();
            const QPoint centered = available.center() - QPoint(winSize.width() / 2, winSize.height() / 2);
            move(centered);
        }
    });
}

void MainWindow::connectSignals() {
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::onLoadClicked);
    connect(translateButton, &QPushButton::clicked, this, &MainWindow::onTranslateClicked);
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::onExportClicked);
    connect(copyButton, &QPushButton::clicked, this, &MainWindow::onCopyClicked);
}

void MainWindow::showBusy() {
    progress->setLabelText("Traduciendo...");
    progress->setRange(0, 0);
    progress->adjustSize();
    const QRect parentRect = geometry();
    const QPoint topLeft = parentRect.center() - QPoint(progress->width() / 2, progress->height() / 2);
    progress->move(topLeft);
    progress->show();
    progress->raise();
    progress->activateWindow();
}

void MainWindow::hideBusy() {
    progress->hide();
    progress->setMinimumDuration(std::numeric_limits<int>::max());
}

void MainWindow::translateAllLines() {
    generator.clearAll();

    QStringList lines = inputEdit->toPlainText().split('\n');
    int i = 0;
    int n = lines.size();
    while (i < n) {
        QString raw = lines.at(i);
        if (!raw.trimmed().isEmpty()) {
            Instruction ins = processor.parseInstruction(raw.toStdString()); // usa la linea completa
            generator.generate(ins);
        }
        i = i + 1;
    }

    std::string cpp = generator.buildProgram();
    codeEdit->setPlainText(QString::fromStdString(cpp));
}


void MainWindow::onLoadClicked() {
    QString path = QFileDialog::getOpenFileName(this, "Seleccionar archivo de instrucciones", "", "Texto (*.txt)");
    if (path.isEmpty()) {
        return;
    }
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        codeEdit->setPlainText("No se pudo abrir el archivo.");
        return;
    }
    QTextStream in(&f);
    QString content = in.readAll();
    f.close();

    inputEdit->setPlainText(content);
}

void MainWindow::onTranslateClicked() {
    if (inputEdit->toPlainText().trimmed().isEmpty()) {
        codeEdit->setPlainText("No hay instrucciones para traducir.");
        return;
    }

    showBusy();

    QTimer::singleShot(80, this, [this]() {
        translateAllLines();
        hideBusy();
        });
}

void MainWindow::onExportClicked() {
    if (codeEdit->toPlainText().trimmed().isEmpty()) {
        codeEdit->setPlainText("No hay codigo para exportar.");
        return;
    }

    QString path = QFileDialog::getSaveFileName(this, "Guardar archivo C++", "programa.cpp", "C++ (*.cpp)");
    if (path.isEmpty()) {
        return;
    }

    std::string cpp = codeEdit->toPlainText().toStdString();
    CodeWriter writer;
    if (!writer.writeToFile(path.toStdString(), cpp)) {
        codeEdit->setPlainText("No se pudo guardar el archivo .cpp.");
        return;
    }
}

void MainWindow::onCopyClicked() {
    QString txt = codeEdit->toPlainText();
    if (txt.isEmpty()) {
        return;
    }
    QClipboard* cb = QGuiApplication::clipboard();
    cb->setText(txt);
}
