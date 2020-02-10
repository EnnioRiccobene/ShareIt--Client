#include "user.h"

User::User(int userIdInFile, ulong userId, QString username,
           int cursorPosition, bool isCurrentUser, QTextEdit *textEdit)
        : userIdInFile(userIdInFile), userId(userId), username(username),
          cursorPosition(cursorPosition), isCurrentUser(isCurrentUser) {

    this->textEdit = textEdit;

    // Ottengo i colori dall'userIdInFile
    if(userIdInFile == 0){
        //creator
        textColor = Qt::red;
        cursorColor = Qt::darkRed;
    } else if(userIdInFile == 1 || ((userIdInFile - 1) % 5) == 0){
        textColor = Qt::green;
        cursorColor = Qt::darkGreen;
    } else if(userIdInFile == 2 || ((userIdInFile - 2) % 5) == 0){
        textColor = QColor(68, 119, 186);
        cursorColor = Qt::darkBlue;
    } else if(userIdInFile == 3 || ((userIdInFile - 3) % 5) == 0){
        textColor = Qt::yellow;
        cursorColor = Qt::darkYellow;
    } else if(userIdInFile == 4 || ((userIdInFile - 4) % 5) == 0){
        textColor = Qt::magenta;
        cursorColor = Qt::darkMagenta;
    } else if(userIdInFile == 5 || (userIdInFile % 5) == 0){
        textColor = Qt::cyan;
        cursorColor = Qt::darkCyan;
    }

    userCursor = new QTextCursor(textEdit->document());
    userCursorLabel = new QLabel(textEdit);
    QPixmap pix(4, 26);
    pix.fill(cursorColor);
    if (isCurrentUser) {
        pix.fill(Qt::transparent);
        userCursorLabel->hide();
    }
    userCursorLabel->setPixmap(pix);

    if (cursorPosition == -1) {
        isOnline = false;
    } else {
        isOnline = true;
    }

}

// TODO forse questa funzione si può sistemare meglio
// ritorna true se l'utente è passato online/offline, ovvero devo ridisegnare la lista degli utenti
bool User::updateRemoteCursorPosition(int newCursorPosition) {
//    userCursorLabel->hide();
    if (this->isCurrentUser) {
        return false;
    }
    cursorPosition = newCursorPosition;

    if (newCursorPosition == -1) {
        userCursorLabel->hide();
        if (isOnline) {
            isOnline = false;
            return true;
        } else {
            return false;
        }

    }

    newCursorPosition = qMin(newCursorPosition, textEdit->toPlainText().size());
    userCursor->setPosition(newCursorPosition);
    const QRect qRect = textEdit->cursorRect(*userCursor);
    userCursorLabel->move(qRect.left(), qRect.top());
    userCursorLabel->show();

    if (!isOnline) {
        isOnline = true;
        return true;
    } else {
        return false;
    }
}



