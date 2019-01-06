#include <QApplication>

#include "lib/tcamcamera.h"

#include "mainwindow.hpp"

int main(int argc, char** argv)
{
    gst_init(&argc, &argv);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
