#include <iostream>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>

#define CVUI_DISABLE_COMPILATION_NOTICES
#define CVUI_IMPLEMENTATION
#include "lib/cvui.h"

#define DEBUG_WINDOW true
#define WINDOW_NAME "Cork"

#define KEY_ESC 27
#define KEY_LEFT 81
#define KEY_RIGHT 83
#define KEY_UP 82
#define KEY_DOWN 84
#define KEY_W 119
#define KEY_S 115

#define PI 3.14159265359

#define IMAGES_START 0
#define IMAGES_COUNT 20

#define USE_CAMERA false
#define USE_USB_CAMERA false
#define USE_IP_CAMERA true
#define IP_CAMERA_ADDRESS "rtsp://admin:123456@192.168.0.10:554/live/ch0"
//#define IP_CAMERA_ADDRESS "rtsp://192.168.0.124:8080/video/h264"

using namespace cv;
using namespace std;

//File number
int fnumber = IMAGES_START;

//Blur parameters
bool BLUR_GLOBAL = false;
int BLUR_GLOBAL_KSIZE = 3;
bool BLUR_MASK = false;
int BLUR_MASK_KSIZE = 3;

//Hough parameters
int MIN_SPACING = 400;
//It is the higher threshold of the two passed to the Canny edge detector (the lower one is twice smaller).
int LOW_CANNY_THRESH = 120;
//It is the accumulator threshold for the circle centers at the detection stage. The smaller it is, the more false circles may be detected. Circles, corresponding to the larger accumulator values, will be returned first.
int HIGH_CANNY_THRESH = 30;
int MIN_SIZE = 47;
int MAX_SIZE = 70;

//Automatic threshold
bool AUTOMATIC_THRESH = true;
bool AUTOMATIC_USE_OTSU_THRESH = false;
bool AUTOMATIC_USE_TENTONE_THRESH = true;
bool AUTOMATIC_USE_ADAPTIVE_THRESH = false;

//Threshold value
int THRESHOLD_BIN = 60;

//Tentone threshold parameters
int TENTONE_THRESH_MIN_DIFF = 40;
int TENTONE_THRESH_NEIGHBORHOOD = 10;
int TENTONE_COLOR_FILTER = 30;
double TENTONE_THRESH_BALANCE = 0.3;

//Otso threhsold parameter
double OTSU_THRESH_RATIO = 0.6;

//Color analysis
bool SPLIT_COLOR_CHANNELS = true;

//Erosion configuration (only used if above 0)
int EROSION_PX = 0;

//Igore skirt
int OUTSIDE_SKIRT_IGNORE_PX = 4;

void trackbar(const cv::String& theText, int theWidth, double *theValue, double theMin, double theMax, int theSegments = 1, const char *theLabelFormat = "%.1Lf", unsigned int theOptions = 0, double theDiscreteStep = 1)
{
	cvui::beginRow();
	cvui::text(theText);
	cvui::beginColumn();
	cvui::space(-19);
	cvui::trackbar(theWidth, theValue, theMin, theMax, theSegments, theLabelFormat, theOptions | cvui::TRACKBAR_HIDE_SEGMENT_LABELS | cvui::TRACKBAR_HIDE_MIN_MAX_LABELS, theDiscreteStep);
	cvui::endColumn();
	cvui::endRow();
}

void trackbar(const cv::String& theText, int theWidth, int *theValue, int theMin, int theMax, int theSegments = 1, const char *theLabelFormat = "%.Lf", unsigned int theOptions = 0, int theDiscreteStep = 1)
{
	cvui::beginRow();
	cvui::text(theText);
	cvui::beginColumn();
	cvui::space(-19);
	cvui::trackbar(theWidth, theValue, theMin, theMax, theSegments, theLabelFormat, theOptions | cvui::TRACKBAR_HIDE_SEGMENT_LABELS | cvui::TRACKBAR_HIDE_MIN_MAX_LABELS, theDiscreteStep);
	cvui::endColumn();
	cvui::endRow();
}

