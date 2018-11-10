#include <iostream>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>

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

//Use adaptive thresold
#define ADAPTIVE_THRESH false

//Use otsu binarization
#define OTSU_THRESH false

//Use custom otsu treshold with mask
#define OTSU_THRESH_MASK false

//Deblur picture before start
#define DEBLUR_IMAGE false

#define OUTSIDE_SKIRT_IGNORE_PX 0

using namespace cv;
using namespace std;

//File number
int fnumber = IMAGES_START;

//Hough parameters
int minSpacing = 70;
int lowCannyThreshold = 100;
int highCannyThreshold = 30;
int minSize = 25;
int maxSize = 55;

//Threshold parameters
int threshold_bin = 60;

//Erosion configuration (only used if above 0)
int erosion = 0;

double otsu_8u_with_mask(const Mat1b src, const Mat1b& mask)
{
	const int N = 256;
	int M = 0;
	int i, j, h[N] = {0};

	for(i = 0; i < src.rows; i++)
	{
		const uchar* psrc = src.ptr(i);
		const uchar* pmask = mask.ptr(i);
		for(j = 0; j < src.cols; j++)
		{
			if(pmask[j])
			{
				h[psrc[j]]++;
				++M;
			}
		}
	}

	double mu = 0, scale = 1. / (M);
	for(i = 0; i < N; i++)
	{
		mu += i*(double)h[i];
	}

	mu *= scale;
	double mu1 = 0, q1 = 0;
	double max_sigma = 0, max_val = 0;

	for(i = 0; i < N; i++)
	{
		double p_i, q2, mu2, sigma;

		p_i = h[i] * scale;
		mu1 *= q1;
		q1 += p_i;
		q2 = 1. - q1;

		if(min(q1, q2) < FLT_EPSILON || max(q1, q2) > 1.0 - FLT_EPSILON)
		{
			continue;
		}

		mu1 = (mu1 + i*p_i) / q1;
		mu2 = (mu - q1*mu1) / q2;
		sigma = q1*q2*(mu1 - mu2)*(mu1 - mu2);

		if(sigma > max_sigma)
		{
			max_sigma = sigma;
			max_val = i;
		}
	}

	return max_val;
}

double threshold_with_mask(Mat1b& src, Mat1b& dst, double thresh, double maxval, int type, const Mat1b& mask = Mat1b())
{
	if(mask.empty() || (mask.rows == src.rows && mask.cols == src.cols && countNonZero(mask) == src.rows * src.cols))
	{
		//If empty mask, or all-white mask, use threshold
		thresh = threshold(src, dst, thresh, maxval, type);
	}
	else
	{
		//Use mask
		bool use_otsu = (type & THRESH_OTSU) != 0;
		if(use_otsu)
		{
			//If OTSU, get thresh value on mask only
			thresh = otsu_8u_with_mask(src, mask);

			//Remove THRESH_OTSU from type
			type &= THRESH_MASK;
		}

		//Apply threshold on all image
		thresh = threshold(src, dst, thresh, maxval, type);

		//Copy original image on inverted mask
		src.copyTo(dst, ~mask);
	}

	return thresh;
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
	namedWindow(WINDOW_NAME, WINDOW_AUTOSIZE);
	setMouseCallback(WINDOW_NAME, [](int event, int x, int y, int flags, void *param)
	{
		//if(event == EVENT_LBUTTONDOWN){}
		//else if(event == EVENT_RBUTTONDOWN)
		//else if(event == EVENT_MBUTTONDOWN)
		//else if(event == EVENT_MOUSEMOVE)
	} , &image);
	createTrackbar("File", WINDOW_NAME, &fnumber, 20, [](int event, void* param){});
	createTrackbar("Spacing", WINDOW_NAME, &minSpacing, 640, [](int event, void* param){});
	createTrackbar("Canny low", WINDOW_NAME, &lowCannyThreshold, 200, [](int event, void* param){});
	createTrackbar("Canny high", WINDOW_NAME, &highCannyThreshold, 200, [](int event, void* param){});
	createTrackbar("Min size", WINDOW_NAME, &minSize, 640, [](int event, void* param){});
	createTrackbar("Max size", WINDOW_NAME, &maxSize, 640, [](int event, void* param){});
	createTrackbar("Threshold", WINDOW_NAME, &threshold_bin, 255, [](int event, void* param){});
	createTrackbar("Erosion", WINDOW_NAME, &erosion, 10, [](int event, void* param){});

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
		HoughCircles(gray, circles, HOUGH_GRADIENT, 1, minSpacing, lowCannyThreshold, highCannyThreshold, minSize, maxSize);

		bool found = circles.size() > 0;

		//Draw circle outline
		for(size_t i = 0; i < circles.size(); i++)
		{
			Vec3i c = circles[i];
			Point center = Point(c[0], c[1]);
			int radius = c[2];
				
			//cout << "Circle point: (" << center.x << ", " << center.y << ")" << endl;
			//cout << "Circle radius: " << radius << endl;
			
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
				if(OTSU_THRESH)
				{
					if(OTSU_THRESH_MASK)
					{
						//double thresh = otsu_8u_with_mask(src, mask);
						//threshold(roi, roi_bin, 0, thresh, THRESH_BINARY, mask);
					}
					else
					{					
						threshold(roi, roi_bin, 0, 255, THRESH_BINARY + THRESH_OTSU);
					}
				}
				else
				{
					threshold(roi, roi_bin, threshold_bin, 255, THRESH_BINARY);
				}
			}

			//Mask outside of the cork in roi bin
			bitwise_or(mask, roi_bin, roi_bin);

			//Invert binary roi
			bitwise_not(roi_bin, roi_bin);
			//imshow("ROI Binary", roi_bin);

			//Erode
			if(erosion > 0)
			{
				int kernel = 2 * erosion + 1;
				Mat element = getStructuringElement(MORPH_RECT, Size(kernel, kernel), Point(erosion, erosion));
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

			//Draw defects
			for(int i = 0; i < roi_bin.rows; i++)
			{
				for(int j = 0; j < roi_bin.cols; j++)
				{
					int t = (i * roi_bin.cols + j);// * 3;

					if(roi_bin.data[t] > 0)
					{
						int k = ((i + roi_rect.y) * image.cols + (j + roi_rect.x)) * 3;

						image.data[k + 2] = (unsigned char) 255;
					}
				}
			}

			circle(image, center, 1, Scalar(255, 0, 0), 2, LINE_AA);
			circle(image, center, radius, Scalar(0, 255, 000), 1, LINE_AA);
			putText(image, to_string(defect) + "%", center, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 255));
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
