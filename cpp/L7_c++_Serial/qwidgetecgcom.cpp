#include "qwidgetecgcom.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QDateTime>


QWidgetEcgCom::QWidgetEcgCom(QWidget *parent) : QWidget(parent)
{
    this->status=-1;
    this->serialInit();
}

int QWidgetEcgCom::serialInit(void)
{
    // 打印串口信息
    // foreach: 标准C++中没有这个关键字
    // qt自己实现了这个关键字，如下会便利availablePorts，并把每个结果依次赋值给info，有点像for
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        qDebug() << "Name : " << info.portName();
        qDebug() << "Description : " << info.description();
        qDebug() << "Manufacturer: " << info.manufacturer();
        qDebug() << "Serial Number: " << info.serialNumber();
    }//执行完应该在应用程序输出位置看到串口数据，需要先启动串口模拟器，并创建串口

    // 创建串口对象, 在头文件中定义了指针
    com = new QSerialPort();
    // 配置串口4, 根据创建串口号自行选择
    com->setPortName("COM1");
    // 尝试打开串口
    if(!com->open(QIODevice::ReadWrite))
    {
        qDebug("serial open err");
        return -1; //C程序中的返回值，一般正常返回时，返回0，异常为非零
    }
    else
        qDebug()<<"serial open success:"<<com->portName();

    // 配置串口参数
    // 波特率=115200, 每秒钟发送115200个bit，方向=双向均采用这个波特率
    com->setBaudRate(QSerialPort::Baud115200, QSerialPort::AllDirections);
    // 数据位
    com->setDataBits(QSerialPort::Data8);
    // 流控=无
    com->setFlowControl(QSerialPort::NoFlowControl);
    // 校验位=无
    com->setParity(QSerialPort::NoParity);
    // 停止位=1
    com->setStopBits(QSerialPort::OneStop);

    // 关联信号与槽函数
    // 当串口发出"可读"信号时，执行serialRx函数
    connect(com, &QSerialPort::readyRead, this, &QWidgetEcgCom::serialRx);

    return 0;
}

void QWidgetEcgCom::serialRx()
{
#if 0 //调试过程
    qDebug()<<"SerialRx";
    QByteArray rxDataArr = com->readAll();
    qDebug()<<rxDataArr;
#endif
    QByteArray data = 0;

    // 读串口数据并处理
    while (!com->atEnd())
    {
        QByteArray data = this->com->read(1);
        this->rxDataHandle(data[0]);
        if (this->status == 4)
        {
            // 发送信号画波形
            // todo

            // 计算心率
            ecgBpmCalc(ecg2);
        }
    }
}

