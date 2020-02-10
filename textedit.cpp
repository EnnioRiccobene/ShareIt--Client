#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
#include <QComboBox>
#include <QFontComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QMenu>
#include <QMenuBar>
#include <QTextCodec>
#include <QTextEdit>
#include <QStatusBar>
#include <QToolBar>
#include <QTextCursor>
#include <QTextDocumentWriter>
#include <QTextList>
#include <QtDebug>
#include <QMessageBox>
#include <QMimeData>
#include <QPrinter>
#include <QStackedWidget>
#include <QThread>
#include <QScrollBar>
#include <QDesktopWidget>
#include <QMainWindow>

#include <iostream>

#include "textedit.h"
#include "util.h"
#include "EditorTypes.h"

const QString rsrcPath = ":/images";

TextEdit::TextEdit(QStackedWidget *parent, Service *service, QStringList fileData) : QMainWindow(parent) {

    const QIcon taskbarIcon = QIcon::fromTheme("taskbarIcon", QIcon(rsrcPath + "/icon.ico"));
    QWidget::setWindowIcon(taskbarIcon);

    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);

    messageExcess = "";
    this->uri = fileData[1];
    ulong siteCounter = fileData[2].toULong();
    QString fileContent = fileData[3];

    this->service = service;

    this->setStyleSheet("background-color:white;selection-background-color:#c2c2c2;selection-color:black;");
    // TODO verificare se è utile
    this->setAutoFillBackground(true);
    setWindowTitle(PROGRAM_NAME + service->currentFilename + " ");

    this->crdtProcessor = new CrdtProcessor(this, fileContent, this->service->userId, siteCounter);
    QString fileContentPlainText = this->crdtProcessor->getCurrentContentPlainText();
    //setWindowTitle(QCoreApplication::applicationName() + "[*]");

    textEdit = new QTextEdit(this);

    //flag booleano da settare a true quando si modifica il testo perché si riceve qualcosa dal server
    textChangeFromServer = false;
    //posizione precedente del cursor
    prevCursorPosition = 0;
    currentCursorPosition = 0;

    textEdit->setFontPointSize(12);
    font = textEdit->font();

    setCentralWidget(textEdit);

    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setupFileActions();
    setupEditActions();
    setupUsersActions();

    // non utilizzato
//    connect(textEdit->document(), &QTextDocument::modificationChanged, this, &QWidget::setWindowModified);
//    setWindowModified(textEdit->document()->isModified());

    connect(textEdit->document(), &QTextDocument::undoAvailable, actionUndo, &QAction::setEnabled);
    connect(textEdit->document(), &QTextDocument::redoAvailable, actionRedo, &QAction::setEnabled);

    actionUndo->setEnabled(textEdit->document()->isUndoAvailable());
    actionRedo->setEnabled(textEdit->document()->isRedoAvailable());

#ifndef QT_NO_CLIPBOARD
    actionCut->setEnabled(false);
    connect(textEdit, &QTextEdit::copyAvailable, actionCut, &QAction::setEnabled);
    actionCopy->setEnabled(false);
    connect(textEdit, &QTextEdit::copyAvailable, actionCopy, &QAction::setEnabled);

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &TextEdit::clipboardDataChanged);
#endif

    textEdit->setFocus();

    // riempio l'editor con il contenuto del file
    updateEditorContent(fileContentPlainText);

    prevCursorPosition = textEdit->textCursor().position();
    currentCursorPosition = textEdit->textCursor().position();

    // quando si fa scroll del textEdit ridisegna i cursori
    connect(this->textEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &TextEdit::drawAllCursors);

    // faccio in modo che l'editor si metta in ascolto di modifiche dal server
    connect(this->service->socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

    connect(textEdit, &QTextEdit::cursorPositionChanged, this, &TextEdit::cursorPositionChanged);

    connect(textEdit, &QTextEdit::textChanged, this, &TextEdit::textChange);
    connect(textEdit->document(), &QTextDocument::contentsChange, this, &TextEdit::contentsChange);

}

