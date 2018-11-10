#include <iostream>
#include <string>

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#define WINDOW_NAME "Cork"

#define KEY_ESC 27
#define KEY_LEFT 81
#define KEY_RIGHT 83

#define IMAGES_COUNT 20

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	VideoCapture cap(0);
	Mat src;

	int image = 1;

	//Prepare output window
	namedWindow(WINDOW_NAME, WINDOW_AUTOSIZE);
	setMouseCallback(WINDOW_NAME, [](int event, int x, int y, int flags, void *param)
	{
		//if(event == EVENT_LBUTTONDOWN){}
		//else if(event == EVENT_RBUTTONDOWN)
		//else if(event == EVENT_MBUTTONDOWN)
		//else if(event == EVENT_MOUSEMOVE)
	} , &src);

	while(1)
	{
		if(cap.isOpened())
		{
			cap >> src;
		}
		else
		{
			src = imread("data/" + std::to_string(image) + ".jpg", IMREAD_COLOR);
		}

		//Convert image to grayscale
		Mat gray;
		cvtColor(src, gray, COLOR_BGR2GRAY);
		imshow("Gray", gray);

		//Detect circles
		vector<Vec3f> circles;
		HoughCircles(gray, circles, HOUGH_GRADIENT, 1, gray.rows / 16, 100, 30, 1, 100);

		bool found = circles.size() > 0;
		
		//Draw circle outline
		for(size_t i = 0; i < circles.size(); i++)
		{
			Vec3i c = circles[i];
			Point center = Point(c[0], c[1]);
			int radius = c[2];

			//Isolate the circle
			Mat cork;
			Mat mask = Mat::zeros(src.rows, src.cols, CV_8UC1);
			circle(mask, center, radius, Scalar(255,255,255), -1, 8, 0);
			src.copyTo(cork, mask);
			imshow("Cork Circle", cork);

			//Debug draw
			circle(src, center, 1, Scalar(0,100,100), 2, LINE_AA);
			circle(src, center, radius, Scalar(255,0,255), 1, LINE_AA);
		}

		//


		imshow(WINDOW_NAME, src);
		
		int key = waitKey(16);
		if(key != -1)
		{
			if(key == KEY_ESC)
			{
				return 0;
			}
			else if(key == KEY_LEFT)
			{
				image--;
				image = image < 1 ? image = IMAGES_COUNT : image;
			}
			else if(key == KEY_RIGHT)
			{
				image++;
				image = image > IMAGES_COUNT ? image = 1 : image;
			}
			else
			{
				std::cout << key << std::endl;
			}
		}
	}

	return 0;
}
