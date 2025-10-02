#include "MainWindow.h"
#include <QScreen>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QTextBlockFormat>
#include <QTextCursor>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QRegularExpression>
#include <QStyle>
#include <QScrollBar>
#include <QFileInfo>
#include <QStringList>
#include <QFontDatabase>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QEasingCurve>
#include <QResizeEvent>
#include <QUrl>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    dropOverlayLabel(nullptr),
    globalDropOverlay(nullptr),
    globalDropIcon(nullptr),
    globalDropTitle(nullptr),
    globalDropSubtitle(nullptr),
    loadButton(nullptr),
    translateButton(nullptr),
    exportButton(nullptr),
    copyButton(nullptr),
    statusChip(nullptr),
    editorSplitter(nullptr),
    leftCard(nullptr),
    rightCard(nullptr),
    metricsSummary(QStringLiteral("Líneas: 0 • Palabras: 0")),
    statusStateText(QStringLiteral("Listo")),
    isTranslating(false) {
    applyDarkTheme();
    buildUi();
    connectSignals();
    initEntryAnimations();
    setAcceptDrops(true);
}

void MainWindow::applyDarkTheme() {
    QPalette pal = qApp->palette();
    pal.setColor(QPalette::Window, QColor("#111217"));
    pal.setColor(QPalette::Base, QColor("#161922"));
    pal.setColor(QPalette::AlternateBase, QColor("#1c202b"));
    pal.setColor(QPalette::Text, QColor("#f1f3f4"));
    pal.setColor(QPalette::Button, QColor("#252a36"));
    pal.setColor(QPalette::ButtonText, QColor("#f1f3f4"));
    pal.setColor(QPalette::Highlight, QColor("#5a82ff"));
    pal.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    qApp->setPalette(pal);

    setStyleSheet(
        "QMainWindow {"
        "  background: #0f1116;"
        "  color: #f1f3f4;"
        "}"
        "QWidget#centralBackground {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #131722, stop:1 #10131b);"
        "}"
        "QLabel#titleLabel {"
        "  color: #ffffff;"
        "  font-weight: 700;"
        "  font-size: 22pt;"
        "  letter-spacing: 0.6px;"
        "  margin: 4px 0 0 0;"
        "}"
        "QLabel#subtitleLabel {"
        "  color: #9aa0a6;"
        "  font-size: 11.2pt;"
        "  margin-bottom: 18px;"
        "}"
        "QLabel#editorTitle {"
        "  color: #9aa0a6;"
        "  font-size: 11pt;"
        "  letter-spacing: 1px;"
        "  text-transform: uppercase;"
        "}"
        "QLabel#hintLabel {"
        "  color: #6f7684;"
        "  font-size: 9.6pt;"
        "}"
        "QFrame#editorCard {"
        "  background: rgba(22, 26, 36, 0.92);"
        "  border: 1px solid rgba(66, 90, 130, 0.35);"
        "  border-radius: 20px;"
        "  transition: all 180ms ease-in-out;"
        "}"
        "QFrame#editorCard[dragging='true'] {"
        "  border: 1.2px dashed #7ba9ff;"
        "  background: rgba(30, 38, 58, 0.95);"
        "}"
        "QPlainTextEdit {"
        "  font-family: 'JetBrains Mono', 'Fira Code', Consolas, 'Courier New', monospace;"
        "  font-size: 13pt;"
        "  color: #e8eaed;"
        "  background: transparent;"
        "  border: none;"
        "  padding: 8px 4px;"
        "  selection-background-color: rgba(90, 130, 255, 0.35);"
        "  selection-color: #ffffff;"
        "}"
        "QPlainTextEdit[readOnly='true'] {"
        "  color: #d7e1ff;"
        "}"
        "QSplitter#editorSplitter::handle {"
        "  background: #262c39;"
        "  border-radius: 3px;"
        "  margin: 0 14px;"
        "}"
        "QSplitter#editorSplitter::handle:hover {"
        "  background: #32394b;"
        "}"
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2d3346, stop:1 #202433);"
        "  border: 1px solid rgba(123, 169, 255, 0.16);"
        "  border-radius: 12px;"
        "  padding: 10px 22px;"
        "  color: #f1f3f4;"
        "  font-size: 11.2pt;"
        "  font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #32394c, stop:1 #242a36);"
        "  border-color: rgba(123, 169, 255, 0.35);"
        "}"
        "QPushButton:pressed {"
        "  background: #1a1f28;"
        "}"
        "QPushButton#translateButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5473ff, stop:1 #3652f7);"
        "  border: 1px solid #5a82ff;"
        "  color: #ffffff;"
        "  font-weight: 600;"
        "}"
        "QPushButton#translateButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6380ff, stop:1 #415cf9);"
        "}"
        "QPushButton#exportButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2f646c, stop:1 #214a51);"
        "  border: 1px solid #3f7f89;"
        "}"
        "QPushButton#exportButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #35717a, stop:1 #27565e);"
        "}"
        "QPushButton#copyButton {"
        "  padding: 6px 18px;"
        "  font-size: 10.8pt;"
        "}"
        "QPushButton#loadButton {"
        "  min-width: 220px;"
        "}"
        "QLabel#statusChip {"
        "  padding: 6px 14px;"
        "  border-radius: 12px;"
        "  font-size: 10.5pt;"
        "  background: rgba(138, 180, 248, 0.12);"
        "  color: #8ab4f8;"
        "  border: 1px solid rgba(138, 180, 248, 0.35);"
        "}"
        "QLabel#statusChip[state='busy'] {"
        "  background: rgba(255, 171, 64, 0.16);"
        "  color: #ffb74d;"
        "  border: 1px solid rgba(255, 183, 77, 0.38);"
        "}"
        "QLabel#statusChip[state='success'] {"
        "  background: rgba(129, 199, 132, 0.18);"
        "  color: #9ccc65;"
        "  border: 1px solid rgba(156, 204, 101, 0.40);"
        "}"
        "QLabel#statusChip[state='warning'] {"
        "  background: rgba(244, 143, 177, 0.18);"
        "  color: #f48fb1;"
        "  border: 1px solid rgba(244, 143, 177, 0.45);"
        "}"
        "QFrame#globalDropOverlay {"
        "  background: rgba(15, 19, 29, 0.92);"
        "  border: 1.4px dashed rgba(123, 169, 255, 0.55);"
        "  border-radius: 26px;"
        "}"
        "QLabel#globalDropIcon {"
        "  font-size: 42pt;"
        "}"
        "QLabel#globalDropTitle {"
        "  color: #d6e4ff;"
        "  font-size: 15pt;"
        "  font-weight: 600;"
        "  letter-spacing: 0.8px;"
        "}"
        "QLabel#globalDropSubtitle {"
        "  color: #95a1c5;"
        "  font-size: 11.5pt;"
        "}"
        "QScrollBar:vertical {"
        "  background: transparent;"
        "  width: 12px;"
        "  margin: 10px 0;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: rgba(138, 180, 248, 0.4);"
        "  border-radius: 6px;"
        "  min-height: 36px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background: rgba(138, 180, 248, 0.7);"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0;"
        "}"
        "QScrollBar:horizontal {"
        "  background: transparent;"
        "  height: 12px;"
        "  margin: 0 10px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "  background: rgba(138, 180, 248, 0.4);"
        "  border-radius: 6px;"
        "  min-width: 36px;"
        "}"
        "QScrollBar::handle:horizontal:hover {"
        "  background: rgba(138, 180, 248, 0.7);"
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
        "  width: 0;"
        "}"
    );
}

