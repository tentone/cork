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
#include <QThread>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "camera_input.hpp"
#include "camera_config.hpp"

#include "mainwindow.hpp"

#define CAMERA_CONFIG_A_FNAME "cameraConfigA.txt"
#define CAMERA_CONFIG_B_FNAME "cameraConfigB.txt"
#define CONFIG_A_FNAME "configA.txt"
#define CONFIG_B_FNAME "configB.txt"

static int SCREEN_MAIN = 100;
static int SCREEN_SETTINGS = 101;

//Wich screen is currently being shown
static int screen;

//Indicates if the camera capture is running
static bool running = false;

static bool hasCork = false;

static double defectA = -1.0;
static double defectB = -1.0;

static bool writingImageA = false;
static bool writingImageB = false;

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
    //Cork counter
    if(defectA > 0 || defectB > 0)
    {
        if(!hasCork)
        {
            corkCounter++;
            ui_static->text_processed->setText(QString::number(corkCounter));
        }

        hasCork = true;
    }
    else
    {
        hasCork = false;
    }

    //Check cork presence
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
        sumDefect += defectA + defectB;
        corkCounter += 2;
        averageDefect = sumDefect / corkCounter;
        ui_static->quality_average->setText(QString::number(averageDefect) + "%");
    }
    //No cork
    else
    {
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
    ui->tab_a_ppmm->setValue(configA->ppmm);
    ui->tab_a_size_min->setValue(configA->minSize);
    ui->tab_a_size_max->setValue(configA->maxSize);
    ui->tab_a_threshold->setValue(configA->thresholdValue);
    ui->tab_a_canny_high->setValue(configA->highCannyThresh);
    ui->tab_a_canny_low->setValue(configA->lowCannyThresh);
    ui->tab_a_threshold_tolerance->setValue(configA->tresholdTolerance);
    ui->tab_a_outside_skirt->setValue(configA->outsizeSkirt);
    ui->tab_a_shadow->setChecked(configA->rgb_shadow);
    ui->tab_a_automatic_circle->setChecked(configA->automaticCircle);

    //Camera B configuration
    int tabBInputIndex = ui->tab_b_input->findData(cameraConfigB->input);
    if(tabBInputIndex != -1)
    {
       ui->tab_b_input->setCurrentIndex(tabBInputIndex);
    }

    ui->tab_b_ip->setText(QString::fromStdString(cameraConfigB->ipAddress));
    ui->tab_b_tcam_serial->setText(QString::fromStdString(cameraConfigB->tcamSerial));
    ui->tab_b_usb->setValue(cameraConfigB->usbNumber);
    ui->tab_b_width->setValue(cameraConfigB->width);
    ui->tab_b_height->setValue(cameraConfigB->height);
    ui->tab_b_width_original->setValue(cameraConfigB->originalWidth);
    ui->tab_b_height_original->setValue(cameraConfigB->originalHeight);

    int tabBVideoIndex = ui->tab_b_input->findData(cameraConfigB->videoBackend);
    if(tabBVideoIndex != -1)
    {
       ui->tab_b_input->setCurrentIndex(tabBVideoIndex);
    }

    //Detector B configuration
    ui->tab_b_ppmm->setValue(configB->ppmm);
    ui->tab_b_size_min->setValue(configB->minSize);
    ui->tab_b_size_max->setValue(configB->maxSize);
    ui->tab_b_threshold->setValue(configB->thresholdValue);
    ui->tab_b_canny_high->setValue(configB->highCannyThresh);
    ui->tab_b_canny_low->setValue(configB->lowCannyThresh);
    ui->tab_b_threshold_tolerance->setValue(configB->tresholdTolerance);
    ui->tab_b_outside_skirt->setValue(configB->outsizeSkirt);
    ui->tab_b_shadow->setChecked(configB->rgb_shadow);
    ui->tab_b_automatic_circle->setChecked(configB->automaticCircle);
}

