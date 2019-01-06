#pragma once

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
//#include <thread>

#include <QPixmap>
#include <QImage>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>

#include "tcamcamera.h"

#include "threshold.hpp"
#include "cork_config.hpp"
#include "cork_analyser.hpp"
#include "cork.hpp"
#include "camera_input.hpp"
#include "camera_config.hpp"
#include "image_status.hpp"
#include "cvgui.hpp"

#include "mainwindow.hpp"

#define KEY_ESC 27
#define KEY_LEFT 81
#define KEY_RIGHT 83
#define KEY_UP 82
#define KEY_DOWN 84
#define KEY_W 119
#define KEY_S 115

#define PI 3.14159265359

#define DEBUG_GUI true

static bool saveNextFrame = false;
static int saveFrameCounter = 0;

static double defectA = -1.0;
static double defectB = -1.0;

static std::string windowA = "CorkA";
static std::string windowB = "CorkB";

static CorkConfig configA;
static CorkConfig configB;

static auto callbackA = [] (cv::Mat &mat, MainWindow *context) -> void
{
    if(saveNextFrame)
    {
        std::cout << "Save frame" << std::endl;
        saveNextFrame = false;
        cv::imwrite("./" + std::to_string(saveFrameCounter++) + ".png", mat);
    }

    CorkAnalyser::processFrame(mat, &configA, &defectA);

    if(!mat.empty())
    {
        context->ui->camera_a->setPixmap(QPixmap::fromImage(QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888)));
    }

    /*
    if(cv::waitKey(1) == KEY_ESC)
    {
        saveNextFrame = true;
    }
    */
};

static auto callbackB = [] (cv::Mat &mat, MainWindow *context) -> void
{
    CorkAnalyser::processFrame(mat, &configB, &defectB);

    if(!mat.empty())
    {
        context->ui->camera_b->setPixmap(QPixmap::fromImage(QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888)));
    }

    cv::waitKey(1);
};

MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    CameraConfig cameraConfigA;
    cameraConfigA.width = 768;
    cameraConfigA.height = 480;
    cameraConfigA.input = CameraConfig::TCAM;
    cameraConfigA.tcamSerial = "46810320";

    CameraInput *cameraInputA = new CameraInput(cameraConfigA);
    cameraInputA->context = this;
    cameraInputA->frameCallback = 0;

    //TODO <MOVE TO THREAD>
    CameraConfig cameraConfigB;
    cameraConfigB.width = 640;
    cameraConfigB.height = 480;
    cameraConfigB.input = CameraConfig::USB;
    cameraConfigB.usbNumber = 1;

    CameraInput *cameraInputB = new CameraInput(cameraConfigB);
    cameraInputA->context = this;
    cameraInputB->frameCallback = 1;

    cameraInputA->start();
    cameraInputB->start();

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
            //std::cout << "Cork: No cork" << std::endl;
        }
    }

    //cameraInputA->stop();
    //cameraInputB->stop();

    /*
    cv::Mat img = cv::imread("download.png");

    if(!img.empty())
    {
        ui->camera_a->setPixmap(QPixmap::fromImage(QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888)));
        //cv::imshow("Display Image", img);
    }
    else
    {
        std::cout << "Image not open" << std::endl;
    }
*/
}

MainWindow::~MainWindow()
{
    delete ui;
}