void MainWindow::buildUi() {
    QWidget* central = new QWidget(this);
    central->setObjectName("centralBackground");

    QVBoxLayout* root = new QVBoxLayout(central);
    root->setContentsMargins(32, 20, 32, 28);
    root->setSpacing(14);

    titleLabel = new QLabel("Traductor a C++", central);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    root->addWidget(titleLabel, 0, Qt::AlignHCenter);

    editorSplitter = new QSplitter(Qt::Horizontal, central);
    editorSplitter->setObjectName("editorSplitter");
    editorSplitter->setHandleWidth(12);
    editorSplitter->setChildrenCollapsible(false);
    editorSplitter->setOpaqueResize(false);
    editorSplitter->setMinimumHeight(560);

    QWidget* left = new QWidget(editorSplitter);
    QVBoxLayout* leftLayout = new QVBoxLayout(left);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    leftCard = new QFrame(left);
    leftCard->setObjectName("editorCard");
    QVBoxLayout* leftCardLayout = new QVBoxLayout(leftCard);
    leftCardLayout->setContentsMargins(30, 28, 30, 30);
    leftCardLayout->setSpacing(16);
    leftCard->setMinimumWidth(520);

    QLabel* inputHeader = new QLabel("Lenguaje natural", leftCard);
    inputHeader->setObjectName("editorTitle");
    leftCardLayout->addWidget(inputHeader);

    QLabel* hintLabel = new QLabel("Escribe una instrucción por línea o arrastra un archivo .txt.", leftCard);
    hintLabel->setObjectName("hintLabel");
    hintLabel->setWordWrap(true);
    leftCardLayout->addWidget(hintLabel);

    inputEdit = new QPlainTextEdit(leftCard);
    inputEdit->setPlaceholderText("Escribe instrucciones o carga un archivo .txt...");
    inputEdit->setWordWrapMode(QTextOption::NoWrap);
    inputEdit->setMinimumHeight(420);
    leftCardLayout->addWidget(inputEdit, 1);

    dropOverlayLabel = new QLabel("Suelta aquí tu archivo .txt\n(o en cualquier parte de la ventana)", inputEdit->viewport());
    dropOverlayLabel->setAlignment(Qt::AlignCenter);
    dropOverlayLabel->setStyleSheet(
        "background: rgba(18, 24, 37, 0.88);"
        "border: 1.3px dashed rgba(123, 169, 255, 0.55);"
        "border-radius: 18px;"
        "color: #aec6ff;"
        "font-size: 12.8pt;"
        "font-weight: 600;"
        "letter-spacing: 0.8px;"
        "text-transform: uppercase;"
        "padding: 28px 24px;"
    );
    dropOverlayLabel->setWordWrap(true);
    dropOverlayLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    dropOverlayLabel->hide();

    leftLayout->addWidget(leftCard);

    highlighter = new InstructionHighlighter(inputEdit->document());

    QWidget* right = new QWidget(editorSplitter);
    QVBoxLayout* rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    rightCard = new QFrame(right);
    rightCard->setObjectName("editorCard");
    QVBoxLayout* rightCardLayout = new QVBoxLayout(rightCard);
    rightCardLayout->setContentsMargins(30, 28, 30, 30);
    rightCardLayout->setSpacing(16);
    rightCard->setMinimumWidth(520);

    QHBoxLayout* rightHeader = new QHBoxLayout();
    rightHeader->setContentsMargins(0, 0, 0, 0);
    rightHeader->setSpacing(12);

    QLabel* outputHeader = new QLabel("Código C++ generado", rightCard);
    outputHeader->setObjectName("editorTitle");
    rightHeader->addWidget(outputHeader);
    rightHeader->addStretch();

    copyButton = new QPushButton("Copiar código", rightCard);
    copyButton->setObjectName("copyButton");
    copyButton->setToolTip("Copiar el código generado al portapapeles");
    copyButton->setMinimumWidth(150);
    rightHeader->addWidget(copyButton);

    rightCardLayout->addLayout(rightHeader);

    codeEdit = new QPlainTextEdit(rightCard);
    codeEdit->setPlaceholderText("El código C++ aparecerá aquí...");
    codeEdit->setReadOnly(true);
    codeEdit->setWordWrapMode(QTextOption::NoWrap);
    codeEdit->setMinimumHeight(420);
    rightCardLayout->addWidget(codeEdit, 1);

    rightLayout->addWidget(rightCard);

    editorSplitter->addWidget(left);
    editorSplitter->addWidget(right);
    editorSplitter->setStretchFactor(0, 1);
    editorSplitter->setStretchFactor(1, 1);

    QList<int> sizes;
    sizes << 600 << 600;
    editorSplitter->setSizes(sizes);

    root->addWidget(editorSplitter, 1);

    QHBoxLayout* bottom = new QHBoxLayout();
    bottom->setContentsMargins(0, 16, 0, 0);
    bottom->setSpacing(20);

    loadButton = new QPushButton("Cargar instrucciones", central);
    loadButton->setObjectName("loadButton");
    loadButton->setToolTip("Abrir instrucciones desde un archivo de texto");

    translateButton = new QPushButton("Traducir", central);
    translateButton->setObjectName("translateButton");
    translateButton->setMinimumWidth(150);
    translateButton->setToolTip("Generar el código C++");

    exportButton = new QPushButton("Exportar a .cpp", central);
    exportButton->setObjectName("exportButton");
    exportButton->setMinimumWidth(170);
    exportButton->setToolTip("Guardar el resultado como archivo .cpp");

    statusChip = new QLabel(central);
    statusChip->setObjectName("statusChip");
    statusChip->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    statusChip->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    statusChip->setMinimumWidth(260);
    setStatusState("ready");
    updateStatusLabel("Listo");

    bottom->addWidget(loadButton);
    bottom->addWidget(statusChip);
    bottom->addStretch(2);
    bottom->addWidget(translateButton);
    bottom->addWidget(exportButton);

    root->addLayout(bottom);
    root->setStretch(0, 0);
    root->setStretch(1, 0);
    root->setStretch(2, 1);
    root->setStretch(3, 0);

    globalDropOverlay = new QFrame(central);
    globalDropOverlay->setObjectName("globalDropOverlay");
    globalDropOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    globalDropOverlay->hide();

    QVBoxLayout* overlayLayout = new QVBoxLayout(globalDropOverlay);
    overlayLayout->setContentsMargins(72, 100, 72, 100);
    overlayLayout->setSpacing(10);
    overlayLayout->setAlignment(Qt::AlignCenter);
    overlayLayout->addStretch();

    globalDropIcon = new QLabel(QStringLiteral("📄"), globalDropOverlay);
    globalDropIcon->setObjectName("globalDropIcon");
    globalDropIcon->setAlignment(Qt::AlignCenter);
    overlayLayout->addWidget(globalDropIcon, 0, Qt::AlignHCenter);

    globalDropTitle = new QLabel(QStringLiteral("Suelta tu archivo .txt"), globalDropOverlay);
    globalDropTitle->setObjectName("globalDropTitle");
    globalDropTitle->setAlignment(Qt::AlignCenter);
    globalDropTitle->setWordWrap(true);
    overlayLayout->addWidget(globalDropTitle, 0, Qt::AlignHCenter);

    globalDropSubtitle = new QLabel(QStringLiteral("Puedes arrastrarlo a cualquier parte de la ventana y lo detectaremos automáticamente."), globalDropOverlay);
    globalDropSubtitle->setObjectName("globalDropSubtitle");
    globalDropSubtitle->setAlignment(Qt::AlignCenter);
    globalDropSubtitle->setWordWrap(true);
    overlayLayout->addWidget(globalDropSubtitle, 0, Qt::AlignHCenter);

    overlayLayout->addStretch();

    setCentralWidget(central);
    setWindowTitle("Traductor a C++");

    auto applyShadow = [](QWidget* target) {
        auto* effect = new QGraphicsDropShadowEffect(target);
        effect->setBlurRadius(32);
        effect->setOffset(0, 20);
        effect->setColor(QColor(0, 0, 0, 160));
        target->setGraphicsEffect(effect);
    };
    applyShadow(leftCard);
    applyShadow(rightCard);

    if (globalDropOverlay) {
        globalDropOverlay->raise();
    }
    if (dropOverlayLabel) {
        dropOverlayLabel->raise();
    }

    QList<QPushButton*> clickableButtons = { loadButton, translateButton, exportButton, copyButton };
    for (QPushButton* btn : clickableButtons) {
        if (btn) {
            btn->setCursor(Qt::PointingHandCursor);
        }
    }

    const QStringList candidateFamilies = QStringList()
        << QStringLiteral("JetBrains Mono")
        << QStringLiteral("Fira Code")
        << QStringLiteral("Consolas")
        << QStringLiteral("Courier New")
        << QStringLiteral("DejaVu Sans Mono");

    QStringList availableFamilies;
    for (const QString& family : candidateFamilies) {
        if (QFontDatabase::hasFamily(family)) {
            availableFamilies << family;
        }
    }

    QFont mono;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (!availableFamilies.isEmpty()) {
        mono.setFamilies(availableFamilies);
    } else {
        mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    }
#else
    if (!availableFamilies.isEmpty()) {
        mono.setFamily(availableFamilies.first());
    } else {
        mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    }
#endif
    mono.setStyleHint(QFont::Monospace);
    mono.setPointSize(13);
    inputEdit->setFont(mono);
    codeEdit->setFont(mono);

    auto setLineHeight = [](QPlainTextEdit* edit, int percent) {
        QTextCursor cur(edit->document());
        cur.select(QTextCursor::Document);
        QTextBlockFormat fmt;
        fmt.setLineHeight(percent, QTextBlockFormat::ProportionalHeight);
        cur.mergeBlockFormat(fmt);
    };
    setLineHeight(inputEdit, 120);
    setLineHeight(codeEdit, 120);

    inputEdit->setTabStopDistance(4 * inputEdit->fontMetrics().horizontalAdvance(QLatin1Char(' ')));
    codeEdit->setTabStopDistance(4 * codeEdit->fontMetrics().horizontalAdvance(QLatin1Char(' ')));

    inputEdit->setCenterOnScroll(true);
    codeEdit->setCenterOnScroll(true);

    updateDropOverlayGeometry();
    updateGlobalDropOverlayGeometry();
    updateInputMetrics();

    resize(1280, 780);
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
    connect(inputEdit, &QPlainTextEdit::textChanged, this, &MainWindow::updateInputMetrics);
    connect(loadButton, &QPushButton::clicked, this, [this]() { pulseButton(loadButton); });
    connect(translateButton, &QPushButton::clicked, this, [this]() { pulseButton(translateButton); });
    connect(exportButton, &QPushButton::clicked, this, [this]() { pulseButton(exportButton); });
    connect(copyButton, &QPushButton::clicked, this, [this]() { pulseButton(copyButton); });
}

