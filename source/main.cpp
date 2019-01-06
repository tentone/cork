#include <QApplication>

#include "lib/tcamcamera.h"

#include "mainwindow.hpp"

#include <QPixmap>
#include <QImage>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>

#include "lib/tcamcamera.h"

#include "threshold.hpp"
#include "cork_config.hpp"
#include "cork_analyser.hpp"
#include "cork.hpp"
#include "camera_input.hpp"
#include "camera_config.hpp"
#include "image_status.hpp"

int main(int argc, char** argv)
{
    gst_init(&argc, &argv);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