void MainWindow::setScreen(int _screen)
{
    screen = _screen;

    if(screen == SCREEN_MAIN)
    {
        ui->group_settings->hide();
    }
    else if(screen == SCREEN_SETTINGS)
    {
        fillSettingsUI();
        ui->group_settings->show();
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
    connect(ui->tab_a_input, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](const int &value)
    {
        cameraConfigA->input = ui->tab_a_input->itemData(value).toInt();
        cameraConfigA->saveFile(CAMERA_CONFIG_A_FNAME);
        std::cout << "Cork: A USB, " << cameraConfigA->input << std::endl;
    });

    connect(ui->tab_a_video_backend, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](const int &value)
    {
        cameraConfigA->videoBackend = ui->tab_a_video_backend->itemData(value).toInt();
        cameraConfigA->saveFile(CAMERA_CONFIG_A_FNAME);
        std::cout << "Cork: A backend, " << cameraConfigA->videoBackend << std::endl;
    });

    connect(ui->tab_a_usb, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        cameraConfigA->usbNumber = value;
        cameraConfigA->saveFile(CAMERA_CONFIG_A_FNAME);
        std::cout << "Cork: A USB, " << cameraConfigA->usbNumber << std::endl;
    });

    connect(ui->tab_a_width, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        cameraConfigA->width = value;
        cameraConfigA->saveFile(CAMERA_CONFIG_A_FNAME);
        std::cout << "Cork: A Width, " << cameraConfigA->width << std::endl;
    });

    connect(ui->tab_a_height, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        cameraConfigA->height = value;
        cameraConfigA->saveFile(CAMERA_CONFIG_A_FNAME);
        std::cout << "Cork: A Height, " << cameraConfigA->height << std::endl;
    });

    connect(ui->tab_a_width_original, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        cameraConfigA->originalWidth = value;
        cameraConfigA->saveFile(CAMERA_CONFIG_A_FNAME);
        std::cout << "Cork: A Original Width, " << cameraConfigA->originalWidth << std::endl;
    });

    connect(ui->tab_a_height_original, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        cameraConfigA->originalHeight = value;
        cameraConfigA->saveFile(CAMERA_CONFIG_A_FNAME);
        std::cout << "Cork: A Original Height, " << cameraConfigA->originalHeight << std::endl;
    });

    connect(ui->tab_a_tcam_serial, &QLineEdit::textChanged, [=](const QString &text)
    {
        cameraConfigA->tcamSerial = text.toStdString();
        cameraConfigA->saveFile(CAMERA_CONFIG_A_FNAME);
        std::cout << "Cork: A Tcam Serial, " << cameraConfigA->tcamSerial << std::endl;
    });

    connect(ui->tab_a_ip, &QLineEdit::textChanged, [=](const QString &text)
    {
        cameraConfigA->ipAddress = text.toStdString();
        cameraConfigA->saveFile(CAMERA_CONFIG_A_FNAME);
        std::cout << "Cork: A IP Address, " << cameraConfigA->ipAddress << std::endl;
    });

    //Tab A cork configuration
    connect(ui->tab_a_ppmm, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        configA->ppmm = value;
        std::cout << "Cork: A PPI, " << configA->ppmm << std::endl;
    });

    connect(ui->tab_a_size_min, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        configA->minSize = value;
        std::cout << "Cork: A Min size, " << configA->minSize << std::endl;
    });

    connect(ui->tab_a_size_max, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        configA->maxSize = value;
        std::cout << "Cork: A Max size, " << configA->maxSize << std::endl;
    });

    connect(ui->tab_a_canny_high, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        configA->highCannyThresh = value;
        std::cout << "Cork: A Canny high, " << configA->highCannyThresh << std::endl;
    });

    connect(ui->tab_a_canny_low, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        configA->lowCannyThresh = value;
        std::cout << "Cork: A Canny low, " << configA->lowCannyThresh << std::endl;
    });

    connect(ui->tab_a_threshold, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        configA->thresholdValue = value;
        std::cout << "Cork: A threshold, " << configA->thresholdValue << std::endl;
    });

    connect(ui->tab_a_threshold_tolerance, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](const double &value)
    {
        configA->tresholdTolerance = value;
        std::cout << "Cork: A threshold tolerance, " << configA->tresholdTolerance << std::endl;
    });

    connect(ui->tab_a_outside_skirt, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        configA->outsizeSkirt = value;
        std::cout << "Cork: A outside skirt, " << configA->outsizeSkirt << std::endl;
    });

    connect(ui->tab_a_shadow, static_cast<void (QCheckBox::*)(bool)>(&QCheckBox::toggled), [=](const bool &value)
    {
        configA->rgb_shadow = value;
        std::cout << "Cork: A RGB shadow, " << configA->rgb_shadow << std::endl;
    });

    connect(ui->tab_a_automatic_circle, static_cast<void (QCheckBox::*)(bool)>(&QCheckBox::toggled), [=](const bool &value)
    {
        configA->automaticCircle = value;
        std::cout << "Cork: A Auto circle, " << configA->automaticCircle << std::endl;
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

    //Tab B camera callbacks
    connect(ui->tab_b_input, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](const int &value)
    {
        cameraConfigB->input = ui->tab_b_input->itemData(value).toInt();
        cameraConfigB->saveFile(CAMERA_CONFIG_B_FNAME);
        std::cout << "Cork: B USB, " << cameraConfigB->input << std::endl;
    });

    connect(ui->tab_b_video_backend, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](const int &value)
    {
        cameraConfigB->videoBackend = ui->tab_b_video_backend->itemData(value).toInt();
        cameraConfigB->saveFile(CAMERA_CONFIG_B_FNAME);
        std::cout << "Cork: B backend, " << cameraConfigB->videoBackend << std::endl;
    });

    connect(ui->tab_b_usb, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        cameraConfigB->usbNumber = value;
        cameraConfigB->saveFile(CAMERA_CONFIG_B_FNAME);
        std::cout << "Cork: B USB, " << cameraConfigB->usbNumber << std::endl;
    });

    connect(ui->tab_b_width, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        cameraConfigB->width = value;
        cameraConfigB->saveFile(CAMERA_CONFIG_B_FNAME);
        std::cout << "Cork: B Width, " << cameraConfigB->width << std::endl;
    });

    connect(ui->tab_b_height, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        cameraConfigB->height = value;
        cameraConfigB->saveFile(CAMERA_CONFIG_B_FNAME);
        std::cout << "Cork: B Height, " << cameraConfigB->height << std::endl;
    });

    connect(ui->tab_b_width_original, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        cameraConfigB->originalWidth = value;
        cameraConfigB->saveFile(CAMERA_CONFIG_B_FNAME);
        std::cout << "Cork: B Original Width, " << cameraConfigB->originalWidth << std::endl;
    });

    connect(ui->tab_b_height_original, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        cameraConfigB->originalHeight = value;
        cameraConfigB->saveFile(CAMERA_CONFIG_B_FNAME);
        std::cout << "Cork: B Original Height, " << cameraConfigB->originalHeight << std::endl;
    });

    connect(ui->tab_b_tcam_serial, &QLineEdit::textChanged, [=](const QString &text)
    {
        cameraConfigB->tcamSerial = text.toStdString();
        cameraConfigB->saveFile(CAMERA_CONFIG_B_FNAME);
        std::cout << "Cork: B Tcam Serial, " << cameraConfigB->tcamSerial << std::endl;
    });

    connect(ui->tab_b_ip, &QLineEdit::textChanged, [=](const QString &text)
    {
        cameraConfigB->ipAddress = text.toStdString();
        cameraConfigB->saveFile(CAMERA_CONFIG_B_FNAME);
        std::cout << "Cork: B IP Address, " << cameraConfigB->ipAddress << std::endl;
    });

    //Tab B cork configuration
    connect(ui->tab_b_ppmm, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        configB->ppmm = value;
        std::cout << "Cork: B PPI, " << configB->ppmm << std::endl;
    });

    connect(ui->tab_b_size_min, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        configB->minSize = value;
        std::cout << "Cork: B Min size, " << configB->minSize << std::endl;
    });

    connect(ui->tab_b_size_max, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        configB->maxSize = value;
        std::cout << "Cork: B Max size, " << configB->maxSize << std::endl;
    });

    connect(ui->tab_b_canny_high, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        configB->highCannyThresh = value;
        std::cout << "Cork: B Canny high, " << configB->highCannyThresh << std::endl;
    });

    connect(ui->tab_b_canny_low, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        configB->lowCannyThresh = value;
        std::cout << "Cork: B Canny low, " << configB->lowCannyThresh << std::endl;
    });

    connect(ui->tab_b_threshold, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        configB->thresholdValue = value;
        std::cout << "Cork: B threshold, " << configB->thresholdValue << std::endl;
    });

    connect(ui->tab_b_threshold_tolerance, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](const double &value)
    {
        configB->tresholdTolerance = value;
        std::cout << "Cork: B threshold tolerance, " << configB->tresholdTolerance << std::endl;
    });

    connect(ui->tab_b_outside_skirt, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](const int &value)
    {
        configB->outsizeSkirt = value;
        std::cout << "Cork: B outside skirt, " << configB->outsizeSkirt << std::endl;
    });

    connect(ui->tab_b_shadow, static_cast<void (QCheckBox::*)(bool)>(&QCheckBox::toggled), [=](const bool &value)
    {
        configB->rgb_shadow = value;
        std::cout << "Cork: B RGB shadow, " << configB->rgb_shadow << std::endl;
    });

    connect(ui->tab_b_automatic_circle, static_cast<void (QCheckBox::*)(bool)>(&QCheckBox::toggled), [=](const bool &value)
    {
        configB->automaticCircle = value;
        std::cout << "Cork: A Auto circle, " << configB->automaticCircle << std::endl;
    });
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
    configA->thresholdValue = 50;
    configA->tresholdTolerance = 0.25;
    configA->rgb_shadow = true;

    //Camera A configuration
    cameraConfigA = new CameraConfig();
    //TODO <REMOVE true>
    if(true || !cameraConfigA->loadFile(CAMERA_CONFIG_A_FNAME))
    {
        std::cout << "Cork: Unable to load camera config A" << std::endl;
        cameraConfigA->originalWidth = 1920;
        cameraConfigA->originalHeight = 1200;
        cameraConfigA->width = 768;
        cameraConfigA->height = 480;
        cameraConfigA->input = CameraConfig::TCAM;
        cameraConfigA->tcamSerial = "46810320";
        cameraConfigA->saveFile(CAMERA_CONFIG_A_FNAME);
    }
    else
    {
        cameraConfigA->debugCout();
        std::cout << "Cork: Loaded camera config A" << std::endl;
    }

    //Cork B configuration
    configB = new CorkConfig();

    //Camera B configuration
    cameraConfigB = new CameraConfig();
    //TODO <REMOVE true>
    if(true || !cameraConfigB->loadFile(CAMERA_CONFIG_B_FNAME))
    {
        std::cout << "Cork: Unable to load camera config B" << std::endl;
        cameraConfigB->width = 640;
        cameraConfigB->height = 480;
        cameraConfigB->input = CameraConfig::USB;
        cameraConfigB->videoBackend = cv::CAP_ANY;
        cameraConfigB->usbNumber = 1;
        cameraConfigB->saveFile(CAMERA_CONFIG_B_FNAME);
    }
    else
    {
        cameraConfigB->debugCout();
        std::cout << "Cork: Loaded camera config B" << std::endl;
    }

    //Timer to update GUI values
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]()
    {
        if(running)
        {
            updateGUI();
        }
    });
    timer->start(300);

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
    }
    catch (const std::exception&)
    {
        std::cout << "Cork: Error starting the camera A capture" << std::endl;
    }

    try
    {
        cameraInputB->start();
    }
    catch (const std::exception&)
    {
        std::cout << "Cork: Error starting the camera B capture" << std::endl;
    }

    running = true;

    ui->button_stop_start->setText("Parar");
}

