#pragma once

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <QPixmap>
#include <QImage>
#include <QThread>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "camera_input.hpp"
#include "camera_config.hpp"

#include "mainwindow.hpp"

static double defectA = -1.0;
static double defectB = -1.0;

static CorkConfig configA;
static CorkConfig configB;

static Ui::MainWindow *ui_static;

MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui_static = ui;

    CameraConfig cameraConfigA;
    cameraConfigA.width = 768;
    cameraConfigA.height = 480;
    cameraConfigA.input = CameraConfig::TCAM;
    cameraConfigA.tcamSerial = "46810320";

    CameraInput *cameraInputA = new CameraInput(cameraConfigA);
    cameraInputA->frameCallback = [] (cv::Mat &mat) -> void
    {
        CorkAnalyser::processFrame(mat, &configA, &defectA);

        if(!mat.empty())
        {
            ui_static->camera_a->setPixmap(QPixmap::fromImage(QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGBA8888)));
        }

        if(defectA > 0 && defectB > 0)
        {
            if(defectA > defectB)
            {
                std::cout << "Cork: Select B" << std::endl;
            }
            else if(defectA < defectB)
            {
                std::cout << "Cork: Select A" << std::endl;
            }
        }
    };

    CameraConfig cameraConfigB;
    cameraConfigB.width = 640;
    cameraConfigB.height = 480;
    cameraConfigB.input = CameraConfig::USB;
    cameraConfigB.usbNumber = 1;

    CameraInput *cameraInputB = new CameraInput(cameraConfigB);
    cameraInputB->frameCallback = [] (cv::Mat &mat) -> void
    {
        CorkAnalyser::processFrame(mat, &configB, &defectB);

        if(!mat.empty())
        {
            ui_static->camera_b->setPixmap(QPixmap::fromImage(QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888)));
        }
    };

    cameraInputA->start();
    cameraInputB->start();

    /*
    while(true)
    {
        //cameraInputA->update();
        cameraInputB->update();

        if(defectA > 0 && defectB > 0)
        {
            if(defectA > defectB)
            {
                std::cout << "Cork: Select B" << std::endl;
            }
            else if(defectA < defectB)
            {
                std::cout << "Cork: Select A" << std::endl;
            }
        }
        else
        {
            std::cout << "Cork: No cork" << std::endl;
        }

        CameraInput::wait();
    }*/

    //cameraInputA->stop();
    //cameraInputB->stop();
}

MainWindow::~MainWindow()
{
    delete ui;
}
