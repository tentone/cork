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
    int ppi = 100;

    //Hough parameters
    int lowCannyThresh = 120;
    int highCannyThresh = 30; //The smaller it is, the more false circles may be detected.
    int minSize = 110;
    int maxSize = 190;
    int minSpacing = 500;

    //Semi auto threshold
    bool semiAutoThresh = true;
    double semiAutoThreshTolerance = 0.4;

    //Threshold value
    int thresholdValue = 60;

    //Color analysis
    //TODO <ACTUALLY USE THIS>
    bool splitColorChannels = false;

    //Blur parameters
    bool blurGlobal = false;
    int blurGlobalKSize = 3;
    bool blurMask = false;
    int blurMaskKSize = 3;

    //Automatic threshold
    bool automaticThresh = false;
    bool automaticUseOtsuThresh = false;
    bool automaticUseHistogramThresh = false;

    //Tentone threshold parameters
    int histThreshMinDiff = 15;
    int histThreshNeighborhood = 15;
    int histThreshColorFilter = 15;
    double histThreshBalance = 0.5;
};
