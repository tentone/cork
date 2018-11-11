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

using namespace cv;
using namespace std;

//File number
int fnumber = IMAGES_START;

//Deblur picture before start
bool DEBLUR_IMAGE = false;

//Hough parameters
int MIN_SPACING = 100;
int LOW_CANNY_THRESH = 75;
int HIGH_CANNY_THRESH = 30;
int MIN_SIZE = 20;
int MAX_SIZE = 55;

//Threshold parameters
int THRESHOLD_BIN = 60;
bool AUTOMATIC_THRESH = false;
bool ADAPTIVE_THRESH = false;

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

double otsu_mask(const Mat1b src, const Mat1b& mask)
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

double automatic_threshold(const Mat1b src, const Mat1b& mask)
{
	/*
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

	for(int i = 0; i < N; i++)
	{
		if(histogram[i] != 0)
		{
			cout << i << ": " << histogram[i] << endl;
		}
	}
	*/

	return otsu_mask(src, mask) * 0.6;
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
	/*if(!cap.open(0))
	{
		cout << "Webcam not available." << endl;
	}*/
	
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
		if(DEBLUR_IMAGE)
		{
			//TODO <ADD CODE HERE>
		}

		//Convert image to grayscale
		Mat gray;
		cvtColor(image, gray, COLOR_BGR2GRAY);
		//imshow("Gray", gray);

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

			//roi = GaussianBlur(roi, (5, 5), 0);

			if(ADAPTIVE_THRESH)
			{
				int block = radius / 2;
				block = block % 2 == 0 ? block + 1 : block;
				
				adaptiveThreshold(roi, roi_bin, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, block, 2);

				int size = 2;
				int norm = 2 * size + 1;
				Mat element = getStructuringElement(MORPH_ELLIPSE, Size(norm, norm), Point(size, size));
				erode(roi_bin, roi_bin, element);
			}
			else
			{
				if(AUTOMATIC_THRESH)
				{
					double thresh = automatic_threshold(roi, mask);
					//cout << "Automatic threshold: " << thresh << endl;
					threshold(roi, roi_bin, thresh, 255, THRESH_BINARY);
				}
				else
				{
					threshold(roi, roi_bin, THRESHOLD_BIN, 255, THRESH_BINARY);
				}
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
			cvui::beginColumn(image, 10, 20);
			if(cvui::button(100, 20, "Close"))
			{
				return 0;
			}
			
			cvui::space(20);

			cvui::text("Threshold");
			cvui::space(12);
			cvui::checkbox("Adaptive", &ADAPTIVE_THRESH);
			cvui::space(12);
			cvui::checkbox("Automatic", &AUTOMATIC_THRESH);
			cvui::space(12);
			trackbar("Value", 200, &THRESHOLD_BIN, 10, 150, 1);
			cvui::space(5);
			trackbar("Erosion", 200, &EROSION_PX, 0, 10, 1);
			cvui::space(5);
			trackbar("Skirt", 200, &OUTSIDE_SKIRT_IGNORE_PX, 0, 20, 1);

			cvui::space(20);
			cvui::text("Circle");
			cvui::space(5);
			trackbar("Spacing", 200, &MIN_SPACING, 1, 400, 1);
			cvui::space(5);
			trackbar("Canny low", 200, &LOW_CANNY_THRESH, 1, 200, 1);
			cvui::space(5);
			trackbar("Canny high", 200, &HIGH_CANNY_THRESH, 1, 100, 1);
			cvui::space(5);
			trackbar("Min size", 200, &MIN_SIZE, 0, 100, 1);
			cvui::space(5);
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
