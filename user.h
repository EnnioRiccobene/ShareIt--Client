#ifndef USER_H
#define USER_H

#include <QString>
#include <QLabel>
#include <QTextCursor>
#include <QTextEdit>

class User {
public:
    User(int userIdInFile, ulong userId, QString username, int cursorPosition,
         bool isCurrentUser, QTextEdit *textEdit);

    QTextEdit *textEdit;

    // userIdInFile rispecchia l'ordine di aggiunta degli utenti al file:
    //      il creatore ha userIdInFile=0, il primo aggiunto ha userIdInFile=1 ...
    int userIdInFile;
    ulong userId;
    QString username;
    int cursorPosition;
    bool isCurrentUser;

    bool isOnline;

    QColor cursorColor;
    QColor textColor;

    QTextCursor* userCursor;
    QLabel* userCursorLabel;

    bool updateRemoteCursorPosition(int newCursorPosition);

};

#endif // USER_H
