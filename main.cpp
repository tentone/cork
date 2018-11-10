#include <iostream>
#include <string>

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#define WINDOW_NAME "Cork"

#define KEY_ESC 27
#define KEY_LEFT 81
#define KEY_RIGHT 83
#define KEY_UP 82
#define KEY_DOWN 84
#define KEY_W 119
#define KEY_S 115

#define IMAGES_COUNT 20

//Choose between adaptive thresold or otsu binarization
#define ADAPTIVE_THRESH false

//Small erosion step to eliminate small defects
#define ERODE false

//Deblur picture before start
#define DEBLUR_IMAGE false

#define OUTSIDE_SKIRT_IGNORE_PX 3

using namespace cv;
using namespace std;

int fnumber = 1;

cv::Mat readImage(int index)
{
	fnumber = index;

	if(fnumber > IMAGES_COUNT)
	{
		fnumber = 1;
	}
	if(fnumber < 1)
	{
		fnumber = IMAGES_COUNT;
	}

	return imread("data/" + std::to_string(fnumber) + ".jpg", IMREAD_COLOR);
}


int main(int argc, char** argv)
{
	VideoCapture cap(0);
	Mat image;

	//Prepare output window
	namedWindow(WINDOW_NAME, WINDOW_AUTOSIZE);
	setMouseCallback(WINDOW_NAME, [](int event, int x, int y, int flags, void *param)
	{
		//if(event == EVENT_LBUTTONDOWN){}
		//else if(event == EVENT_RBUTTONDOWN)
		//else if(event == EVENT_MBUTTONDOWN)
		//else if(event == EVENT_MOUSEMOVE)
	} , &image);

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
		int minSpacing = gray.rows / 3;
		int lowCannyThreshold = 100;
		int highCannyThreshold = 30;
		int maxSize = gray.rows / 2;
		HoughCircles(gray, circles, HOUGH_GRADIENT, 1, minSpacing, lowCannyThreshold, highCannyThreshold, 0, maxSize);

		bool found = circles.size() > 0;

		//Draw circle outline
		for(size_t i = 0; i < circles.size(); i++)
		{
			Vec3i c = circles[i];
			Point center = Point(c[0], c[1]);
			int radius = c[2];

			//Create the roi
			Mat roi(gray, cv::Rect(center.x - radius, center.y - radius, radius * 2, radius * 2));
			Mat roibin;
				
			//Binarize the image
			if(ADAPTIVE_THRESH)
			{
				int block = radius / 2;
				block = block % 2 == 0 ? block + 1 : block;
				
				adaptiveThreshold(roi, roibin, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, block, 2);

				int size = 2;
				int norm = 2 * size + 1;
				Mat element = getStructuringElement(MORPH_ELLIPSE, Size(norm, norm), Point(size, size));
				erode(roibin, roibin, element);
			}
			else
			{
				threshold(roi, roibin, 0, 255, THRESH_BINARY + THRESH_OTSU);
			}

			//Mask outside of the cork
			Mat mask = Mat(roibin.rows, roibin.cols, roibin.type(), Scalar(255, 255, 255));
			circle(mask, Point(roibin.rows / 2, roibin.cols / 2), radius - OUTSIDE_SKIRT_IGNORE_PX, Scalar(0, 0, 0), -1, 8, 0);
			bitwise_or(mask, roibin, roibin);

			//Invert binary roi
			bitwise_not(roibin, roibin);
			imshow("ROI Binary", roibin);

			//Erode
			if(ERODE)
			{
				int size = 1;
				int kernel = 2 * size + 1;
				Mat element = getStructuringElement(MORPH_CROSS, Size(kernel, kernel), Point(size, size));
				erode(roibin, roibin, element);
				//imshow("ROI Eroded", roibin);
			}

			//Measure defective area
			//TODO <ADD CODE HERE>

			//Draw circle
			circle(image, center, 1, Scalar(0, 0, 255), 2, LINE_AA);
			circle(image, center, radius, Scalar(0, 255, 000), 1, LINE_AA);
		}

		//Display result
		imshow(WINDOW_NAME, image);
			
		//Keyboard input
		int key = waitKey(16);
		if(key != -1)
		{
			if(key == KEY_ESC)
			{
				return 0;
			}
			else if(key == KEY_LEFT)
			{
				readImage(--fnumber);
				std::cout << "Fname:" << fnumber << std::endl;
			}
			else if(key == KEY_RIGHT)
			{
				readImage(++fnumber);
				std::cout << "Fname:" << fnumber << std::endl;
			}
			else
			{
				std::cout << "Unkown key:" << key << std::endl;
			}
		}
	}

	return 0;
}
