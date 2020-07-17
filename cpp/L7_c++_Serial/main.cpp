#include <QApplication>
#include "qwidgetecgcom.h"

int main(int argc, char * argv[])
{
    QApplication a(argc, argv);

    QWidgetEcgCom *com = new QWidgetEcgCom();
    com->show();


    return a.exec();
}




