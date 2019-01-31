#pragma once

#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>

#include "cork_config.hpp"
#include "threshold.hpp"

#define PI 3.14159265359

#define DEBUG_DEFECTS true

#define MEASURE_PERFORMANCE_CORK true

/**
 * Processes cork images and returns cork found and defect found.
 */
class CorkAnalyser
{
public:
    /**
     * Process a frame captured from the camera.
     *
     * @param image Input imagem to be processed
     * @
     */
    static void processFrame(cv::Mat &image, CorkConfig *config, double *defectOutput)
    {
        if(image.empty())
        {
            *defectOutput = -1.0;
            return;
        }

        #if MEASURE_PERFORMANCE_CORK
            int64 init = cv::getTickCount();
        #endif

        //Deblur the image
        if(config->blurGlobal)
        {
            cv::medianBlur(image, image, config->blurGlobalKSize);
        }

        cv::Mat gray;

        //Split color channels
        if(config->rgb_shadow)
        {
            cv::Mat bgra[4];
            cv::split(image, bgra);

            //Mix green 60% and red 40% channels
            cv::addWeighted(bgra[1], 0.6, bgra[2], 0.4, 0.0, gray);
        }
        //Convert image to grayscale
        else
        {
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        }

        //Detect circles
        if(config->automaticCircle)
        {
            std::vector<cv::Vec3f> circles;
            cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 1, config->minSpacing, config->lowCannyThresh, config->highCannyThresh, config->minSize, config->maxSize);

            //Iterate all found circles
            if(circles.size() > 0)
            {
                config->circle = circles[0];
            }
            else
            {
                *defectOutput = -1.0;
                return;
            }
        }

        cv::Vec3i c = config->circle;
        cv::Point center = cv::Point(c[0], c[1]);
        int radius = c[2];

        //Check if not fully inside of the image cannot be used
        if(radius > center.x || radius > center.y || radius + center.x > gray.cols || radius + center.y > gray.rows)
        {
            std::cout << "Cork: Circle is outside of the image." << std::endl;
            *defectOutput = -1.0;
            return;
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

        //Only calculate otsu if the tolerance is more than 0
        if(config->tresholdTolerance > 0.0)
        {
            double thresh = Threshold::otsuMask(roi, mask);
            thresh = (thresh * config->tresholdTolerance) + (config->thresholdValue * (1 - config->tresholdTolerance));
            cv::threshold(roi, roi_bin, thresh, 255, cv::THRESH_BINARY);
            //std::cout << "Semi Automatic threshold: " << thresh << std::endl;
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

        double radiusSkirt = (radius - config->outsizeSkirt);
        double area = PI * radiusSkirt * radiusSkirt;
        double defect = (count / area) * 100.0;

        //std::cout << "cv::Points: " << points << std::endl;
        //std::cout << "Resolution: " << (roi_bin.rows * roi_bin.cols) << std::endl;
        //std::cout << "Count: " << count << std::endl;
        //std::cout << "Area: " << area << std::endl;
        //std::cout << "Defect: " << defect << "%" << std::endl;

        //Draw debug information
        #if DEBUG_DEFECTS
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
            //cv::circle(image, center, 1, cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
            cv::circle(image, center, radius, cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
            //cv::putText(image, std::to_string(defect) + "%", center, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255));
        #endif

        if(config->checkIdentation)
        {
            std::vector<cv::Vec3f> inside;
            cv::HoughCircles(roi, inside, cv::HOUGH_GRADIENT, 1, config->minSpacing, config->lowCannyThresh, config->highCannyThresh, roi.rows / 10, roi.rows / 3);

            std::cout << roi.rows << std::endl;

            if(inside.size() > 0)
            {
                config->identation = inside[0];

                #if DEBUG_DEFECTS
                    //cv::Point(config->identation[0], config->identation[1])
                    cv::circle(image, center, config->identation[2], cv::Scalar(255, 0, 0), 1, cv::LINE_AA);
                #endif
            }
        }


        #if MEASURE_PERFORMANCE_CORK
            int64 end = cv::getTickCount();
            double secs = (end - init) / cv::getTickFrequency();
            std::cout << "Cork: Processing time was " << secs << " s." << std::endl;
        #endif

        *defectOutput = defect;
    }

    /**
     * Check if a value is in the neighborhood of another one.
     *
     * @param value
     * @param center
     * @param neighborhood
     */
    static bool isNeighbor(int value, int center, int neighborhood)
    {
        return value > (center - neighborhood) && value < (center + neighborhood);
    }
};