/**
 * Otsu treshold algorithm with mask support.
 */
double otsuMask(const Mat1b src, const Mat1b& mask)
{
	//Colors
	const int N = 256;

	int M = 0;

	//Create the image histogram
	int h[N] = {0};
	for(int i = 0; i < src.rows; i++)
	{
		const uchar* psrc = src.ptr(i);
		const uchar* pmask = mask.ptr(i);
		for(int j = 0; j < src.cols; j++)
		{
			if(pmask[j])
			{
				h[psrc[j]]++;
				++M;
			}
		}
	}

	double mu = 0, scale = 1.0 / (M);
	for(int i = 0; i < N; i++)
	{
		mu += i * (double)h[i];
	}

	mu *= scale;
	double mu1 = 0, q1 = 0;
	double max_sigma = 0, max_val = 0;

	for(int i = 0; i < N; i++)
	{
		double p_i, q2, mu2, sigma;

		p_i = h[i] * scale;
		mu1 *= q1;
		q1 += p_i;
		q2 = 1.0 - q1;

		if(min(q1, q2) < FLT_EPSILON || max(q1, q2) > 1.0 - FLT_EPSILON)
		{
			continue;
		}

		mu1 = (mu1 + i * p_i) / q1;
		mu2 = (mu - q1 * mu1) / q2;
		sigma = q1 * q2 * (mu1 - mu2) * (mu1 - mu2);

		if(sigma > max_sigma)
		{
			max_sigma = sigma;
			max_val = i;
		}
	}

	return max_val;
}

/**
 * Check if a value is in the neighborhood of another one.
 *
 * @param value
 * @param center
 * @param neighborhood
 */
bool isNeighbor(int value, int center, int neighborhood)
{
	return  value > (center - neighborhood) && value < (center + neighborhood);
}

/**
 * Corsk specific automatic treshold calculation.
 *
 * Differently from the OTSU algorithm that always aceppts a separations between the values this algorithm can allow situations were there is no sparation.
 *
 * @param src Input image.
 * @param mask Mask image.
 * @param min_diff Minimum separation between the colors found to get a treshold, if the diff exceds this limit 0 is returned.
 * @param neighborhood Neighborhood to analyse when creating the comulative histogram.
 */
double corkTreshold(const Mat1b src, const Mat1b& mask, int min_diff = 40, int neighborhood = 10, int neighborhood_filter = 30, double balance = 0.3)
{
	const int N = 256;
	int histogram[N] = {0};
	
	//Create the image histogram
	for(int i = 0; i < src.rows; i++)
	{
		const uchar* psrc = src.ptr(i);
		const uchar* pmask = mask.ptr(i);
		for(int j = 0; j < src.cols; j++)
		{
			if(pmask[j])
			{
				histogram[psrc[j]]++;
			}
		}
	}

	//Count in the neighborhood
	int count[N] = {0};
	int colors[N] = {0};
	
	//Neighborhood to be analysed
	int neighborhood_half = neighborhood / 2;
	int start = neighborhood_half;
	int end = N - neighborhood_half;

	for(int i = start; i < end; i++)
	{
		int sum_count = 0;
		int sum_color = 0;
		int c = 0;

		//Analyse the neighborhood
		for(int j = i - neighborhood_half; j < i + neighborhood_half; j++)
		{
			sum_color += i;
			sum_count += histogram[i];
			c++;
		}

		colors[i] = sum_color / c;
		count[i] = sum_count;
	}

	//Sort the array from bigger to smaller
	//TODO <PROPER SORTING ALGORITHM>
	//TODO <EXPLICIT SIMD>
	for(int i = 0; i < N; i++)
	{
		for(int j = i; j < N; j++)
		{
			if(count[i] < count[j])
		 	{
		 		int temp = count[i];
		 		count[i] = count[j];
		 		count[j] = temp;

		 		temp = colors[i];
		 		colors[i] = colors[j];
		 		colors[j] = temp;
		 	}
		}
	}

	//Colors with more occurences
	int high = colors[0];
	int low = -1;
	
	//Select the b color (the next color with more occurences far from b)
	for(int i = 0; i < 10; i++)
	{
		int highc = max(colors[i], high);
		int lowc = min(colors[i], high);
		
		if((highc - lowc) < neighborhood_filter)
		{
			continue;
		}
		
		low = colors[i];
		break;
	}

	double diff = high - low;
	if(diff < min_diff || low == -1)
	{
		return 0;
	}

	return low + (diff * balance);
}

