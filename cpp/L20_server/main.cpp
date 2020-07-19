#include "mainwindow.h"
#include <QApplication>
#include <QTcpServer>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QDebug>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QIODevice>

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
    qint64 SendDataAll(const char * data, qint64 len);
    qint64 SendData(QTcpSocket *tcpClient,const char * data, qint64 len);
private:
    QTcpServer *tcpServer;
    QList<QTcpSocket*> tcpClient;
    QTcpSocket *currentClient;

public slots:
    void NewConnectionSlot();
    void onDisconnectedSlot();
    void onReadData();
};

void TcpServer::NewConnectionSlot()
{
    qDebug()<<"New Connection Slot coming...";
    currentClient = tcpServer->nextPendingConnection();
    tcpClient.append(currentClient);
    connect(currentClient, &QIODevice::readyRead, this, &TcpServer::onReadData);
    connect(currentClient, &QAbstractSocket::disconnected, this, &TcpServer::onDisconnectedSlot);

    qDebug()<<"ip:"<<currentClient->peerAddress().toString()<<":"<<currentClient->peerPort();
}

void TcpServer::onDisconnectedSlot()
{
    qDebug()<<"onDisconnectedSlot";
    //由于disconnected信号并未提供SocketDescriptor，所以需要遍历寻找
    for(int i=0; i<tcpClient.length(); i++)
    {
        if(tcpClient[i]->state() == QAbstractSocket::UnconnectedState)
        {
            qDebug()<<"disconnect ip:"<<currentClient->peerAddress().toString()<<":"<<currentClient->peerPort();
            // 删除存储在tcpClient列表中的客户端信息
             tcpClient[i]->destroyed();
             tcpClient.removeAt(i);
        }
    }
}

void TcpServer::onReadData()
{
    qDebug()<<"onReadData";

    //方式1:
    // 遍历所有客户端,批量处理
    for(int i=0; i<tcpClient.length(); i++)
    {
        QByteArray buffer = tcpClient[i]->readAll();
        if(buffer.isEmpty())    continue;

        static QString IP_Port, IP_Port_Pre;
        qDebug()<<"read data ip:"<<currentClient->peerAddress().toString()<<":"<<currentClient->peerPort();
        qDebug()<<buffer;

        //以下代码仅供调试
        //SendDataAll(buffer,buffer.size());//echo to all
        SendData(tcpClient[i],buffer,buffer.size());//echo to sender
    }

    //方式2:
    //根据发送信号的socket直接读取数据
//    QTcpSocket *socket=qobject_cast<QTcpSocket *>(sender());
//    qDebug()<<"from socket "<<socket->peerPort();
//    QByteArray buffer = socket->readAll();
//    static QString IP_Port, IP_Port_Pre;
//    qDebug()<<"read data ip:"<<socket->peerAddress().toString()<<":"<<socket->peerPort();
//    qDebug()<<buffer;

}

//给所有连接发数据
qint64 TcpServer::SendDataAll(const char * data, qint64 len)
{
    qint64 slen;
    //全部连接
    for(int i=0; i<tcpClient.length(); i++){
        slen=tcpClient[i]->write(data,len); //qt5除去了.toAscii()
        qDebug()<<"SendDataAll len:"<<slen<<"data:"<<data;
    }
}

qint64 TcpServer::SendData(QTcpSocket *tcpClient,const char * data, qint64 len)
{
    qint64 slen;
    slen=tcpClient->write(data,len); //qt5除去了.toAscii()
    qDebug()<<"SendData len:"<<slen<<"data:"<<data;
    return slen;
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
