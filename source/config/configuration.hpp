
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
	bool AUTOMATIC_USE_OTSU_THRESH = false;
	bool AUTOMATIC_USE_HIST_THRESH = false;

	bool semiAutoThresh = true;
	double semiAutoThreshTolerance = 0.4;

	//Threshold value
	int thresholdValue = 60;

	//Tentone threshold parameters
	int HIST_THRESH_MIN_DIFF = 15;
	int HIST_THRESH_NEIGHBORHOOD = 15;
	int HIST_COLOR_FILTER = 15;
	double HIST_THRESH_BALANCE = 0.5;

	//Otso threhsold parameter
	double OTSU_THRESH_RATIO = 1.0;

	//Color analysis
	bool splitColorChannels = false;

	//Outside skirt size in pixels
	int outsizeSkirt = 7;
};