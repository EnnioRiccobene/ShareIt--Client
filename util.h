#ifndef UTIL_H
#define UTIL_H

#include <QByteArray>
#include <QStringList>

class Util
{
public:
    // le funzioni sotto sono in comune tra server e client
    static QByteArray serialize(QStringList qStringList);
    static QStringList deserialize(QString opcode, QByteArray qByteArray);
    static QList<QStringList> deserializeList(QByteArray qByteArray);
    static bool usernameIsValid(QString username);
    static bool emailIsValid(QString email);
    static bool passwordIsValid(QString password);
    static bool emailOrUsernameIsValid(QString emailOrUsername);
    static bool filenameIsValid(QString filename);
    static void showErrorAndClose(QString error);
    static bool addressIsValid(QString address);
};

#endif // UTIL_H
