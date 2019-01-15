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

static int SCREEN_MAIN = 100;
static int SCREEN_SETTINGS = 101;

//Wich screen is currently being shown
static int screen;

//Indicates if the camera capture is running
static bool running = false;

static bool hasCork = false;
static double defectA = -1.0;
static double defectB = -1.0;

/**
 * Camera A configuration object.
 */
static CameraConfig *cameraConfigA;

/**
 * Camera B configuration object.
 */
static CameraConfig *cameraConfigB;

/**
 * Camera A cork detection configuration.
 */
static CorkConfig *configA;

/**
 * Camera B cork detection configuration.
 */
static CorkConfig *configB;

static int corkCounter = 0;
static int corkRotated = 0;
static int corkDiscarded = 0;

static double sumDefect = 0.0;

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

void MainWindow::fillSettingsUI()
{
    //Camera A configuration
    int tabAInputIndex = ui->tab_a_input->findData(cameraConfigA->input);
    if(tabAInputIndex != -1)
    {
       ui->tab_a_input->setCurrentIndex(tabAInputIndex);
    }

    ui->tab_a_ip->setText(QString::fromStdString(cameraConfigA->ipAddress));
    ui->tab_a_tcam_serial->setText(QString::fromStdString(cameraConfigA->tcamSerial));
    ui->tab_a_usb->setValue(cameraConfigA->usbNumber);
    ui->tab_a_width->setValue(cameraConfigA->width);
    ui->tab_a_height->setValue(cameraConfigA->height);
    ui->tab_a_width_original->setValue(cameraConfigA->originalWidth);
    ui->tab_a_height_original->setValue(cameraConfigA->originalHeight);

    int tabAVideoIndex = ui->tab_a_input->findData(cameraConfigA->videoBackend);
    if(tabAVideoIndex != -1)
    {
       ui->tab_a_input->setCurrentIndex(tabAVideoIndex);
    }

    //Detector A configuration
    ui->tab_a_ppi->setValue(configA->ppi);
    ui->tab_a_size_min->setValue(configA->minSize);
    ui->tab_a_size_max->setValue(configA->maxSize);
    ui->tab_a_threshold->setValue(configA->thresholdValue);
    ui->tab_a_canny_high->setValue(configA->highCannyThresh);
    ui->tab_a_canny_low->setValue(configA->lowCannyThresh);
    ui->tab_a_threshold_tolerance->setValue(configA->semiAutoThreshTolerance);
    ui->tab_a_outside_skirt->setValue(configA->outsizeSkirt);
    ui->tab_a_shadow->setChecked(configA->rgb_shadow);
}

void MainWindow::setScreen(int _screen)
{
    screen = _screen;

    if(screen == SCREEN_MAIN)
    {
        ui->group_home->show();
        ui->group_settings->hide();
    }
    else if(screen == SCREEN_SETTINGS)
    {
        fillSettingsUI();
        ui->group_settings->show();
        ui->group_home->hide();
    }
}

void MainWindow::initializeGUI()
{
    //Tab A camera input options
    ui->tab_a_input->clear();
    ui->tab_a_input->addItem("Ficheiro", CameraConfig::FILE);
    ui->tab_a_input->addItem("USB", CameraConfig::USB);
    ui->tab_a_input->addItem("TCam", CameraConfig::TCAM);
    ui->tab_a_input->addItem("IP", CameraConfig::IP);

    //Tab A camera video backend options
    ui->tab_a_video_backend->clear();
    ui->tab_a_video_backend->addItem("Any", cv::CAP_ANY);
    ui->tab_a_video_backend->addItem("V4L", cv::CAP_V4L);
    ui->tab_a_video_backend->addItem("FFMPEG", cv::CAP_FFMPEG);
    ui->tab_a_video_backend->addItem("GStreamer", cv::CAP_GSTREAMER);
    ui->tab_a_video_backend->addItem("XINE Engine", cv::CAP_XINE);
    ui->tab_a_video_backend->addItem("Aravis", cv::CAP_ARAVIS);
    ui->tab_a_video_backend->addItem("OpenNI", cv::CAP_OPENNI);
    ui->tab_a_video_backend->addItem("OpenNI 2", cv::CAP_OPENNI2);
    ui->tab_a_video_backend->addItem("Firewire", cv::CAP_FIREWIRE);

    //Tab A camera callbacks
    connect(ui->tab_a_tcam_serial, &QLineEdit::textChanged, this, [=](const QString &text)
    {
        cameraConfigA->tcamSerial = text.toStdString();
        std::cout << "Cork: A Tcam Serial, " << cameraConfigA->tcamSerial << std::endl;
    });

    connect(ui->tab_a_ip, &QLineEdit::textChanged, this, [=](const QString &text)
    {
        cameraConfigA->ipAddress = text.toStdString();
        std::cout << "Cork: A IP Address, " << cameraConfigA->ipAddress << std::endl;
    });

    //Tab B camera input options
    ui->tab_b_input->clear();
    ui->tab_b_input->addItem("Ficheiro", CameraConfig::FILE);
    ui->tab_b_input->addItem("USB", CameraConfig::USB);
    ui->tab_b_input->addItem("TCam", CameraConfig::TCAM);
    ui->tab_b_input->addItem("IP", CameraConfig::IP);

    //Tab A camera video backend options
    ui->tab_b_video_backend->clear();
    ui->tab_b_video_backend->addItem("Any", cv::CAP_ANY);
    ui->tab_b_video_backend->addItem("V4L", cv::CAP_V4L);
    ui->tab_b_video_backend->addItem("FFMPEG", cv::CAP_FFMPEG);
    ui->tab_b_video_backend->addItem("GStreamer", cv::CAP_GSTREAMER);
    ui->tab_b_video_backend->addItem("XINE Engine", cv::CAP_XINE);
    ui->tab_b_video_backend->addItem("Aravis", cv::CAP_ARAVIS);
    ui->tab_b_video_backend->addItem("OpenNI", cv::CAP_OPENNI);
    ui->tab_b_video_backend->addItem("OpenNI 2", cv::CAP_OPENNI2);
    ui->tab_b_video_backend->addItem("Firewire", cv::CAP_FIREWIRE);
}

MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent), ui(new Ui::MainWindow)
{
    //Fullscreen
    //QTimer::singleShot(1000, this, SLOT(showFullScreen()));

    //Store GUI
    ui->setupUi(this);
    ui_static = ui;

    //Initialize GUI
    initializeGUI();

    //Settings Button
    QObject::connect(ui->button_settings, &QPushButton::clicked, [=]()
    {
        setScreen(screen == SCREEN_MAIN ? SCREEN_SETTINGS : SCREEN_MAIN);
    });

    //Stop and start buttons
    QObject::connect(ui->button_stop_start, &QPushButton::clicked, [=]()
    {
        if(running)
        {
            stopCapture();
        }
        else
        {
            startCapture();
        }
    });

    //Cork A configuration
    configA = new CorkConfig();

    //Camera A configuration
    cameraConfigA = new CameraConfig();
    cameraConfigA->originalWidth = 1920;
    cameraConfigA->originalHeight = 1200;
    cameraConfigA->width = 768;
    cameraConfigA->height = 480;
    cameraConfigA->input = CameraConfig::TCAM;
    cameraConfigA->tcamSerial = "46810320";

    //Cork B configuration
    configB = new CorkConfig();

    //Camera B configuration
    cameraConfigB = new CameraConfig();
    cameraConfigB->width = 640;
    cameraConfigB->height = 480;
    cameraConfigB->input = CameraConfig::USB;
    cameraConfigB->videoBackend = cv::CAP_ANY;
    cameraConfigB->usbNumber = 0;

    //Set main screen
    setScreen(SCREEN_MAIN);

    //Start camera capture
    createCaptureHandlers();
    startCapture();
}


void MainWindow::startCapture()
{
    try
    {
        cameraInputA->start();
        cameraInputB->start();
        running = true;

        ui->button_stop_start->setText("Parar");
    }
    catch (const std::exception& e)
    {
        std::cout << "Cork: Error starting the camera capture" << std::endl;
    }

}

void MainWindow::stopCapture()
{
    try
    {
        cameraInputA->stop();
        cameraInputB->stop();
        running = false;

        ui->button_stop_start->setText("Iniciar");
        ui->camera_a->clear();
        ui->camera_b->clear();
    }
    catch (const std::exception& e)
    {
        std::cout << "Cork: Error stoping the camera capture" << std::endl;
    }
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
        CorkAnalyser::processFrame(mat, configA, &defectA);

        //TODO <MAYBE MOVE TO ANOTHER THREAD>
        if(!mat.empty())
        {
            cv::Mat resized, color;
            cv::cvtColor(mat, color, cv::COLOR_BGRA2RGB);
            cv::resize(color, resized, cv::Size(ui_static->camera_a->width(), ui_static->camera_a->height()), 0, 0, cv::INTER_CUBIC);
            ui_static->camera_a->setPixmap(QPixmap::fromImage(QImage(resized.data, resized.cols, resized.rows, resized.step, QImage::Format_RGB888)));
        }
    };

    cameraInputB = new CameraInput(cameraConfigB);
    cameraInputB->frameCallback = [] (cv::Mat &mat) -> void
    {
        CorkAnalyser::processFrame(mat, configB, &defectB);

        //TODO <MAYBE MOVE TO ANOTHER THREAD>
        if(!mat.empty())
        {
            cv::Mat resized, color;
            cv::cvtColor(mat, color, cv::COLOR_BGR2RGB);
            cv::resize(color, resized, cv::Size(ui_static->camera_b->width(), ui_static->camera_b->height()), 0, 0, cv::INTER_CUBIC);
            ui_static->camera_b->setPixmap(QPixmap::fromImage(QImage(resized.data, resized.cols, resized.rows, resized.step, QImage::Format_RGB888)));
        }

        //TODO <MAYBE MOVE TO ANOTHER THREAD>
        updateGUI();
    };
}

MainWindow::~MainWindow()
{
    deleteCaptureHandlers();

    delete ui;
}
