#include <QString>
#include <QRegExp>
#include <QDebug>
#include <QBuffer>

#include "service.h"
#include "util.h"

const QString FIELD_SEPARATOR = "\r\n";

Service::Service(QObject *parent) : QObject(parent) {
    socket = new QTcpSocket(this);
}

bool Service::connectToServer(QString ipAddress) {
    socket->connectToHost(ipAddress, 1234);
    if(!socket->waitForConnected(2000)) {
        qDebug() << "Error: " << socket->errorString();
        return false;
    } else {
        socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1048576);
        return true;
    }
}

QStringList Service::login(QString emailOrUsername, QString password) {
    if (!Util::emailOrUsernameIsValid(emailOrUsername)) {
        return {"ERR", "Please enter a valid email or a valid username"};
    } else if (!Util::passwordIsValid(password)) {
        return {"ERR", "The password must have minimum eight characters, at least one letter and one number"};
    } else {
        return sendMessage({"oplogin", emailOrUsername, password});
    }
}

QStringList Service::signup(QString email, QString username, QString password) {
    if (!Util::emailIsValid(email)) {
        return {"ERR", "Please enter a valid email"};
    } else if (!Util::usernameIsValid(username.trimmed())) {
        return {"ERR", "Username should be at least 4 characters long, only letters and numbers"};
    } else if (!Util::passwordIsValid(password)) {
        return {"ERR", "The password must have minimum eight characters, at least one letter and one number"};
    } else {
        return sendMessage({"opsignup", email, username, password});
    }
}
QStringList Service::logout() {
    return sendMessage({"oplogout"});
}
QStringList Service::newfile(QString filename) {
    if (!Util::filenameIsValid(filename)) {
        return {"ERR", "The name of the file must contain only letters, numbers, - and _"};
    }
    return sendMessage({"opnewfile", filename});
}
QStringList Service::addfile(QString uri) {
    if (uri.isEmpty() || uri.split("/").size() != 3) {
        return {"ERR", "Please enter a valid link"};
    } else {
        return sendMessage({"opaddfile", uri});
    }
}
QStringList Service::openfile(QString filename) {
    return sendMessageWithLongResponse({"opopenfile", filename}); // aspetta per 20 secondi, il file può essere grosso
}
void Service::closefile() {
    sendMessageWithoutResponse({"opclosefile"});
}

QStringList Service::filelist() {
    return sendMessage({"opfilelist"});
}
QStringList Service::changeUsername(QString newUsername) {
    if (!Util::usernameIsValid(newUsername.trimmed())) {
        return {"ERR", "Username should be at least 4 characters long, only letters and numbers"};
    } else {
        return sendMessage({"opchangeusername", newUsername});
    }
}
QStringList Service::changePassword(QString newPassword, QString newPasswordAgain) {
    if (newPassword != newPasswordAgain) {
        return {"ERR", "The two passwords must be equal"};
    } else if (!Util::passwordIsValid(newPassword)) {
        return {"ERR", "The password must have minimum eight characters, at least one letter and one number"};
    } else {
        return sendMessage({"opchangepassword", newPassword});
    }
}
QStringList Service::userlist() {
    // ritorna [userId, username, cursorPosition] per ogni utente
    return sendMessage({"opuserlist"});
}
QStringList Service::addUser(QString username) {
    return sendMessage({"opadduser", username});
}

void Service::sendUpdateFile(ulong siteCounter, QString message) {
    sendMessageWithoutResponse({"opsendupdate", QString::number(siteCounter), message});
}
void Service::sendCursorPosition(int cursorPosition) {
    sendMessageWithoutResponse({"opcursorpos", QString::number(cursorPosition)});
}

QStringList Service::sendMessage(QStringList list) {
    // prende in input una lista di QString, le manda al server, e ritorna la risposta
    QByteArray request = Util::serialize(list);
    socket->write(request);

//    qDebug() << "Io (client) mando: ---" << request;
    if(socket->waitForReadyRead(3000)) { // aspetto 3 secondi
        QByteArray byteArray = socket->readAll();

        QString opcode = list[0];
        QStringList response = Util::deserialize(opcode, byteArray);
//        qDebug() << "Io (client) ho mandato: ---" << request << "--- e il server ha risposto ---" << response.join(", ");
        response.removeAt(0);
        return response;
    } else {
//        qDebug() << "TIMEOUT";
        return {"TIMEOUT"};
    }
}

QStringList Service::sendMessageWithLongResponse(QStringList list) {
    QByteArray request = Util::serialize(list);
    socket->write(request);
//    qDebug() << "Io (client) mando: ---" << request;

    QByteArray totalByteArray;
    int wholePacketSize = -1;
    while (true) {
        if(socket->waitForReadyRead(10000)) {
            QByteArray byteArray = socket->readAll();
            totalByteArray += byteArray;

            if (wholePacketSize == -1) {
                QString resp = byteArray.mid(0, 100);
                wholePacketSize = resp.split(FIELD_SEPARATOR)[1].toInt();
            }
            if (totalByteArray.size() >= wholePacketSize) {
                break;
            }

        } else {
//            qDebug() << "TIMEOUT";
            return {"TIMEOUT"};
        }
    }

    QString opcode = list[0];
    QStringList response = Util::deserialize(opcode, totalByteArray);
//    qDebug() << "Io (client) ho mandato: ---" << request << "--- e il server ha risposto ---";
    response.removeAt(0);
    return response;


}

void Service::sendMessageWithoutResponse(QStringList list) {
    QByteArray update = Util::serialize(list);
    socket->write(update);
}