void TextEdit::readyRead() {
try{
    QByteArray request = this->service->socket->readAll();
    request = messageExcess.toUtf8() + request;
    QList<QStringList> tokenLists = Util::deserializeList(request);

    messageExcess = tokenLists[tokenLists.size()-1][0];
    tokenLists.removeLast();
    // TODO da rimuovere
    if (messageExcess != "") {
        qDebug() << "messageExcess :" << messageExcess;
    }


    for (QStringList tokenList : tokenLists) {
        if (tokenList[0] == "opsendupdate") {

            QString message = tokenList[1];
//            QString readableMessage = this->crdtProcessor->printRemote(message);
//            qDebug() << readableMessage;

            QVector<ulong> operationInfos = this->crdtProcessor->processRemote(message);

            int editPos = static_cast<int>(operationInfos[0]);
            int diff = static_cast<int>(operationInfos[META_TOT_INSERT]) -
                        static_cast<int>(operationInfos[META_TOT_DELETE]);

            QString newContent = this->crdtProcessor->getCurrentContentPlainText();

            prevCursorPosition = textEdit->textCursor().position();
            textChangeFromServer = true;

            QTextCursor cursor = textEdit->textCursor();
            int startSelection = 0;
            int endSelection = 0;
            if (cursor.hasSelection()) {
                cursor.select(QTextCursor::WordUnderCursor);
                startSelection = cursor.selectionStart();
                endSelection = cursor.selectionEnd();
            }

            disconnect(textEdit, &QTextEdit::cursorPositionChanged, this, &TextEdit::cursorPositionChanged);
            updateEditorContent(newContent);
            connect(textEdit, &QTextEdit::cursorPositionChanged, this, &TextEdit::cursorPositionChanged);

            int newCursorPosition = prevCursorPosition;
            if (newCursorPosition > editPos) {
                newCursorPosition = newCursorPosition + diff;
                if (diff < 0) {
                    newCursorPosition = qMax(newCursorPosition, editPos);
                }
            }

            QTextCursor c = textEdit->textCursor();
            c.setPosition(newCursorPosition);

            if (startSelection != 0 || endSelection != 0) {
                int realDiff = prevCursorPosition - newCursorPosition;
                startSelection = qMin(qMax(startSelection - realDiff, 0), textEdit->toPlainText().size());
                endSelection = qMin(qMax(endSelection - realDiff, 0), textEdit->toPlainText().size());

                if (startSelection != newCursorPosition) {
                    c.setPosition(startSelection);
                    c.setPosition(endSelection, QTextCursor::KeepAnchor);
                } else {
                    c.setPosition(endSelection);
                    c.setPosition(startSelection, QTextCursor::KeepAnchor);
                }
            }

            textEdit->setTextCursor(c);
            if (prevCursorPosition != c.position()) {
                this->service->sendCursorPosition(c.position());
            }
        } else if (tokenList[0] == "opcursorpos") {
            if (map_userId_user.contains(tokenList[1].toULong())) {
                User updater = map_userId_user.find(tokenList[1].toULong()).value();
                if (updater.updateRemoteCursorPosition(tokenList[2].toInt())) {
                    resetupUsersActions();
                    drawAllCursors();
                }
            } else { // allora l'utente è nuovo
                resetupUsersActions();
                drawAllCursors();
            }
        }
    }
} catch (...) { goFileListClicked(); }
}

void TextEdit::contentsChange(int position, int charsRemoved, int charsAdded) {
try {
    //se il cambiamento è avvenuto a causa di un pacchetto dal server non fare niente
    if(textChangeFromServer == true) {
//        textChangeFromServer = false;
        return;
    }

    QVector<ENUM_EDIT_CODE> insDelList;
    QVector<int> posList;
    QVector<QChar> qcharList;

    for (int i=0; i<charsRemoved; i++) {
        insDelList.append(EDIT_CODE_DELETE);
        posList.append(position);
        qcharList.append('_'); // qui non posso mettere '\0', qualunque altro carattere dovrebbe andare bene
    }
    for (int i=0; i<charsAdded; i++) {
        int pos = position + i;
        insDelList.append(EDIT_CODE_INSERT);
        posList.append(pos);
        qcharList.append(textEdit->toPlainText()[pos]);
    }

    // questo if l'ho messo perchè se faccio select all (ctrl+a) allora QT mi considera una replace dell'ultimo
    // char (ovvero '\0') e incasina tutto, quindi tolgo la delete e la insert di '\0'
    if (qcharList.last() == '\0') {
        insDelList.removeAt(0);
        posList.removeAt(0);
        qcharList.removeAt(0);
        insDelList.removeAt(insDelList.size()-1);
        posList.removeAt(posList.size()-1);
        qcharList.removeAt(qcharList.size()-1);
    }

    int maxPacketSize = 60; // la dimensione del pacchetto in numero di operazioni (es: 20 insert)
    bool isFirst = true;
    // questo ciclo for lo devo fare per evitare un problema che viene fuori quando si fa una modifica enorme (>3000 char)
    for (int i = 0; i < insDelList.size(); i += maxPacketSize) {
        if (!isFirst) {
            QThread::msleep(50);
        } else {
            isFirst = false;
        }
        QString message = this->crdtProcessor->processLocal(
                    insDelList.mid(i, maxPacketSize),
                    posList.mid(i, maxPacketSize),
                    qcharList.mid(i, maxPacketSize));

//        qDebug() << "SIZE: " << crdtProcessor->getSizeOfText();

//        QString readableMessage = this->crdtProcessor->printRemote(message);
//        qDebug() << readableMessage;

        ulong siteCounter = this->crdtProcessor->getSiteCnt();
        this->service->sendUpdateFile(siteCounter, message);
    }
} catch (...) { goFileListClicked(); }

}

