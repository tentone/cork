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
#include <QTimer>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "camera_input.hpp"
#include "camera_config.hpp"

#include "mainwindow.hpp"

static double defectA = -1.0;
static double defectB = -1.0;

static bool hasCorkA = false;
static bool hasCorkB = false;

static CorkConfig configA;
static CorkConfig configB;

static double sumDefect = 0.0;

static int corkCounter = 0;
static int corkRotated = 0;
static int corkDiscarded = 0;

static double averageDefect = 0.0;
static double minimumDefect = 100.0;
static double maximumDefect = 0.0;

static Ui::MainWindow *ui_static;

static void updateGUI()
{
    if(defectA > 0 && defectB > 0)
    {
        if(defectA > defectB)
        {
            ui_static->label_selected->setText("Rollha B");
        }
        else if(defectA < defectB)
        {
            ui_static->label_selected->setText("Rollha A");
        }

        //Minimum defect
        if(defectA < minimumDefect)
        {
            minimumDefect = defectA;
            //ui_static->quality_minimum->setValue(int(minimumDefect));
        }
        if(defectB < minimumDefect)
        {
            minimumDefect = defectB;
            //ui_static->quality_minimum->setValue(int(minimumDefect));
        }

        //Maximum defect
        if(defectA > maximumDefect)
        {
            maximumDefect = defectA;
            //ui_static->quality_maximum->setValue(int(maximumDefect));
        }
        if(defectB > maximumDefect)
        {
            maximumDefect = defectB;
            //ui_static->quality_maximum->setValue(int(maximumDefect));
        }

        //Average defect
        //TODO <PREVENT OVERFLOW>
        //sumDefect += defectA + defectB;
        //corkCounter += 2;
        //averageDefect = sumDefect / corkCounter;
        //ui_static->quality_average->setValue(int(averageDefect));
    }
    else
    {
        ui_static->label_selected->setText("Sem Rollha");
    }
}

MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent), ui(new Ui::MainWindow)
{
    //Fullscreen
    //QTimer::singleShot(1000, this, SLOT(showFullScreen()));

    ui->setupUi(this);

    ui_static = ui;

    //Camera A configuration
    cameraConfigA.originalWidth = 1920;
    cameraConfigA.originalHeight = 1200;
    cameraConfigA.width = 768;
    cameraConfigA.height = 480;
    cameraConfigA.input = CameraConfig::TCAM;
    cameraConfigA.tcamSerial = "46810320";

    /*
    cameraConfigA.width = 640;
    cameraConfigA.height = 480;
    cameraConfigA.input = CameraConfig::USB;
    cameraConfigA.videoBackend = cv::CAP_ANY;
    cameraConfigA.usbNumber = 0;
    */

    //Camera B configuration
    cameraConfigB.width = 640;
    cameraConfigB.height = 480;
    cameraConfigB.input = CameraConfig::USB;
    cameraConfigB.videoBackend = cv::CAP_ANY;
    cameraConfigB.usbNumber = 1;
    //cameraConfigB.ipAddress = "rtsp://admin:123456@192.168.0.10:554/live/ch0";

    //Start camera capture
    createCaptureHandlers();
    startCapture();
}


void MainWindow::startCapture()
{
    cameraInputA->start();
    cameraInputB->start();
}

void MainWindow::stopCapture()
{
    cameraInputA->stop();
    cameraInputB->stop();
}


void MainWindow::deleteCaptureHandlers()
{
    if(cameraInputA != nullptr)
    {
        cameraInputA->stop();
        delete cameraInputA;
        cameraInputA = nullptr;
    }

    if(cameraInputB != nullptr)
    {
        cameraInputB->stop();
        delete cameraInputB;
        cameraInputB = nullptr;
    }
}


void MainWindow::createCaptureHandlers()
{
    cameraInputA = new CameraInput(cameraConfigA);
    cameraInputA->frameCallback = [] (cv::Mat &mat) -> void
    {
        CorkAnalyser::processFrame(mat, &configA, &defectA);

        if(!mat.empty())
        {
            cv::Mat resized, color;
            cv::cvtColor(mat, color, cv::COLOR_BGRA2RGB);
            cv::resize(color, resized, cv::Size(ui_static->camera_a->width(), ui_static->camera_a->height()), 0, 0, cv::INTER_CUBIC);
            ui_static->camera_a->setPixmap(QPixmap::fromImage(QImage(resized.data, resized.cols, resized.rows, resized.step, QImage::Format_RGB888)));
        }

        if(defectA > 0.0)
        {
            ui_static->text_defect_a->setText(QString::number(defectA) + "%");
        }
        else
        {
            ui_static->text_defect_a->setText("-");
        }

        updateGUI();
    };

    cameraInputB = new CameraInput(cameraConfigB);
    cameraInputB->frameCallback = [] (cv::Mat &mat) -> void
    {
        CorkAnalyser::processFrame(mat, &configB, &defectB);

        if(!mat.empty())
        {
            cv::Mat resized, color;
            cv::cvtColor(mat, color, cv::COLOR_BGR2RGB);
            cv::resize(color, resized, cv::Size(ui_static->camera_b->width(), ui_static->camera_b->height()), 0, 0, cv::INTER_CUBIC);
            ui_static->camera_b->setPixmap(QPixmap::fromImage(QImage(resized.data, resized.cols, resized.rows, resized.step, QImage::Format_RGB888)));
        }

        if(defectB > 0.0)
        {
            ui_static->text_defect_b->setText(QString::number(defectB) + "%");
        }
        else
        {
            ui_static->text_defect_b->setText("-");
        }

        updateGUI();
    };
}

MainWindow::~MainWindow()
{
    deleteCaptureHandlers();

    delete ui;
}
