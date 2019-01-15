#pragma once

/**
 * Stores the cork detection configuration.
 */
class CorkConfig
{
public:
    //Outside skirt size in pixels
    int outsizeSkirt = 7;

    //Pixels per inch
    //TODO <USE PPI ON CALCULATIONS>
    int ppmm = 100;

    //Color analysis
    //TODO <ACTUALLY USE THIS>
    bool rgb_shadow = false;

    //Hough parameters
    int lowCannyThresh = 120;
    int highCannyThresh = 30; //The smaller it is, the more false circles may be detected.
    int minSize = 110;
    int maxSize = 190;
    int minSpacing = 500;

    //Semi auto threshold
    double tresholdTolerance = 0.4;

    //Threshold value
    int thresholdValue = 60;

    //Blur parameters
    bool blurGlobal = false;
    int blurGlobalKSize = 3;
    bool blurMask = false;
    int blurMaskKSize = 3;
};
