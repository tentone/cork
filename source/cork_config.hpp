#pragma once

#include <opencv2/core.hpp>

/**
 * Stores the cork detection configuration.
 */
class CorkConfig
{
public:
    //Pixels per milimeter
    int ppmm = 100;

    //Color analysis
    bool rgb_shadow = false;

    //Outside skirt size in pixels
    int outsizeSkirt = 7;

    //Min spacing
    int minSize = 90;
    int maxSize = 190;
    int minSpacing = 500;

    //Hough parameters
    int lowCannyThresh = 120;
    int highCannyThresh = 30; //The smaller it is, the more false circles may be detected.

    //Semi auto threshold
    double tresholdTolerance = 0.5;

    //Threshold value
    int thresholdValue = 30;

    //Blur parameters
    bool blurGlobal = false;
    int blurGlobalKSize = 3;
    bool blurMask = false;
    int blurMaskKSize = 3;

    //Detection circle
    bool automaticCircle = true;

    //Cork circle stored as a vector3 [x,y,r]
    cv::Vec3f circle;
};
