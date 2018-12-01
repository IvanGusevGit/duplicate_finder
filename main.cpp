#include "main_widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    main_widget w;
    w.show();

    return a.exec();
}