void MainWindow::stopCapture()
{
    try
    {
        cameraInputA->stop();
    }
    catch (const std::exception&)
    {
        std::cout << "Cork: Error stoping the camera A capture" << std::endl;
    }

    try
    {
        cameraInputB->stop();
    }
    catch (const std::exception&)
    {
        std::cout << "Cork: Error stoping the camera B capture" << std::endl;
    }

    running = false;

    ui->button_stop_start->setText("Iniciar");

    QTimer::singleShot(500, this, [=]()
    {
        ui->camera_a->clear();
        ui->camera_b->clear();
    });
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

        if(!writingImageA && !mat.empty())
        {
            writingImageA = true;

            cv::Mat resized, color;
            cv::cvtColor(mat, color, cv::COLOR_BGRA2RGB);
            cv::resize(color, resized, cv::Size(ui_static->camera_a->width(), ui_static->camera_a->height()), 0, 0, cv::INTER_CUBIC);
            ui_static->camera_a->setPixmap(QPixmap::fromImage(QImage(resized.data, resized.cols, resized.rows, static_cast<int>(resized.step), QImage::Format_RGB888)));

            writingImageA = false;
        }
    };

    cameraInputB = new CameraInput(cameraConfigB);
    cameraInputB->frameCallback = [] (cv::Mat &mat) -> void
    {
        CorkAnalyser::processFrame(mat, configB, &defectB);


        if(!writingImageB && !mat.empty())
        {
            writingImageB = true;

            cv::Mat resized, color;
            cv::cvtColor(mat, color, cv::COLOR_BGR2RGB);
            cv::resize(color, resized, cv::Size(ui_static->camera_b->width(), ui_static->camera_b->height()), 0, 0, cv::INTER_CUBIC);
            ui_static->camera_b->setPixmap(QPixmap::fromImage(QImage(resized.data, resized.cols, resized.rows, static_cast<int>(resized.step), QImage::Format_RGB888)));

            writingImageB = false;
        }
    };
}

MainWindow::~MainWindow()
{
    deleteCaptureHandlers();

    delete ui;
}
