
#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#define PROGRAM_NAME "ShareIt! - "

#include <QMainWindow>
#include <QMap>
#include <QPointer>
#include <QStackedWidget>
#include "service.h"
#include "crdtprocessor.h"
#include "user.h"

extern bool textChangeFromServer;
extern int prevCursorPosition;
extern int currentCursorPosition;

QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QFontComboBox;
class QTextEdit;
class QTextCharFormat;
class QMenu;
class QPrinter;
QT_END_NAMESPACE

class TextEdit : public QMainWindow
{
    Q_OBJECT

public:
    QString uri;
    bool textChangeFromServer;
    int prevCursorPosition;
    int currentCursorPosition;
    TextEdit(QStackedWidget *parent = nullptr, Service *service = nullptr, QStringList fileData = {});

    void drawAllCursors();

private slots:
    void filePrintPdf();

    void cursorPositionChanged();

    void clipboardDataChanged();

    void textChange();
    void contentsChange(int position, int charsRemoved, int charsAdded);

    void readyRead();

private:
    void goFileListClicked();
    void showUri();
    void updateEditorContent(QString fileContent);

    void setupFileActions();
    void setupEditActions();
    void setupUsersActions();
    void resetupUsersActions();

    void changeIsBackgroundColored();
    void drawTextBackground();
    void doNothing();
    void drawSingleTextBackground(int startPos, int finishPos, QColor color);

    QAction *actionSave;
    QAction *actionTextColor;
    QAction *actionUndo;
    QAction *actionRedo;

    QAction *actionColorBackground;
    bool isBackgroundColored = false;

    QFont font;

#ifndef QT_NO_CLIPBOARD
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
#endif
    QToolBar *tb;
    QTextEdit *textEdit;

    QMenu *userListMenu;
    Service *service;

    CrdtProcessor *crdtProcessor;

    QMap<ulong, User> map_userId_user;

    QList<QTextEdit::ExtraSelection> _usersText;

    QString messageExcess;

signals:
    void showStackedWidget();

};

#endif // TEXTEDIT_H
