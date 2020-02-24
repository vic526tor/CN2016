#include "mainwindow.h"
#include <QApplication>
#include <QObject>
#include <cstdio>
#include <cstdlib>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <QtCore>

int fd;

const int LOGIN        = 0;
const int LOGOUT       = 1;
const int REGISTRATION = 2;
const int KNOCKING     = 3;
const int MESSAGE      = 4;
const int FILED        = 5;

const int SUCCESS      = 0;
const int WRONG_PASS   = 1;
const int NO_SUCH_ID   = 2;
const int ID_EXIST     = 1;
const int ONLINE       = 0;
const int OFFLINE      = 1;
const int CHAT         = 1;
const int PRIVATE      = 0;
MainWindow* wptr;

class Thread : public QThread
{
    Q_OBJECT
public:
    void run();
    void Login(char buf[]);
    void Logout(char buf[]);
    void Registration(char buf[]);
    void Knocking(char buf[]);
    void Message(char buf[]);
    void File(char buf[]);
    void Send_File(char buf[]);

signals:
    void DisplayPrivateMessage(QString info);
    void DisplayChatMessage(QString info);
    void SetButtons();
};

void Thread::run()
{
    char buf[256];
    int t = 0;
    while(!wptr->connected){;}
    fd = wptr->fd;
    while(1){
        read(fd, buf, 256);
        sscanf(buf, "%d", &t);
        if(t == LOGIN) Login(buf);
        else if(t == LOGOUT){
            emit DisplayPrivateMessage("SYSTEM: logout");
            wptr->logon =false;
            emit SetButtons();
        }
        else if(t == REGISTRATION) Registration(buf);
        else if(t == KNOCKING) Knocking(buf);
        else if(t == MESSAGE) Message(buf);
        else if(t == FILED) File(buf);
        else{
            printf("error\n");
            exit(0);
        }
    }
    this->thread()->quit();
}

void Thread::Login(char buf[])
{
    int a, b;
    sscanf(buf, "%d%d", &a, &b);
    if(b == SUCCESS){
        emit DisplayPrivateMessage("SYSTEM: login as "+wptr->myName);
        wptr->logon = true;
        emit SetButtons();
    }
    else if(b == WRONG_PASS) emit DisplayPrivateMessage("SYSTEM: wrong password");
    else emit DisplayPrivateMessage("SYSTEM: no such ID");
    return;
}

void Thread::Registration(char buf[])
{
    int a, b;
    sscanf(buf, "%d%d", &a, &b);
    if(b == ID_EXIST) emit DisplayPrivateMessage("SYSTEM: ID already exist");
    else{
        emit DisplayPrivateMessage("SYSTEM: registration success, login as "+wptr->myName);
        wptr->logon = true;
        emit SetButtons();
    }
    return;
}

void Thread::Knocking(char buf[])
{
    int a, b;
    char knockee[20];
    sscanf(buf, "%d%d%s%[^\n]", &a, &b, knockee);
    if(b == ONLINE) emit DisplayPrivateMessage("SYSTEM: "+QString::fromLatin1(knockee, -1)+" is online");
    else if(b == OFFLINE) emit DisplayPrivateMessage("SYSTEM: "+QString::fromLatin1(knockee, -1)+" is offline");
    else emit DisplayPrivateMessage("SYSTEM: cannot find ID "+QString::fromLatin1(knockee, -1));
    return;
}

void Thread::Message(char buf[])
{
    QString str, str_1, str_2, str_3;
    char sender[20], receiver[20], msg[200];
    int t, c;
    sscanf(buf, "%d%d%s%s%[^\n]", &t, &c, sender, receiver, msg);
    str_1 = QString::fromLatin1(sender,-1);
    str_2 = QString::fromLatin1(receiver,-1);
    str_3 = QString::fromLatin1(msg,-1);
    str="";
    if(c == 2){//offline
        str = "(offline)";
    }
    if(c == 2 || c == PRIVATE){
        if(str_2 == wptr->myName)
            str+= str_1 +": " + str_3;
        else
            str+= str_1 + ": @" + str_2 + " " + str_3;
        DisplayPrivateMessage(str);
    }
    else{
        str = str_1 + ": " + str_3;
        DisplayChatMessage(str);
    }
    return;
}

