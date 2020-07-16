#include <QApplication>
#include "qwidgetserialtx.h"

int main(int argc, char * argv[])
{
    QApplication a(argc, argv);

    QWidgetSerialTx *com = new QWidgetSerialTx();

    return a.exec();
}




