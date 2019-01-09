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

static bool hasCork = false;

static double defectA = -1.0;
static double defectB = -1.0;

static CorkConfig configA;
static CorkConfig configB;

static double sumDefect = 0.0;

static bool running = false;

static int corkCounter = 0;
static int corkRotated = 0;
static int corkDiscarded = 0;

static double averageDefect = 0.0;
static double minimumDefect = 100.0;
static double maximumDefect = 0.0;

static Ui::MainWindow *ui_static;

static void updateGUI()
{
    //Check cork presence
    if(defectA > 0 && defectB > 0)
    {
        if(!hasCork)
        {
            corkCounter++;
            ui_static->text_processed->setText(QString::number(corkCounter));
        }

        hasCork = true;

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
            ui_static->quality_minimum->setText(QString::number(minimumDefect) + "%");
        }
        if(defectB < minimumDefect)
        {
            minimumDefect = defectB;
            ui_static->quality_minimum->setText(QString::number(minimumDefect) + "%");
        }

        //Maximum defect
        if(defectA > maximumDefect)
        {
            maximumDefect = defectA;
            ui_static->quality_maximum->setText(QString::number(maximumDefect) + "%");
        }
        if(defectB > maximumDefect)
        {
            maximumDefect = defectB;
            ui_static->quality_maximum->setText(QString::number(maximumDefect) + "%");
        }

        //Average defect
        //TODO <PREVENT OVERFLOW>
        sumDefect += defectA + defectB;
        corkCounter += 2;
        averageDefect = sumDefect / corkCounter;
        ui_static->quality_average->setText(QString::number(averageDefect) + "%");
    }
    //No cork
    else
    {
        hasCork = false;
        ui_static->label_selected->setText("Sem Rollha");
    }

    //Defect A
    if(defectA > 0.0)
    {
        ui_static->text_defect_a->setText(QString::number(defectA) + "%");
    }
    else
    {
        ui_static->text_defect_a->setText("-");
    }

    //Defect B
    if(defectB > 0.0)
    {
        ui_static->text_defect_b->setText(QString::number(defectB) + "%");
    }
    else
    {
        ui_static->text_defect_b->setText("-");
    }

}

MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent), ui(new Ui::MainWindow)
{
    //Fullscreen
    //QTimer::singleShot(1000, this, SLOT(showFullScreen()));

    //Store GUI
    ui->setupUi(this);
    ui->tab_main->hide();



    ui_static = ui;

    //Settings Button
    connect(ui->button_settings, &QPushButton::clicked, []()
    {
        if(ui_static->tab_main->isHidden())
        {
            ui_static->tab_main->show();
        }
        else
        {
            ui_static->tab_main->hide();
        }
    });

    //Stop and start buttons
    connect(ui->button_stop_start, &QPushButton::clicked, []()
    {
        ui_static->button_settings->hide();
    });

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

    running = true;
}

void MainWindow::stopCapture()
{
    cameraInputA->stop();
    cameraInputB->stop();

    running = true;
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

        //updateGUI();
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

        updateGUI();
    };
}

MainWindow::~MainWindow()
{
    deleteCaptureHandlers();

    delete ui;
}