void Thread::File(char buf[])
{
    QString str, str_1, str_2, str_3;
    char sender[20], receiver[20], filename[150];
    int wfd, t, c, size, num_of_buf, remain;
    int i;
    sscanf(buf, "%d%d%s%s%s%d", &t, &c, sender, receiver, filename, &size);
    QString qsender = QString::fromLatin1(sender);
    if(qsender == wptr->myName){
        Send_File(buf);
        return;
    }
    if(size % 256 == 0){
        num_of_buf = size/256;
        remain = 256;
    }
    else{
        num_of_buf = size/256 + 1;
        remain = size % 256;
    }

    str_1 = QString::fromLatin1(sender,-1);
    str_2 = QString::fromLatin1(receiver,-1);
    str_3 = QString::fromLatin1(basename(filename),-1);

    wfd = open(basename(filename), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(c == PRIVATE){
        str = "SYSTEM: receiving file \""+str_3+"\" from "+str_1+"...";
        DisplayPrivateMessage(str);
    }
    else{
        str = "SYSTEM: receiving file \""+str_3+"\" from "+str_1+"...";
        DisplayChatMessage(str);
    }
    for(i = 0; i < num_of_buf; i++){
        if(i == (num_of_buf - 1)){
            read(fd, buf, remain);
            write(wfd, buf, remain);
        }
        else{
            read(fd, buf, 256);
            write(wfd, buf, 256);
        }
    }
    close(wfd);
    if(c == PRIVATE)DisplayPrivateMessage("SYSTEM: completed");
    else DisplayChatMessage("SYSTEM: competed");
    return;
}

void Thread::Send_File(char buf[])
{
    int rfd, size, num_of_buf, remain;
    int i, a, b;
    char sender[20], receiver[20], filename[150];
    sscanf(buf, "%d%d%s%s%s%d", &a, &b, sender, receiver, filename, &size);
    rfd = open(filename, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(rfd<0){
        DisplayChatMessage("SYSTEM: cannot open file "+QString::fromLatin1(basename(filename),-1));
        return;
    }
    if(b==PRIVATE)DisplayPrivateMessage("SYSTEM: sending file \""+QString::fromLatin1(basename(filename),-1)+"\" to "+receiver+"...");
    else DisplayChatMessage("SYSTEM: sending file \""+QString::fromLatin1(basename(filename),-1)+"\"...");
    if(size % 256 == 0){
        num_of_buf = size/256;
        remain = 256;
    }
    else{
        num_of_buf = size/256 + 1;
        remain = size % 256;
    }
    for(i = 0; i < num_of_buf; i++){
        if(i == num_of_buf-1){
            read(rfd, buf, remain);
            write(fd, buf, remain);
        }
        else{
            read(rfd, buf, 256);
            write(fd, buf, 256);
        }
    }
    ::close(rfd);
    if(b == PRIVATE)DisplayPrivateMessage("SYSTEM: completed");
    else DisplayChatMessage("SYSTEM: completed");
    return;
}

int main(int argc, char *argv[])
{        
    QApplication a(argc, argv);
    MainWindow w;
    Thread t;

    QObject::connect(&t, SIGNAL(DisplayPrivateMessage(QString)), &w, SLOT(PrintPrivateMessage(QString)));
    QObject::connect(&t, SIGNAL(DisplayChatMessage(QString)), &w, SLOT(PrintChatMessage(QString)));
    QObject::connect(&t, SIGNAL(SetButtons()), &w, SLOT(setButtons()));
    QObject::connect(&t, SIGNAL(finished()), &w, SLOT(close()));


    w.setRDOnly();
    wptr = &w;
    t.start();
    w.show();
    return a.exec();
}

#include "main.moc"