bool hasCorkColorTone(const Mat src, Vec3f circle)
{
	//TODO <ADD CODE HERE>

	return true;
}

Mat readImage(int index)
{
	fnumber = index;

	if(fnumber > IMAGES_COUNT)
	{
		fnumber = IMAGES_START;
	}
	if(fnumber < IMAGES_START)
	{
		fnumber = IMAGES_COUNT;
	}

	return imread("data/" + to_string(fnumber) + ".jpg", IMREAD_COLOR);
}


int main(int argc, char** argv)
{
	VideoCapture cap;
	
	if(USE_CAMERA)
	{
		if(USE_USB_CAMERA)
		{
			if(!cap.open(0))
			{
				cout << "Webcam not available." << endl;
			}

			//Set resolution
			cap.set(CAP_PROP_FRAME_WIDTH, 1280);
			cap.set(CAP_PROP_FRAME_HEIGHT, 720);

			if(cap.get(CAP_PROP_FRAME_HEIGHT) != 720 || cap.get(CAP_PROP_FRAME_WIDTH)!=1280)
			{
				cout << "Unable to set webcam 1280x720" << endl;
			}

			//Set exposure
			cap.set(CAP_PROP_AUTO_EXPOSURE, 0);
			cap.set(CAP_PROP_EXPOSURE, -100);

			//Focus
			//cap.set(CAP_PROP_FOCUS, 0);
		}
		else if(USE_IP_CAMERA)
		{
			if(!cap.open(IP_CAMERA_ADDRESS))
			{
				cout << "Camera not available." << endl;
			}
		}
	}

	
	Mat image;

	//Prepare output window
	cvui::init(WINDOW_NAME);

	while(1)
	{
		//Get image
		if(cap.isOpened())
		{
			cap >> image;
		}
		else
		{
			image = readImage(fnumber);
		}



		//Deblur the image
		if(BLUR_GLOBAL)
		{
			medianBlur(image, image, BLUR_GLOBAL_KSIZE);
		}
		
		Mat gray;

		//Split color channels
		if(SPLIT_COLOR_CHANNELS)
		{
			Mat bgr[3];

			split(image, bgr);
			
			imshow("B", bgr[0]);
			imshow("G", bgr[1]);
			imshow("R", bgr[2]);

			gray = bgr[0];
		}
		//Convert image to grayscale
		else
		{
			cvtColor(image, gray, COLOR_BGR2GRAY);
			//imshow("Gray", gray);
		}

		//Detect circles
		vector<Vec3f> circles;
		HoughCircles(gray, circles, HOUGH_GRADIENT, 1, MIN_SPACING, LOW_CANNY_THRESH, HIGH_CANNY_THRESH, MIN_SIZE, MAX_SIZE);

		bool found = circles.size() > 0;

		//Draw circle outline
		for(size_t i = 0; i < circles.size(); i++)
		{
			Vec3i c = circles[i];
			Point center = Point(c[0], c[1]);
			int radius = c[2];
			
			//Check if fully inside of the image
			if(radius > center.x || radius > center.y || radius + center.x > gray.cols || radius + center.y > gray.rows)
			{
				continue;
			}

			//Create the roi
			Rect roi_rect = Rect(center.x - radius, center.y - radius, radius * 2, radius * 2);
			Mat roi = Mat(gray, roi_rect);
			
			//Circle mask for the roi
			Mat mask = Mat(roi.rows, roi.cols, roi.type(), Scalar(255, 255, 255));
			circle(mask, Point(roi.rows / 2, roi.cols / 2), radius - OUTSIDE_SKIRT_IGNORE_PX, Scalar(0, 0, 0), -1, 8, 0);

			//Binarize the roi
			Mat roi_bin;

			if(BLUR_MASK)
			{
				medianBlur(roi, roi, BLUR_MASK_KSIZE);
			}

			if(AUTOMATIC_THRESH)
			{
				if(AUTOMATIC_USE_ADAPTIVE_THRESH)
				{
					int block = radius / 2;
					block = block % 2 == 0 ? block + 1 : block;
					
					adaptiveThreshold(roi, roi_bin, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, block, 2);

					int size = 2;
					int norm = 2 * size + 1;
					Mat element = getStructuringElement(MORPH_ELLIPSE, Size(norm, norm), Point(size, size));
					erode(roi_bin, roi_bin, element);
				}
				else if(AUTOMATIC_USE_OTSU_THRESH)
				{
					//cout << "Automatic threshold: " << thresh << endl;
					double thresh = OTSU_THRESH_RATIO * otsuMask(roi, mask);
					threshold(roi, roi_bin, thresh, 255, THRESH_BINARY);
				}
				else// if(AUTOMATIC_USE_TENTONE_THRESH)
				{
					//cout << "Automatic threshold: " << thresh << endl;
					double thresh = corkTreshold(roi, mask, TENTONE_THRESH_MIN_DIFF, TENTONE_THRESH_NEIGHBORHOOD, TENTONE_COLOR_FILTER, TENTONE_THRESH_BALANCE);
					threshold(roi, roi_bin, thresh, 255, THRESH_BINARY);
				}
			}
			else
			{
				threshold(roi, roi_bin, THRESHOLD_BIN, 255, THRESH_BINARY);
			}

			//Mask outside of the cork in roi bin
			bitwise_or(mask, roi_bin, roi_bin);

			//Invert binary roi
			bitwise_not(roi_bin, roi_bin);
			//imshow("ROI Binary", roi_bin);

			//Erode
			if(EROSION_PX > 0)
			{
				int kernel = 2 * EROSION_PX + 1;
				Mat element = getStructuringElement(MORPH_RECT, Size(kernel, kernel), Point(EROSION_PX, EROSION_PX));
				erode(roi_bin, roi_bin, element);
			}

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

			//cout << "Points: " << points << endl;
			//cout << "Resolution: " << (roi_bin.rows * roi_bin.cols) << endl;
			//cout << "Count: " << count << endl;
			//cout << "Area: " << area << endl;
			//cout << "Defect: " << defect << "%" << endl;

			//Draw debug information
			if(DEBUG_WINDOW)
			{
				//Draw defect
				for(int i = 0; i < roi_bin.rows; i++)
				{
					for(int j = 0; j < roi_bin.cols; j++)
					{
						int t = (i * roi_bin.cols + j);

						if(roi_bin.data[t] > 0)
						{
							int k = ((i + roi_rect.y) * image.cols + (j + roi_rect.x)) * 3;

							image.data[k + 2] = (unsigned char) 255;
						}
					}
				}

				//Cicle position
				circle(image, center, 1, Scalar(255, 0, 0), 2, LINE_AA);
				circle(image, center, radius, Scalar(0, 255, 000), 1, LINE_AA);
				putText(image, to_string(defect) + "%", center, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 255));
			}
		}

		if(DEBUG_WINDOW)
		{
			cvui::beginColumn(image, 10, 10);
			cvui::beginRow();
			cvui::checkbox("Blur Global", &BLUR_GLOBAL);
			cvui::space(12);
			cvui::checkbox("Blur Mask", &BLUR_MASK);
			cvui::endRow();

			if(BLUR_GLOBAL)
			{
				cvui::space(12);
				trackbar("Blur Global Kernel", 200, &BLUR_GLOBAL_KSIZE, 3, 101, 2);
				if(BLUR_GLOBAL_KSIZE % 2 == 0)
				{
					BLUR_GLOBAL_KSIZE++;
				}
			}

			if(BLUR_MASK)
			{
				cvui::space(12);
				trackbar("Blur Mask Kernel", 200, &BLUR_MASK_KSIZE, 3, 101, 2);
				if(BLUR_MASK_KSIZE % 2 == 0)
				{
					BLUR_MASK_KSIZE++;
				}
			}

			cvui::space(12);
			cvui::text("Threshold ___________________");

			cvui::space(12);
			cvui::beginRow();
			cvui::checkbox("Automatic", &AUTOMATIC_THRESH);
			if(AUTOMATIC_THRESH)
			{
				cvui::space(12);
				if(cvui::checkbox("Otsu", &AUTOMATIC_USE_OTSU_THRESH))
				{
					AUTOMATIC_USE_TENTONE_THRESH = false;
					AUTOMATIC_USE_ADAPTIVE_THRESH = false;
				}
				cvui::space(12);
				if(cvui::checkbox("Custom", &AUTOMATIC_USE_TENTONE_THRESH))
				{
					AUTOMATIC_USE_OTSU_THRESH = false;
					AUTOMATIC_USE_ADAPTIVE_THRESH = false;
				}
				cvui::space(12);
				if(cvui::checkbox("Adaptive", &AUTOMATIC_USE_ADAPTIVE_THRESH))
				{
					AUTOMATIC_USE_TENTONE_THRESH = false;
					AUTOMATIC_USE_OTSU_THRESH = false;
				}
			}
			cvui::endRow();
			
			if(!AUTOMATIC_THRESH)
			{
				cvui::space(12);
				trackbar("Threshold", 200, &THRESHOLD_BIN, 10, 150, 1);
			}
			else if(AUTOMATIC_USE_TENTONE_THRESH)
			{
				cvui::space(12);
				trackbar("Min-diff", 200, &TENTONE_THRESH_MIN_DIFF, 5, 100, 1);
				cvui::space(12);
				trackbar("Neighborhood", 200, &TENTONE_THRESH_NEIGHBORHOOD, 1, 100, 1);
				cvui::space(12);
				trackbar("Neigh. Filter", 200, &TENTONE_COLOR_FILTER, 1, 100, 1);
				cvui::space(12);
				trackbar("Balance", 200, &TENTONE_THRESH_BALANCE, 0, 1, 1);	
			}
			
			cvui::space(12);
			trackbar("Skirt", 200, &OUTSIDE_SKIRT_IGNORE_PX, 0, 20, 1);
			//cvui::space(12);
			//trackbar("Erosion", 200, &EROSION_PX, 0, 10, 1);

			cvui::space(12);
			cvui::text("Circle ___________________");
			cvui::space(12);
			trackbar("Spacing", 200, &MIN_SPACING, 1, 400, 1);
			cvui::space(12);
			trackbar("Canny low", 200, &LOW_CANNY_THRESH, 1, 200, 1);
			cvui::space(12);
			trackbar("Canny high", 200, &HIGH_CANNY_THRESH, 1, 100, 1);
			cvui::space(12);
			trackbar("Min size", 200, &MIN_SIZE, 0, 100, 1);
			cvui::space(12);
			trackbar("Max size", 200, &MAX_SIZE, 0, 300, 1);
			cvui::endColumn();

			cvui::update();
			cvui::imshow(WINDOW_NAME, image);
		}

		//Keyboard input
		int key = waitKey(1);
		if(key != -1)
		{
			if(key == KEY_ESC)
			{
				return 0;
			}
			else if(key == KEY_LEFT)
			{
				readImage(--fnumber);
				cout << "Fname:" << fnumber << endl;
			}
			else if(key == KEY_RIGHT)
			{
				readImage(++fnumber);
				cout << "Fname:" << fnumber << endl;
			}
			else
			{
				cout << "Unkown key:" << key << endl;
			}
		}
	}

	return 0;
}
