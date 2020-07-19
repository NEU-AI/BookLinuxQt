#include "mainwindow.h"
#include <QApplication>
#include <QTcpServer>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QDebug>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QIODevice>
#include <QThread>
#include <process.h>

#include <QThread>

class MyThread : public QThread
{
public:
    MyThread(QObject *parent = nullptr):QThread(parent){}
    QString name;
protected:
    void run();//多线程执行的内容将通过重新该虚函数实现
};

void MyThread::run()
{
    int count=0;
    qDebug()<<"Thread "<<this->name<<" run:"<<QThread::currentThread();

    while(1)
    {
        QThread::sleep(1);
        count++;
        qDebug()<<"Thread "<<this->name<<count<<" in"<<QThread::currentThread();;
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MyThread th1;
    th1.name = "thread one";
    th1.start();//启动线程
    MyThread th2;
    th2.name = "thread two";
    th2.start();//启动线程

    return a.exec();
}
