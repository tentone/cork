#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <QPixmap>
#include <QImage>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    cv::Mat img = cv::imread("download.png");

    if(!img.empty())
    {
        ui->label_camera_a->setPixmap(QPixmap::fromImage(QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888)));
        //cv::imshow("Display Image", img);
    }
    else
    {
        std::cout << "Image not open" << std::endl;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
