
/**
 * Stores the cork detection configuration.
 */
class Configuration
{
public:
	//Blur parameters
	bool BLUR_GLOBAL = false;
	int BLUR_GLOBAL_KSIZE = 3;
	bool BLUR_MASK = false;
	int BLUR_MASK_KSIZE = 3;

	//Hough parameters
	int LOW_CANNY_THRESH = 120;
	int HIGH_CANNY_THRESH = 30; //The smaller it is, the more false circles may be detected.
	int MIN_SIZE = 110;
	int MAX_SIZE = 190;
	int MIN_SPACING = 500;

	//Automatic threshold
	bool AUTOMATIC_THRESH = false;
	bool AUTOMATIC_USE_OTSU_THRESH = false;
	bool AUTOMATIC_USE_HIST_THRESH = false;

	bool SEMIAUTO_THRESH = true;
	double SEMIAUTO_THRESH_TOLERANCE = 0.4;

	//Threshold value
	int THRESHOLD_BIN = 60;

	//Tentone threshold parameters
	int HIST_THRESH_MIN_DIFF = 15;
	int HIST_THRESH_NEIGHBORHOOD = 15;
	int HIST_COLOR_FILTER = 15;
	double HIST_THRESH_BALANCE = 0.5;

	//Otso threhsold parameter
	double OTSU_THRESH_RATIO = 1.0;

	//Color analysis
	bool SPLIT_COLOR_CHANNELS = false;

	//Outside skirt size in pixels
	int OUTSIDE_SKIRT = 7;
};