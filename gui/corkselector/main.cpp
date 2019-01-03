#include <QApplication>

#include "mainwindow.hpp"

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <thread>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>

#include "threshold.hpp"
#include "cork_config.hpp"
#include "cork_analyser.hpp"
#include "cork.hpp"
#include "camera_input.hpp"
#include "camera_config.hpp"
#include "image_status.hpp"
#include "cvgui.hpp"

#include "tcamcamera.h"

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

/**
 * Process a frame captured from the camera.
 *
 * @param image Input imagem to be processed
 * @
 */
void processFrame(cv::Mat &image, CorkConfig *config, double *defectOutput)
{
    //Deblur the image
    if(config->blurGlobal)
    {
        cv::medianBlur(image, image, config->blurGlobalKSize);
    }

    cv::Mat gray;

    //Split color channels
    if(config->splitColorChannels)
    {
        cv::Mat bgr[3];
        cv::split(image, bgr);

        //Use the gree channel
        gray = bgr[1];
    }
    //Convert image to grayscale
    else
    {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    }

    //Detect circles
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 1, config->minSpacing, config->lowCannyThresh, config->highCannyThresh, config->minSize, config->maxSize);

    bool found = circles.size() > 0;

    //Iterate all found circles
    for(size_t i = 0; i < circles.size(); i++)
    {
        cv::Vec3i c = circles[i];
        cv::Point center = cv::Point(c[0], c[1]);
        int radius = c[2];

        //Check if fully inside of the image
        if(radius > center.x || radius > center.y || radius + center.x > gray.cols || radius + center.y > gray.rows)
        {
            continue;
        }

        //Create the roi
        cv::Rect roi_rect = cv::Rect(center.x - radius, center.y - radius, radius * 2, radius * 2);
        cv::Mat roi = cv::Mat(gray, roi_rect);

        //Circle mask for the roi
        cv::Mat mask = cv::Mat(roi.rows, roi.cols, roi.type(), cv::Scalar(255, 255, 255));
        cv::circle(mask, cv::Point(roi.rows / 2, roi.cols / 2), radius - config->outsizeSkirt, cv::Scalar(0, 0, 0), -1, 8, 0);

        //Binarize the roi
        cv::Mat roi_bin;

        if(config->blurMask)
        {
            cv::medianBlur(roi, roi, config->blurMaskKSize);
        }

        if(config->automaticThresh)
        {
            if(config->automaticUseOtsuThresh)
            {
                double thresh = Threshold::otsuMask(roi, mask);
                //std::cout << "Otsu Automatic threshold: " << thresh << std::endl;
                cv::threshold(roi, roi_bin, thresh, 255, cv::THRESH_BINARY);
            }
            else// if(config->automaticUseHistogramThresh)
            {
                double thresh = Threshold::histogram(roi, mask, config->histThreshMinDiff, config->histThreshNeighborhood, config->histThreshColorFilter, config->histThreshBalance);
                //std::cout << "Histogram automatic threshold: " << thresh << std::endl;
                cv::threshold(roi, roi_bin, thresh, 255, cv::THRESH_BINARY);
            }
        }
        else if(config->semiAutoThresh)
        {
            double thresh = Threshold::otsuMask(roi, mask);
            thresh = (thresh * config->semiAutoThreshTolerance) + (config->thresholdValue * (1 - config->semiAutoThreshTolerance));
            //std::cout << "Semi Automatic threshold: " << thresh << std::endl;
            cv::threshold(roi, roi_bin, thresh, 255, cv::THRESH_BINARY);
        }
        else
        {
            cv::threshold(roi, roi_bin, config->thresholdValue, 255, cv::THRESH_BINARY);
        }

        //Mask outside of the cork in roi bin
        bitwise_or(mask, roi_bin, roi_bin);

        //Invert binary roi
        bitwise_not(roi_bin, roi_bin);

        //Measure defective area
        double count = 0.0;
        unsigned char *data = (unsigned char*)(roi_bin.data);
        for(int j = 0; j < roi_bin.rows; j++)
        {
            for(int i = 0; i < roi_bin.cols; i++)
            {
                if(data[roi_bin.step * j + i] > 0)
                {
                    count++;
                }
            }
        }

        double area = PI * radius * radius;
        double defect = (count / area) * 100.0;

        //std::cout << "cv::Points: " << points << std::endl;
        //std::cout << "Resolution: " << (roi_bin.rows * roi_bin.cols) << std::endl;
        //std::cout << "Count: " << count << std::endl;
        //std::cout << "Area: " << area << std::endl;
        //std::cout << "Defect: " << defect << "%" << std::endl;

        //Draw debug information
        if(DEBUG_DEFECTS)
        {
            int channels = image.channels();

            //Draw defect
            for(int i = 0; i < roi_bin.rows; i++)
            {
                for(int j = 0; j < roi_bin.cols; j++)
                {
                    int t = (i * roi_bin.cols + j);

                    if(roi_bin.data[t] > 0)
                    {
                        int k = ((i + roi_rect.y) * image.cols + (j + roi_rect.x)) * channels;

                        image.data[k + 2] = (unsigned char) 255;
                    }
                }
            }

            //Cicle position
            cv::circle(image, center, 1, cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
            cv::circle(image, center, radius, cv::Scalar(0, 255, 000), 1, cv::LINE_AA);
            cv::putText(image, std::to_string(defect) + "%", center, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255));
        }

        *defectOutput = defect;
        return;
    }

    *defectOutput = -1.0;
}

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
    cameraConfigA.input = CameraConfig::TCAM;
    cameraConfigA.usbNumber = 0;

    CameraInput *cameraInputA = new CameraInput(cameraConfigA);
    cameraInputA->frameCallback = [] (cv::Mat &mat) -> void
    {
        if(saveNextFrame)
        {
            std::cout << "Save frame" << std::endl;
            saveNextFrame = false;
            cv::imwrite("./" + std::to_string(saveFrameCounter++) + ".png", mat);
        }

        processFrame(mat, &configA, &defectA);

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
    cameraConfigB.input = CameraConfig::USB;
    cameraConfigB.usbNumber = 1;

    CameraInput *cameraInputB = new CameraInput(cameraConfigB);
    cameraInputB->frameCallback = [] (cv::Mat &mat) -> void
    {
        processFrame(mat, &configB, &defectB);

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
