#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int fd;
    bool connected;
    bool logon;
    QString myName;
    void Send(QString qstr);
    void setRDOnly(void);

public slots:
    void PrintPrivateMessage(QString info);
    void PrintChatMessage(QString info);

private slots:

    void on_LoginButton_clicked();

    void on_LogoutButton_clicked();

    void on_RegistrationButton_clicked();

    void on_ChatSendButton_clicked();

    void on_PrivateSendButton_clicked();

    void on_KnockingButton_clicked();

    void on_ChatUploadButton_clicked();

    void on_PrivateUploadButton_clicked();    

    void on_ConnectButton_clicked();

    void setButtons();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
