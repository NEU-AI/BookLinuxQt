#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QMainWindow>
#include <QTcpSocket>
#include <QHostAddress>
#include <QMessageBox>
#include <QIODevice>


class TcpClient : public QMainWindow
{
public:
    explicit TcpClient(QWidget *parent = 0) : QMainWindow(parent)
    {
        tcpClient = new QTcpSocket(this);   //实例化tcpClient
        tcpClient->abort();                 //取消原有连接
    }

    ~TcpClient(){
        tcpClient->abort();                 //取消原有连接
        delete tcpClient;
    }
public:
    QHostAddress ip;
    quint16 port;

public:
    QTcpSocket *tcpClient;
    void ConServer();
    qint64 SentData(const char* data, qint64 len);
    void DisconServer();
};


void TcpClient::ConServer()
{
    tcpClient->connectToHost(ip, port);
    if (tcpClient->waitForConnected(1000))  // 连接成功则进入if{}
    {
        qDebug()<<"connected.....";
        return;
    }
    qDebug("connect failed!");
}

void TcpClient::DisconServer()
{
    qDebug()<<"try to disconnect.....";
    tcpClient->disconnectFromHost();
    if (tcpClient->state() == QAbstractSocket::UnconnectedState \
            || tcpClient->waitForDisconnected(1000))  //已断开连接则进入if{}
    {
        qDebug()<<"disconnected!";
        return;
    }
    qDebug()<<"disconnect failed";
}


qint64 TcpClient::SentData(const char* data, qint64 len)
{
    qDebug()<<"sent data.....";
    qint64 slen=tcpClient->write(data,len);
    qDebug()<<"send "<<slen<<" bytes";
    return slen;
}

int main(int argc, char *argv[])
{
    char data[]={(char)0x38,(char)0x36,(char)0x37,(char)0x35};
    QApplication a(argc, argv);
    TcpClient tc;
    tc.ip= "127.0.0.1";
    tc.port=8888;//8989;
    //qDebug()<<tc.tcpClient->state();
    tc.ConServer();
    //qDebug()<<tc.tcpClient->state();
    if(tc.tcpClient->state() == QAbstractSocket::ConnectedState)
        tc.SentData(data,sizeof(data));
    while(1)
        if(tc.tcpClient->state() == QAbstractSocket::ConnectedState)
            if( tc.tcpClient->waitForReadyRead())
            {
                qDebug()<<"read data from server.....";
                QByteArray buffer = tc.tcpClient->readAll();
                if(!buffer.isEmpty())
                {
                    qDebug()<<"data size "<<buffer.size();
                    qDebug()<<buffer;
                }
            }
            else
                qDebug()<<"read data timeout. try again";
        else
            break;
    tc.DisconServer();

    return a.exec();
}
