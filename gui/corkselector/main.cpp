#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <thread>

#include <QApplication>

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

#define DEBUG_DEFECTS true
#define DEBUG_GUI true

bool saveNextFrame = false;
int saveFrameCounter = 0;

double defectA = -1.0;
double defectB = -1.0;

bool debugConfigA = false;
bool debugConfigB = true;

std::string windowA = "CorkA";
std::string windowB = "CorkB";

CorkConfig configA;
CorkConfig configB;

int main(int argc, char** argv)
{
    gst_init(&argc, &argv);

    if(debugConfigA)
    {
        cvui::init(windowA);
    }

    if(debugConfigB)
    {
        cvui::init(windowB);
    }

    CameraConfig cameraConfigA;
    cameraConfigA.width = 768;
    cameraConfigA.height = 480;
    cameraConfigA.input = CameraConfig::TCAM;

    CameraInput *cameraInputA = new CameraInput(cameraConfigA);
    cameraInputA->frameCallback = [] (cv::Mat &mat) -> void
    {
        if(saveNextFrame)
        {
            std::cout << "Save frame" << std::endl;
            saveNextFrame = false;
            cv::imwrite("./" + std::to_string(saveFrameCounter++) + ".png", mat);
        }

        CorkAnalyser::processFrame(mat, &configA, &defectA);

        if(DEBUG_GUI)
        {
            if(debugConfigA)
            {
                cvgui::drawConfigEditor(windowA, mat, &configA);
            }
            else
            {
                cv::imshow(windowA, mat);
            }

            int key = cv::waitKey(1);

            if(key != -1)
            {
                if(key == KEY_ESC)
                {
                    saveNextFrame = true;
                }
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

        if(DEBUG_GUI)
        {
            if(debugConfigB)
            {
                cvgui::drawConfigEditor(windowB, mat, &configB);
            }
            else
            {
                cv::imshow(windowB, mat);
            }

            cv::waitKey(1);
        }
    };

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

    cameraInputA->stop();
    cameraInputB->stop();

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
