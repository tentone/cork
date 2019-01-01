#include <QApplication>

#include <opencv2/opencv.hpp>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    cv::Mat inputImage = cv::imread("download.png");

    if(!inputImage.empty())
    {
        cv::imshow("Display Image", inputImage);
    }
    else
    {
        std::cout << "Image not open" << std::endl;
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
