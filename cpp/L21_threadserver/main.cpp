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

class TcpSocket : public QTcpSocket
{
public:
    TcpSocket(QTcpSocket *parent = NULL): QTcpSocket(parent){}
    ~TcpSocket(){}

public slots:
    void ReadData();
    void onDisconnectedSlotThread();
    qint64 SendData(TcpSocket *socket,const char * data, qint64 len);
    QByteArray m_buffer;
};

void TcpSocket::ReadData()
{
    TcpSocket *socket = (TcpSocket*)sender();
    QByteArray buffer = socket->readAll();
    if(buffer.isEmpty())    return;
    qDebug()<<"ReadData:"<<buffer<<" in "<<QThread::currentThread();
    m_buffer.append(buffer);
    //以下代码仅供调试
    SendData(socket,buffer,buffer.size());//echo to sender
}

void TcpSocket::onDisconnectedSlotThread()
{
    qDebug()<<"onDisconnectedSlotThread.pid:"<<QThread::currentThread();
}

qint64 TcpSocket::SendData(TcpSocket* socket,const char * data, qint64 len)
{
    qint64 slen;
    slen=socket->write(data,len); //qt5除去了.toAscii()
    qDebug()<<"SendData len:"<<slen<<"data:"<<data;
    return slen;
}

class TcpServer:public QObject
{
public:
    QList<QHostAddress> localaddrs;
    TcpServer(QWidget *parent = 0):QObject(parent)
    {
        tcpServer = new QTcpServer(this);
        //获得本机可用地址清单.
        localaddrs = QNetworkInterface::allAddresses();
        foreach(QHostAddress ha, localaddrs)
        {
            qDebug()<<ha.toString();
        }
        ip="127.0.0.1";//本地loop地址
        port=8888;

        connect(tcpServer, &QTcpServer::newConnection, this, &TcpServer::NewConnectionSlot);
    }

    ~TcpServer()
    {
        delete tcpServer;
    }
public:
    QHostAddress ip;
    quint16 port;
    bool SvStart();
private:
    QTcpServer *tcpServer;

public slots:
    void NewConnectionSlot();
};

void TcpServer::NewConnectionSlot()
{
    qDebug()<<"New Connection Slot coming...";
    QTcpSocket *socket = tcpServer->nextPendingConnection();
    qDebug()<<"ip:"<<socket->peerAddress().toString()<<":"<<socket->peerPort();

    //建立新线程来处理后续数据
    TcpSocket *threadSocket= new TcpSocket();
    threadSocket->setSocketDescriptor(socket->socketDescriptor());
    QThread *thread = new QThread(threadSocket);
    connect(threadSocket, &QAbstractSocket::disconnected, thread, &QThread::quit);
    connect(threadSocket, &QAbstractSocket::disconnected, threadSocket, &TcpSocket::onDisconnectedSlotThread);
    connect(threadSocket, &QIODevice::readyRead, threadSocket, &TcpSocket::ReadData);
    threadSocket->moveToThread(thread);
    thread->start();
}

bool TcpServer::SvStart()
{
    qDebug()<<"try to start tcpserver at"<<ip.toString()<<":"<<port;
    bool ok = tcpServer->listen(ip, port);
     if(ok)
         qDebug("tcp server started!");
     else
         qDebug("tcp server start failed!");
     return ok;
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TcpServer ts;
    ts.SvStart();//启动服务器

    return a.exec();
}
