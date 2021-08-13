#include "LogUi.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LogUi w;
    w.show();
    return a.exec();
}
