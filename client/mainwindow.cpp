#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>
#include <QDateTime>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connected = false;
    logon = false;
    setButtons();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_ConnectButton_clicked()
{
    QString serverStr = ui->server->text();
    QString port = ui->port->text();
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int portno = port.toInt();
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
       PrintPrivateMessage("SYSTEM: failed to open socket");
       return;
    }
    server = gethostbyname(serverStr.toStdString().c_str());
    if (server == NULL) {
       PrintPrivateMessage("SYSTEM: cannot find server "+serverStr);
       return;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if(::connect(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
       PrintPrivateMessage("SYSTEM: failed to connect to "+serverStr+":"+port);
       return;
    }
    PrintPrivateMessage("SYSTEM: connected to server "+serverStr);
    connected = true;
    setButtons();
    return;
}
void MainWindow::on_LoginButton_clicked()
{
    QString username = ui->username->text();
    QString password = ui->password->text();
    QString str = "0 " + username + " " + password + "\n";
    Send(str);
    myName = username;
    return;
}

void MainWindow::on_LogoutButton_clicked()
{
    QString str = "1\n";
    Send(str);
    return;
}

void MainWindow::on_RegistrationButton_clicked()
{
    QString username = ui->username->text();
    QString password = ui->password->text();
    QString str = "2 " + username + " " + password + "\n";
    Send(str);
    myName = username;
    return;
}

void MainWindow::on_ChatSendButton_clicked()
{
    QString receiver = "everybody";
    QString msg = ui->chat_msg->text();
    QString str = "4 1 " + myName + " " + receiver + " " + msg + "\n";
    Send(str);
    return;
}

void MainWindow::on_PrivateSendButton_clicked()
{
    QString receiver = ui->ID->text();
    QString msg = ui->private_msg->text();
    QString str = "4 0 " + myName + " " + receiver + " " + msg + "\n";
    Send(str);
    return;
}

void MainWindow::on_KnockingButton_clicked()
{
    QString ID = ui->ID->text();
    QString str = "3 " + ID + "\n";
    Send(str);
    return;
}

void MainWindow::on_ChatUploadButton_clicked()
{
    QString qfile = QFileDialog::getOpenFileName(this, tr("Open File"), "./", tr("All Files (*)"));
    const char* cfile = qfile.toStdString().c_str();
    QString receiver = "everybody";
    struct stat st;
    int rfd, size;
    rfd = open(cfile, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(rfd<0){
        PrintChatMessage("SYSTEM: cannot open "+qfile);
        return;
    }
    fstat(rfd, &st);
    ::close(rfd);
    size = st.st_size;
    QString s = QString::number(size);
    QString str = "5 1 " + myName + " " + receiver + " " + qfile + " " + s + "\n";
    Send(str);

    return;
}

void MainWindow::on_PrivateUploadButton_clicked()
{
    QString qfile = QFileDialog::getOpenFileName(this, tr("Open File"), "./", tr("All Files (*)"));
    const char* cfile = qfile.toStdString().c_str();
    QString receiver = ui->ID->text();
    QByteArray ba = qfile.toLatin1();
    struct stat st;
    int rfd, size;
    rfd = open(cfile, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(rfd<0){
        PrintPrivateMessage("SYSTEM: cannot open "+qfile);
        return;
    }
    fstat(rfd, &st);
    ::close(rfd);
    size = st.st_size;
    QString s = QString::number(size);
    QString str = "5 0 " + myName + " " + receiver + " " + qfile + " " + s + "\n";
    Send(str);


    return;
}

void MainWindow::PrintPrivateMessage(QString info){
    QDateTime current = QDateTime::currentDateTime();
    if(strncmp(info.toStdString().c_str(), "(off", 4)==0)
        ui->PrivateRoom->append(info);
    else
        ui->PrivateRoom->append(current.toString("hh:mm")+" "+info);
    return;
}

void MainWindow::PrintChatMessage(QString info){
    QDateTime current = QDateTime::currentDateTime();
    ui->ChatRoom->append(current.toString("hh:mm")+" "+info);
    return;
}

void MainWindow::Send(QString qstr){
    QByteArray ba = qstr.toLatin1();
    const char* cstr = ba.data();
    write(fd, cstr, strlen(cstr));
    return;
}

void MainWindow::setRDOnly(void){
    ui->ChatRoom->setReadOnly(true);
    ui->PrivateRoom->setReadOnly(true);
}

void MainWindow::setButtons(){
    ui->ChatSendButton->setEnabled(logon);
    ui->ChatUploadButton->setEnabled(logon);
    ui->PrivateSendButton->setEnabled(logon);
    ui->PrivateUploadButton->setEnabled(logon);
    ui->KnockingButton->setEnabled(logon);
    ui->LogoutButton->setEnabled(logon);
    ui->username->setEnabled(!logon);
    ui->password->setEnabled(!logon);
    ui->LoginButton->setEnabled(connected&&!logon);
    ui->RegistrationButton->setEnabled(connected&&!logon);
    ui->ConnectButton->setEnabled(!connected);
    ui->server->setEnabled(!connected);
    ui->port->setEnabled(!connected);
}
