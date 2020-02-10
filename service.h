#ifndef SERVICE_H
#define SERVICE_H
#include <QString>
#include <QTcpSocket>

class Service : public QObject {
    Q_OBJECT
public:
    explicit Service(QObject *parent = nullptr);
    bool connectToServer(QString ipAddress);

    QStringList login(QString emailOrUsername, QString password);
    QStringList signup(QString email, QString username, QString password);
    QStringList logout();
    QStringList newfile(QString filename);
    QStringList addfile(QString uri);
    QStringList openfile(QString filename);
    void closefile(); // non serve il currentFilename perchè il server sa già quale file ho aperto
    QStringList filelist();
    QStringList changeUsername(QString newUsername);
    QStringList changePassword(QString newPassword, QString newPasswordAgain);
    QStringList userlist(); // non serve il currentFilename perchè il server sa già quale file ho aperto
    QStringList addUser(QString username); // non serve il currentFilename perchè il server sa già quale file ho aperto

    void sendUpdateFile(ulong siteCounter, QString fileContent); // non serve il currentFilename perchè il server sa già quale file ho aperto
    void sendCursorPosition(int cursorPosition);

    QString currentFilename;
    ulong userId;
    QString username;

    QTcpSocket *socket;

private:
    QStringList sendMessage(QStringList list);
    QStringList sendMessageWithLongResponse(QStringList list);
    void sendMessageWithoutResponse(QStringList list);

signals:
    void signalUpdateFile(QString fileContent);

};

#endif // SERVICE_H