void QWidgetEcgCom::rxDataHandle(char data)
{
    char dataTmp;
    // 状态机
    // 状态0：空闲状态， 接收到包头08后进入状态1
    // 状态1：接收数据头状态，收到的数据存入数据头，同时进入下一状态
    // 状态2：数据接收状态，用数据头中的相应数据为填补接收数据的最高位，并缓存数据，接收到特定数据长度后进入下一状态
    // 状态3：数据校验，接收到的数据为校验值，取数据低7为与其他数据部分的低7位进行验证

//    qDebug("%02X",data&0xFF);
//    return;
    data = data&0x0FF;
    int datamask = data&0x80;
    if(datamask == 0x00)//最高位为0
        this->status = 0;
    //qDebug("status:%d,data:%02X,datamask:%02X",this->status,data&0xFF,datamask);

    switch(this->status)
    {
        case 0:
            if (data == 0x08)//ECG
            {
                this->status = 1;
                this->pkgHead = data;
                this->pkgDataCnt = 0;
                qDebug()<<"get pkgHead";
            }else
            {
                qDebug()<<"unknown cmdId"<<data;
            }
            break;
        case 1:
            this->status = 2;
            this->pkgDataHead = data;
            qDebug()<<"get pkgDataHead";
            break;
        case 2:
            dataTmp = data & 0x7F; //保留数据的第7位
            // 第1个数据的最高位在数据头的最低位中
            dataTmp = dataTmp | (((this->pkgDataHead >> this->pkgDataCnt) & 0x01) << 7);
            this->pkgData[this->pkgDataCnt] = dataTmp; // 缓存解析后的数据
            this->pkgDataBuf[this->pkgDataCnt] = data; // 缓存解析前的数据
            // 计数器中的值为数据头后面有效数据的字符数
            this->pkgDataCnt = this->pkgDataCnt + 1;

            // 根据通信协议可知数据长度最大为7
            if (this->pkgDataCnt >= 7)
                this->status = 3;

            break;
        case 3:
            // BCC（异或校验，一个简单的算法）
            // 不提供，有学生自己设计校验方法
            /*
             * if data !=  this->bccCheck(this->pkgData, this->pkgDataCnt)
             *      this->status = 0;
             *      return ;
             */

            // 得到ecg数据
            ecg1 = this->pkgData[0];
            ecg1 = (ecg1 |((pkgDataHead &0x01)<<7 ))  <<8;
            ecg1 = ecg1 | ((this->pkgData[1] | ((pkgDataHead &0x02)<<6))&0xFF);

            ecg2 = this->pkgData[2];
            //qDebug("ecg2=%02X mask=%02X ",ecg2,((pkgDataHead &0x04)<<5 ));
            ecg2 = (ecg2 |((pkgDataHead &0x04)<<5 ))  <<8;
            //qDebug("ecg2=%02X mask=%02X ",ecg2,((pkgDataHead &0x08)<<4));
            ecg2 = ecg2 | ((this->pkgData[3] | ((pkgDataHead &0x08)<<4))&0xFF);
            //qDebug("ecg2=%02X",ecg2);

            ecg3 = this->pkgData[4];
            ecg3 = (ecg3 |((pkgDataHead &0x10)<<3 ))  <<8;
            ecg3 = ecg3 | ((this->pkgData[5] | ((pkgDataHead &0x20)<<2))&0xFF);

            // 调试用数据打印，绘制波形时应注释掉，不然会打印很多数据
            qDebug()<<"ecg1="<<ecg1<<"ecg2="<<ecg2<<"ecg3="<<ecg3;

            this->status = 4;
            break;
    }
}

// data为待校验数据
// len为数据长度
// 返回值为校验结果
char QWidgetEcgCom::bccCheck(char *data, int len)
{
    // todo
    return 0;
}

/*
 * 心率检测简易算法
 * 已知采样间隔，每秒500次
 * 取1-2个周期的最大值，计算最大值与2048的中值作为阈值
 * 计算相邻2次越过阈值时的时间差为脉搏周期
 * 60000/脉搏周期为每分钟脉搏数，即心率
 */
void QWidgetEcgCom::ecgBpmCalc(int ecgData)
{
    // 计算脉搏最高值
    ecg2Buff[ecg2BuffIndex] = ecgData;
    ecg2BuffIndex = ecg2BuffIndex + 1;

    if(ecg2BuffIndex >= sizeof(ecg2Buff)/sizeof(int)) // 缓冲区过大会有问题
    {
        ecg2BuffIndex = 0; // 循环缓冲区，用于计算脉搏中值
        ecg2PulseMax = get_max(ecg2Buff, sizeof(ecg2Buff));
        ecg2PulseMid = (ecg2Zero + ecg2PulseMax)/2; // ecg2Zero 是已知的
    }

    // 找到脉搏
    prePulse = pulse;
    if (ecgData > ecg2PulseMid)
        pulse = 1;
    else
        pulse = 0;

    // 找到脉搏上升沿中值
    if (prePulse == 0 && pulse == 1)
    {
        pulseStart = pulseEnd;
        pulseEnd = QDateTime::currentMSecsSinceEpoch();// current

        // 计算相邻2次脉搏的时间差
        // 求出心率
        ecg2Period = pulseEnd - pulseStart; //
        ecg2Bpm = (int)(60000/ecg2Period); // 每分钟脉搏数量
        qDebug("BPM:%d",ecg2Bpm);
    }
}

int QWidgetEcgCom::get_max(int * data, int len)
{
    // todo
    return 4096;
}


