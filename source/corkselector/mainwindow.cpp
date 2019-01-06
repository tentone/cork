#pragma once

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <QPixmap>
#include <QImage>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "camera_input.hpp"
#include "camera_config.hpp"

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
    cameraInputA->side = 0;

    CameraConfig cameraConfigB;
    cameraConfigB.width = 640;
    cameraConfigB.height = 480;
    cameraConfigB.input = CameraConfig::USB;
    cameraConfigB.usbNumber = 1;

    CameraInput *cameraInputB = new CameraInput(cameraConfigB);
    cameraInputA->context = this;
    cameraInputB->side = 1;

    cameraInputA->start();
    cameraInputB->start();

    /*
    //TODO <MOVE TO THREAD>
    while(true)
    {
        cameraInputA->update();
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

    cameraInputA->stop();
    cameraInputB->stop();
    */
}

MainWindow::~MainWindow()
{
    delete ui;
}
