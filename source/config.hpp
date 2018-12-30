#pragma once

/**
 * Stores the cork detection configuration.
 */
class Configuration
{
public:
	//Blur parameters
	bool blurGlobal = false;
	int blurGlobalKSize = 3;
	bool blurMask = false;
	int blurMaskKSize = 3;

	//Hough parameters
	int lowCannyThresh = 120;
	int highCannyThresh = 30; //The smaller it is, the more false circles may be detected.
	int minSize = 110;
	int maxSize = 190;
	int minSpacing = 500;

	//Automatic threshold
	bool automaticThresh = false;
	bool automaticUseOtsuThresh = false;
	bool automaticUseHistogramThresh = false;

	bool semiAutoThresh = true;
	double semiAutoThreshTolerance = 0.4;

	//Threshold value
	int thresholdValue = 60;

	//Tentone threshold parameters
	int histThreshMinDiff = 15;
	int histThreshNeighborhood = 15;
	int histThreshColorFilter = 15;
	double histThreshBalance = 0.5;

	//Color analysis
	bool splitColorChannels = false;

	//Outside skirt size in pixels
	int outsizeSkirt = 7;
};