#ifndef QWIDGETSERIALTX_H
#define QWIDGETSERIALTX_H

#include <QWidget>
#include <QtSerialPort/QSerialPort>
#include <QTimer>

class QWidgetSerialTx : public QWidget
{
    Q_OBJECT
public:
    QSerialPort *com;
    QTimer *timer;
    int ecgDataIndex = 0;

    explicit QWidgetSerialTx(QWidget *parent = nullptr);

    char bccCheck(char *data, int len);
    int serialInit(void);
    void sendEcgPkg();

signals:

};

#endif // QWIDGETSERIALTX_H