void MainWindow::initEntryAnimations() {
    setWindowOpacity(0.0);
    QTimer::singleShot(20, this, [this]() {
        QPropertyAnimation* fadeIn = new QPropertyAnimation(this, "windowOpacity", this);
        fadeIn->setDuration(380);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->setEasingCurve(QEasingCurve::OutCubic);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

        if (!leftCard || !rightCard) {
            return;
        }

        QRect leftGeom = leftCard->geometry();
        QRect rightGeom = rightCard->geometry();

        leftCard->setGeometry(leftGeom.translated(0, 36));
        rightCard->setGeometry(rightGeom.translated(0, 36));

        auto* group = new QParallelAnimationGroup(this);

        auto createAnim = [&](QWidget* widget, const QRect& finalGeom) {
            QPropertyAnimation* anim = new QPropertyAnimation(widget, "geometry", group);
            anim->setDuration(520);
            anim->setStartValue(widget->geometry());
            anim->setEndValue(finalGeom);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            group->addAnimation(anim);
        };

        createAnim(leftCard, leftGeom);
        createAnim(rightCard, rightGeom);

        group->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void MainWindow::pulseButton(QPushButton* button) {
    if (!button) {
        return;
    }
    QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(button->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(button);
        effect->setOpacity(1.0);
        button->setGraphicsEffect(effect);
    }

    QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity", button);
    anim->setDuration(260);
    anim->setStartValue(1.0);
    anim->setKeyValueAt(0.5, 0.75);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::loadInstructionsFromFile(const QString& path) {
    if (path.isEmpty()) {
        return;
    }

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        codeEdit->setPlainText("No se pudo abrir el archivo.");
        setStatusState(QStringLiteral("warning"));
        updateStatusLabel(QStringLiteral("Error al abrir archivo"));
        scheduleStatusReset();
        return;
    }

    QTextStream in(&f);
    QString content = in.readAll();
    f.close();

    inputEdit->setPlainText(content);
    updateInputMetrics();

    QFileInfo info(path);
    const QString fileName = info.fileName();
    setStatusState(QStringLiteral("success"));
    updateStatusLabel(QStringLiteral("Archivo cargado: %1").arg(fileName));
    scheduleStatusReset();
}

void MainWindow::updateInputMetrics() {
    if (!inputEdit) {
        return;
    }

    const QString text = inputEdit->toPlainText();
    const int lineCount = text.isEmpty() ? 0 : text.count('\n') + 1;
    const int wordCount = text.isEmpty() ? 0 : text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
    const int charCount = text.length();

    metricsSummary = QStringLiteral("Líneas: %1 • Palabras: %2 • Caracteres: %3")
        .arg(lineCount)
        .arg(wordCount)
        .arg(charCount);

    if (statusChip) {
        statusChip->setToolTip(QStringLiteral("Resumen del texto – %1").arg(metricsSummary));
    }

    updateStatusLabel(statusStateText);
}

void MainWindow::setTranslatingState(bool active) {
    isTranslating = active;
    if (active) {
        setStatusState(QStringLiteral("busy"));
        updateStatusLabel(QStringLiteral("Traduciendo…"));
    } else {
        setStatusState(QStringLiteral("success"));
        updateStatusLabel(QStringLiteral("Traducción lista"));
        scheduleStatusReset();
    }
}

void MainWindow::updateStatusLabel(const QString& stateText) {
    statusStateText = stateText;
    if (!statusChip) {
        return;
    }
    statusChip->setText(QStringLiteral("%1 • %2").arg(stateText, metricsSummary));
}

void MainWindow::setStatusState(const QString& stateName) {
    if (!statusChip) {
        return;
    }
    statusChip->setProperty("state", stateName);
    statusChip->style()->unpolish(statusChip);
    statusChip->style()->polish(statusChip);
    statusChip->update();
}

void MainWindow::scheduleStatusReset(int durationMs) {
    QTimer::singleShot(durationMs, this, [this]() {
        if (!isTranslating) {
            setStatusState(QStringLiteral("ready"));
            updateStatusLabel(QStringLiteral("Listo"));
        }
    });
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
    if (QScrollBar* vScroll = codeEdit->verticalScrollBar()) {
        vScroll->setValue(vScroll->minimum());
    }
}

void MainWindow::updateDropOverlayGeometry() {
    if (!dropOverlayLabel || !inputEdit) {
        return;
    }
    dropOverlayLabel->setGeometry(inputEdit->viewport()->rect().adjusted(36, 36, -36, -36));
}

void MainWindow::updateGlobalDropOverlayGeometry() {
    if (!globalDropOverlay || !centralWidget()) {
        return;
    }
    const QRect area = centralWidget()->rect().adjusted(26, 26, -26, -26);
    globalDropOverlay->setGeometry(area);
}

bool MainWindow::mimeHasTxtFile(const QMimeData* mime) const {
    if (!mime || !mime->hasUrls()) {
        return false;
    }
    const QList<QUrl> urls = mime->urls();
    for (const QUrl& url : urls) {
        if (url.isLocalFile() && url.toLocalFile().endsWith(QStringLiteral(".txt"), Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

void MainWindow::setDraggingVisualState(bool active) {
    if (leftCard) {
        leftCard->setProperty("dragging", active);
        leftCard->style()->unpolish(leftCard);
        leftCard->style()->polish(leftCard);
        leftCard->update();
    }
    if (dropOverlayLabel) {
        if (active) {
            updateDropOverlayGeometry();
            dropOverlayLabel->show();
        } else {
            dropOverlayLabel->hide();
        }
    }
    if (globalDropOverlay) {
        if (active) {
            updateGlobalDropOverlayGeometry();
            globalDropOverlay->show();
            globalDropOverlay->raise();
        } else {
            globalDropOverlay->hide();
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (mimeHasTxtFile(event->mimeData())) {
        event->acceptProposedAction();
        setDraggingVisualState(true);
        return;
    }
    QMainWindow::dragEnterEvent(event);
}

void MainWindow::dragMoveEvent(QDragMoveEvent* event) {
    if (mimeHasTxtFile(event->mimeData())) {
        event->acceptProposedAction();
        setDraggingVisualState(true);
        return;
    }
    QMainWindow::dragMoveEvent(event);
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent* event) {
    setDraggingVisualState(false);
    QMainWindow::dragLeaveEvent(event);
}

void MainWindow::dropEvent(QDropEvent* event) {
    setDraggingVisualState(false);

    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl& url : urls) {
            if (url.isLocalFile() && url.toLocalFile().endsWith(QStringLiteral(".txt"), Qt::CaseInsensitive)) {
                event->acceptProposedAction();
                loadInstructionsFromFile(url.toLocalFile());
                return;
            }
        }
    }
    QMainWindow::dropEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    updateDropOverlayGeometry();
    updateGlobalDropOverlayGeometry();
}


void MainWindow::onLoadClicked() {
    QString path = QFileDialog::getOpenFileName(this, "Seleccionar archivo de instrucciones", "", "Texto (*.txt)");
    if (path.isEmpty()) {
        return;
    }
    loadInstructionsFromFile(path);
}

void MainWindow::onTranslateClicked() {
    if (inputEdit->toPlainText().trimmed().isEmpty()) {
        codeEdit->setPlainText("No hay instrucciones para traducir.");
        setStatusState(QStringLiteral("warning"));
        updateStatusLabel(QStringLiteral("Nada que traducir"));
        scheduleStatusReset();
        return;
    }

    setTranslatingState(true);

    QTimer::singleShot(0, this, [this]() {
        translateAllLines();
        setTranslatingState(false);
    });
}

void MainWindow::onExportClicked() {
    if (codeEdit->toPlainText().trimmed().isEmpty()) {
        codeEdit->setPlainText("No hay codigo para exportar.");
        setStatusState(QStringLiteral("warning"));
        updateStatusLabel(QStringLiteral("Nada para exportar"));
        scheduleStatusReset();
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
        setStatusState(QStringLiteral("warning"));
        updateStatusLabel(QStringLiteral("Error al guardar"));
        scheduleStatusReset();
        return;
    }

    QFileInfo info(path);
    setStatusState(QStringLiteral("success"));
    updateStatusLabel(QStringLiteral(".cpp guardado: %1").arg(info.fileName()));
    scheduleStatusReset();
}

void MainWindow::onCopyClicked() {
    QString txt = codeEdit->toPlainText();
    if (txt.isEmpty()) {
        setStatusState(QStringLiteral("warning"));
        updateStatusLabel(QStringLiteral("Nada que copiar"));
        scheduleStatusReset();
        return;
    }
    QClipboard* cb = QGuiApplication::clipboard();
    cb->setText(txt);
    setStatusState(QStringLiteral("success"));
    updateStatusLabel(QStringLiteral("Código copiado"));
    scheduleStatusReset();
}