void TextEdit::drawAllCursors() {
    for (User user : this->map_userId_user.values()) {
        user.updateRemoteCursorPosition(user.cursorPosition);
    }
}

void TextEdit::changeIsBackgroundColored() {
    this->isBackgroundColored = !this->isBackgroundColored;
    drawTextBackground();
}

void TextEdit::drawTextBackground() {
    _usersText.clear();
    if (!this->isBackgroundColored) {
        // "disegna" il background trasparente
        drawSingleTextBackground(0, textEdit->toPlainText().size(), Qt::transparent);
    } else {
        QList<ulong> siteIds = this->crdtProcessor->getSymIdList();

        for (int i = 0; i<siteIds.size(); i++) {
            drawSingleTextBackground(i, i+1, this->map_userId_user.find(siteIds[i]).value().textColor);
        }
    }
    textEdit->setExtraSelections(_usersText);
}

void TextEdit::drawSingleTextBackground(int startPos, int finishPos, QColor color) {
    QTextCursor _cursor(textEdit->document());
    _cursor.setPosition(startPos);
    _cursor.setPosition(finishPos, QTextCursor::KeepAnchor);
    QTextEdit::ExtraSelection _userText;
    _userText.format.setBackground(color);
    _userText.cursor = _cursor;
    _usersText.append(_userText);
}

void TextEdit::updateEditorContent(QString fileContentPlainText) {
    // setta il contenuto dell'editor
    textEdit->setText(fileContentPlainText);
}

void TextEdit::goFileListClicked() {
    this->service->closefile();
    this->service->currentFilename = nullptr;
    disconnect(this->service->socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    delete this->crdtProcessor;
    this->deleteLater();
    emit showStackedWidget();
}

void TextEdit::setupFileActions() {

    QToolBar *tb = addToolBar(tr("File Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&File"));

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(rsrcPath + "/home.png"));
    QAction *a = menu->addAction(newIcon,  tr("&Home"), this, &TextEdit::goFileListClicked);
    tb->addAction(a);
    menu->addSeparator();

    const QIcon uriIcon = QIcon::fromTheme("document-new", QIcon(rsrcPath + "/generateUri.png"));
    a = menu->addAction(uriIcon,  tr("&Generate link"), this, &TextEdit::showUri);
    tb->addAction(a);
    menu->addSeparator();

    const QIcon exportPdfIcon = QIcon::fromTheme("exportpdf", QIcon(rsrcPath + "/exportpdf.png"));
    a = menu->addAction(exportPdfIcon, tr("&Export PDF..."), this, &TextEdit::filePrintPdf);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(Qt::CTRL + Qt::Key_D);
    tb->addAction(a);

}

void TextEdit::showUri() {
    QMessageBox::information(this, "LINK", this->uri);
}

void TextEdit::setupEditActions() {

    QToolBar *tb = addToolBar(tr("Edit Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&Edit"));

    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(rsrcPath + "/editundo.png"));
    actionUndo = menu->addAction(undoIcon, tr("&Undo"), textEdit, &QTextEdit::undo);
    actionUndo->setShortcut(QKeySequence::Undo);
    tb->addAction(actionUndo);

    const QIcon redoIcon = QIcon::fromTheme("edit-redo", QIcon(rsrcPath + "/editredo.png"));
    actionRedo = menu->addAction(redoIcon, tr("&Redo"), textEdit, &QTextEdit::redo);
    actionRedo->setPriority(QAction::LowPriority);
    actionRedo->setShortcut(QKeySequence::Redo);
    tb->addAction(actionRedo);
    menu->addSeparator();

