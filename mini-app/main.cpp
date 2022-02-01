#include "miniapp.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    miniapp w;
    w.show();
    return a.exec();
}
