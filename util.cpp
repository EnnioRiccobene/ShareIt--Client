#include <QMessageBox>
#include <QApplication>
#include "util.h"

const QString FIELD_SEPARATOR = "\r\n";
const bool isDebug = false;


void Util::showErrorAndClose(QString error) {
    QMessageBox::warning(nullptr, "FATAL ERROR", error);
    QApplication::quit();
}

// le funzioni sotto sono in comune tra server e client
QByteArray Util::serialize(QStringList qStringList) {
    QString message = "";
    for(QString elem : qStringList) {
        message += elem + FIELD_SEPARATOR;
    }
    message += FIELD_SEPARATOR + FIELD_SEPARATOR + FIELD_SEPARATOR;
    return message.toUtf8();
}
QStringList Util::deserialize(QString opcode, QByteArray qByteArray) {
    QString message = QString(qByteArray);

    // rimuovo tutto ciò che potrebbe esserci prima dell'opcode
    message = opcode + message.split(opcode)[1];

    QStringList qStringList = message.split(FIELD_SEPARATOR);
    QStringList final;
    for (int i = 0; i < qStringList.size()-4; i++) {
        final += qStringList[i];
    }
    return final;
}
QList<QStringList> Util::deserializeList(QByteArray qByteArray) {
    QString message = QString(qByteArray);
    QStringList listOfLists = message.split(FIELD_SEPARATOR + FIELD_SEPARATOR + FIELD_SEPARATOR + FIELD_SEPARATOR);
    QList<QStringList> ret;
    for (int i=0; i<listOfLists.size()-1; i++) {
        QStringList qStringList = listOfLists[i].split(FIELD_SEPARATOR);
        ret.append(qStringList);
    }
    // se il messaggio viene ricevuto correttamente finalPart è una stringa vuota,
    // altrimenti è ciò che rimane dopo l'ultimo delimitatore
    QString finalPart = listOfLists[listOfLists.size()-1];
    ret.append({finalPart});
    return ret;
}

bool Util::usernameIsValid(QString username) {
    return isDebug || QRegExp("^[a-zA-Z0-9]{4,}$").exactMatch(username);
}
bool Util::emailIsValid(QString email) {
    return isDebug || QRegExp("^([\\w\\.\\-]+)@([\\w\\-]+)((\\.(\\w){2,3})+)$").exactMatch(email);
}
bool Util::passwordIsValid(QString password) {
    return isDebug || QRegExp("^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d]{8,}$").exactMatch(password);
}
bool Util::emailOrUsernameIsValid(QString emailOrUsername) {
    return usernameIsValid(emailOrUsername) || emailIsValid(emailOrUsername);
}
bool Util::filenameIsValid(QString filename) {
    return isDebug || QRegExp("^[\\w,\\s-]+$").exactMatch(filename);
}
bool Util::addressIsValid(QString address) {
    return QRegExp("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)[.]){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$|^localhost$").exactMatch(address);
}
