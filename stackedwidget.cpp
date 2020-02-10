#include <QMessageBox>

#include "stackedwidget.h"
#include "ui_stackedwidget.h"
#include "service.h"

StackedWidget::StackedWidget(QWidget *parent) : QStackedWidget(parent), ui(new Ui::StackedWidget) {
    qApp->setStyleSheet("QMessageBox { messagebox-text-interaction-flags: 5; }"); // rende i QMessageBox selezionabili

    ui->setupUi(this);
    this->setCurrentIndex(7);
    this->show();
    service = new Service();
}

StackedWidget::~StackedWidget() {
    delete ui;
}


// funzioni page CONNECT
void StackedWidget::on_connectButton_clicked() {
    QString ipAddress = ui->ipAddressLineEdit->text();
    if (!Util::addressIsValid(ipAddress)) {
        QMessageBox::warning(this, "WARNING", "Insert a valid address");
        return;
    }
    bool connected = this->service->connectToServer(ipAddress);
    if (connected) {
        this->setCurrentIndex(0);
        connect(this->service->socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    } else {
        QMessageBox::warning(this, "WARNING", "Insert a different address");
    }
}

// funzioni page LOGIN
void StackedWidget::on_loginButton_clicked() {
    QString email = ui->loginUsernameOrEmailLineEdit->text();
    QString password = ui->loginPasswordLineEdit->text();

    QStringList response = service->login(email, password);
    if (checkIsOK(response)) {
        cleanupLineEdits();
        service->userId = response[1].toULong();
        service->username = response[2];
        initFilelist();
    }
}
void StackedWidget::on_goSignup_clicked() {
    cleanupLineEdits();
    this->setCurrentIndex(1);
}

// funzioni page SIGNUP
void StackedWidget::on_signupButton_clicked() {
    QString email = ui->signupEmailLineEdit->text();
    QString username = ui->signupUsernameLineEdit->text();
    QString password = ui->signupPasswordLineEdit->text();

    QStringList response = service->signup(email, username, password);
    if (checkIsOK(response)) {
        cleanupLineEdits();
        service->userId = response[1].toULong();
        service->username = response[2];
        initFilelist();
    }
}
void StackedWidget::on_goLogin_clicked() {
    cleanupLineEdits();
    this->setCurrentIndex(0);
}

// funzioni page FILELIST
void StackedWidget::initFilelist() {
    cleanupLineEdits();
    ui->filelistWidget->clear();
    QStringList filelist = service->filelist();
    ui->filelistWidget->addItems(filelist);
    this->setCurrentIndex(2);

    ui->signupLabel_13->setText("HI, " + service->username);
    ui->signupLabel_13->setStyleSheet("QLabel { font-size:14pt; font-weight:600; color:#303f9f; }");
    ui->signupLabel_13->setAlignment(Qt::AlignCenter);
    ui->signupLabel_13->setIndent(-1);
}
void StackedWidget::on_goCreatefile_clicked() {
    this->setCurrentIndex(5);
}
void StackedWidget::on_filelistWidget_itemDoubleClicked(QListWidgetItem *item) {
    openTextEdit(item->text());
}
void StackedWidget::on_goChangeUsername_clicked() {
    this->setCurrentIndex(3);
}
void StackedWidget::on_goChangePassword_clicked() {
    this->setCurrentIndex(4);
}
void StackedWidget::on_goAddFile_clicked() {
    this->setCurrentIndex(6);

}
void StackedWidget::on_logoutButton_clicked() {
    service->logout();
    this->service->currentFilename = nullptr;
    this->setCurrentIndex(0);
}

// funzioni page CHANGEUSERNAME
void StackedWidget::on_changeUsername_clicked() {
    QString newUsername = ui->newUsernameLineEdit->text();
    QStringList response = service->changeUsername(newUsername);
    if (checkIsOK(response)) {
        ui->newUsernameLineEdit->clear();
        this->service->username = newUsername;
        initFilelist();
    }
}
void StackedWidget::on_goFilelist1_clicked() {
    initFilelist();
}

// funzioni page CHANGEPASSWORD
void StackedWidget::on_changePassword_clicked() {
    QString newPassword = ui->newPasswordLineEdit->text();
    QString newPasswordAgain = ui->newPasswordAgainLineEdit->text();
    QStringList response = service->changePassword(newPassword, newPasswordAgain);
    if (checkIsOK(response)) {
        ui->newPasswordLineEdit->clear();
        ui->newPasswordAgainLineEdit->clear();
        initFilelist();
    }
}
void StackedWidget::on_goFilelist2_clicked() {
    initFilelist();
}

// funzioni page CREATEFILE
void StackedWidget::on_createFileButton_clicked() {
    QString filename = ui->fileNameLineEdit->text();
    ui->fileNameLineEdit->clear();
    QStringList response = service->newfile(filename);
    if (checkIsOK(response)) {
        filename = response[1];
        openTextEdit(filename);
    }
}
void StackedWidget::on_goFilelist3_clicked() {
    initFilelist();
}

// funzioni page ADDFILE
void StackedWidget::on_addFileButton_clicked() {
    QString uri = ui->fileUriLineEdit->text();
    ui->fileUriLineEdit->clear();
    QStringList response = service->addfile(uri);
    if (checkIsOK(response)) {
        QString filename = response[1];
        openTextEdit(filename);
    }
}
void StackedWidget::on_goFilelist4_clicked() {
    initFilelist();
}

// funzioni condivise
void StackedWidget::openTextEdit(QString filename) {
    this->hide();
    initFilelist();
    QStringList fileData = service->openfile(filename);
    this->service->currentFilename = filename;

    textEdit = new TextEdit(nullptr, service, fileData);
    textEdit->show();
    textEdit->drawAllCursors();
    connect(textEdit, &TextEdit::showStackedWidget, this, &StackedWidget::show);
}

bool StackedWidget::checkIsOK(QStringList response) {
    if (response[0] == "OK") {
        return true;
    } else {
        QString warningMessage;
        if (response.size() == 1) {
            warningMessage = response[0];
        } else {
            warningMessage = response[1];
        }
        QMessageBox::warning(this, "WARNING", warningMessage);
        return false;
    }
}

void StackedWidget::cleanupLineEdits() {
    ui->loginPasswordLineEdit->clear();
    ui->loginUsernameOrEmailLineEdit->clear();
    ui->signupEmailLineEdit->clear();
    ui->signupPasswordLineEdit->clear();
    ui->signupUsernameLineEdit->clear();
    ui->newUsernameLineEdit->clear();
    ui->newPasswordAgainLineEdit->clear();
    ui->newPasswordLineEdit->clear();
    ui->fileNameLineEdit->clear();
    //ui->addUsernameLineEdit->clear();
    ui->fileUriLineEdit->clear();
}

void StackedWidget::disconnected() {
    this->setCurrentIndex(7);
    if (this->textEdit != nullptr) {
        this->textEdit->deleteLater();
    }
    this->show();
}
