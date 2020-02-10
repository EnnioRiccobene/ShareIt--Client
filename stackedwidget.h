#ifndef STACKEDWIDGET_H
#define STACKEDWIDGET_H

#include <QStackedWidget>
#include <QListWidgetItem>
#include "textedit.h"
#include "service.h"

namespace Ui {
    class StackedWidget;
}

class StackedWidget : public QStackedWidget {
    Q_OBJECT
public:
    explicit StackedWidget(QWidget *parent = nullptr);
    ~StackedWidget();
private:
    TextEdit *textEdit = nullptr;
    Service *service;
    Ui::StackedWidget *ui;
    void openTextEdit(QString filename);
    void initFilelist();
    bool checkIsOK(QStringList response);
    void cleanupLineEdits();

private slots:
    void on_connectButton_clicked();
    void on_goSignup_clicked();
    void on_goLogin_clicked();
    void on_loginButton_clicked();
    void on_signupButton_clicked();

    void on_goChangeUsername_clicked();
    void on_goChangePassword_clicked();
    void on_goCreatefile_clicked();
    void on_goAddFile_clicked();
    void on_logoutButton_clicked();
    void on_filelistWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_changeUsername_clicked();
    void on_changePassword_clicked();
    void on_createFileButton_clicked();
    void on_addFileButton_clicked();

    void on_goFilelist1_clicked();
    void on_goFilelist2_clicked();
    void on_goFilelist3_clicked();
    void on_goFilelist4_clicked();

    void disconnected();
};

#endif // STACKEDWIDGET_H