#ifndef QT_NO_CLIPBOARD
    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(rsrcPath + "/editcut.png"));
    actionCut = menu->addAction(cutIcon, tr("Cu&t"), textEdit, &QTextEdit::cut);
    actionCut->setPriority(QAction::LowPriority);
    actionCut->setShortcut(QKeySequence::Cut);
    tb->addAction(actionCut);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(rsrcPath + "/editcopy.png"));
    actionCopy = menu->addAction(copyIcon, tr("&Copy"), textEdit, &QTextEdit::copy);
    actionCopy->setPriority(QAction::LowPriority);
    actionCopy->setShortcut(QKeySequence::Copy);
    tb->addAction(actionCopy);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(rsrcPath + "/editpaste.png"));
    actionPaste = menu->addAction(pasteIcon, tr("&Paste"), textEdit, &QTextEdit::paste);
    actionPaste->setPriority(QAction::LowPriority);
    actionPaste->setShortcut(QKeySequence::Paste);
    tb->addAction(actionPaste);
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
}

void TextEdit::setupUsersActions() {
    this->userListMenu = menuBar()->addMenu(tr("&Users"));

    const QIcon colorBackgroundIcon = QIcon::fromTheme("color-text", QIcon(rsrcPath + "/usercolor.png"));
    actionColorBackground = this->userListMenu->addAction(
                colorBackgroundIcon, tr("&Color Text"), this, &TextEdit::changeIsBackgroundColored);
    QToolBar *tb = addToolBar(tr("Color Actions"));
    tb->addAction(actionColorBackground);

    resetupUsersActions();
}

void TextEdit::resetupUsersActions() {
try {
    this->userListMenu->clear();

    const QIcon colorBackgroundIcon = QIcon::fromTheme("color-text", QIcon(rsrcPath + "/usercolor.png"));
    actionColorBackground = this->userListMenu->addAction(
                colorBackgroundIcon, tr("&Color Text"), this, &TextEdit::changeIsBackgroundColored);
    this->userListMenu->addSeparator();

    // questo per eliminare i cursori vecchi quando si ridisenga la usersActions, conviene trovare un altro modo
    for (User user : this->map_userId_user.values()) {
        user.updateRemoteCursorPosition(-1);
    }
    this->map_userId_user.clear();

    QString filename = this->service->currentFilename;
    QStringList userlist = this->service->userlist();

    for (int i=0; i<userlist.size()/3; i++) {
        int userIdInFile = i;
        ulong userId = userlist[i*3 + 0].toULong();
        QString username = userlist[i*3 + 1];
        int cursorPosition = userlist[i*3 + 2].toInt();
        bool isCurrentUser = (userId == this->service->userId);
        User user(userIdInFile, userId, username, cursorPosition, isCurrentUser, this->textEdit);
        this->map_userId_user.insert(userId, user);

        QString isOnlineStr = " [ME]";
        if (!isCurrentUser) {
            if (user.isOnline) {
                isOnlineStr = " [on]";
            } else {
                isOnlineStr = "";
            }
        }
        QPixmap pix(16, 16);
        pix.fill(user.textColor);

        QAction *a = this->userListMenu->addAction(
                    pix, tr((username + isOnlineStr).toStdString().c_str()), this, &TextEdit::doNothing);
        a->setPriority(QAction::LowPriority);

    }
} catch (...) { goFileListClicked(); }
}

void TextEdit::doNothing() {}

void TextEdit::filePrintPdf() {

    QFileDialog fileDialog(this, tr("Export PDF"));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setMimeTypeFilters(QStringList("application/pdf"));
    fileDialog.setDefaultSuffix("pdf");
    if (fileDialog.exec() != QDialog::Accepted)
        return;
    QString fileName = fileDialog.selectedFiles().first();
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    textEdit->document()->print(&printer);
    statusBar()->showMessage(tr("Exported \"%1\"")
                             .arg(QDir::toNativeSeparators(fileName)));
}

void TextEdit::clipboardDataChanged() {
#ifndef QT_NO_CLIPBOARD
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
}

void TextEdit::cursorPositionChanged() {
    QTextCursor cursor = textEdit->textCursor();
    if (prevCursorPosition != cursor.position()) {
        prevCursorPosition = currentCursorPosition;
        currentCursorPosition = cursor.position();

        this->service->sendCursorPosition(cursor.position());
    }
}

void TextEdit::textChange() {

    textEdit->setFontPointSize(12);
    textEdit->setFont(font);
    drawTextBackground();
    //se il cambiamento è avvenuto a causa di un pacchetto dal server non fare niente
    if(textChangeFromServer == true) {
        textChangeFromServer = false;
        return;
    }

}
