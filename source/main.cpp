#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>

#include "tcamcamera.h"

#define CVUI_DISABLE_COMPILATION_NOTICES
#define CVUI_IMPLEMENTATION
#include "../lib/cvui.h"

#include "ImageStatus.hpp"
#include "Threshold.hpp"

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

//Input sources
#define FILE 0
#define USB_CAMERA 1
#define IP_CAMERA 2
#define TCAM_CAMERA 3

#define INPUT_SOURCE 0

//File range
#define IMAGES_START 0
#define IMAGES_COUNT 20

//IP Camera address
#define IP_CAMERA_ADDRESS "rtsp://admin:123456@192.168.0.10:554/live/ch0"
//#define IP_CAMERA_ADDRESS "rtsp://192.168.0.124:8080/video/h264"

using namespace gsttcam;
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
int LOW_CANNY_THRESH = 120;
int HIGH_CANNY_THRESH = 30; //The smaller it is, the more false circles may be detected.
int MIN_SIZE = 47;
int MAX_SIZE = 70;
int MIN_SPACING = 400;

//Automatic threshold
bool AUTOMATIC_THRESH = true;
bool AUTOMATIC_USE_OTSU_THRESH = false;
bool AUTOMATIC_USE_HIST_THRESH = true;

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
int OUTSIDE_SKIRT = 4;

/**
 * Create a GUI trackbar.
 */
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

/**
 * Create a GUI trackbar.
 */
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
 * Check if the image inside of the circle has an overall cork like tonality.
 */
bool hasCorkColorTone(const Mat src, Vec3f circle)
{
	//TODO <ADD CODE HERE>

	return true;
}

/**
 * Read image from file.
 */
Mat readImageFile(int index)
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

/**
 * Process a frame captured from the camera.
 */
void processFrame(Mat &image)
{
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
		
		//imshow("B", bgr[0]);
		imshow("G", bgr[1]);
		imshow("R", bgr[2]);

		gray = bgr[1];
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
		circle(mask, Point(roi.rows / 2, roi.cols / 2), radius - OUTSIDE_SKIRT, Scalar(0, 0, 0), -1, 8, 0);

		//Binarize the roi
		Mat roi_bin;

		if(BLUR_MASK)
		{
			medianBlur(roi, roi, BLUR_MASK_KSIZE);
		}

		if(AUTOMATIC_THRESH)
		{
			if(AUTOMATIC_USE_OTSU_THRESH)
			{
				//cout << "Automatic threshold: " << thresh << endl;
				double thresh = OTSU_THRESH_RATIO * otsuThreshold(roi, mask);
				threshold(roi, roi_bin, thresh, 255, THRESH_BINARY);
			}
			else// if(AUTOMATIC_USE_HIST_THRESH)
			{
				//cout << "Automatic threshold: " << thresh << endl;
				double thresh = histogramThreshold(roi, mask, HIST_THRESH_MIN_DIFF, HIST_THRESH_NEIGHBORHOOD, HIST_COLOR_FILTER, HIST_THRESH_BALANCE);
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
						AUTOMATIC_USE_HIST_THRESH = false;
					}
					cvui::space(12);
					if(cvui::checkbox("Histogram", &AUTOMATIC_USE_HIST_THRESH))
					{
						AUTOMATIC_USE_OTSU_THRESH = false;
					}
				}
			cvui::endRow();
			
			if(!AUTOMATIC_THRESH)
			{
				cvui::space(12);
				trackbar("Threshold", 200, &THRESHOLD_BIN, 10, 150, 1);
			}
			else if(AUTOMATIC_USE_HIST_THRESH)
			{
				cvui::space(12);
				trackbar("Min-diff", 200, &HIST_THRESH_MIN_DIFF, 5, 100, 1);
				cvui::space(12);
				trackbar("Neighborhood", 200, &HIST_THRESH_NEIGHBORHOOD, 3, 100, 1);
				cvui::space(12);
				trackbar("Neigh. Filter", 200, &HIST_COLOR_FILTER, 1, 100, 1);
				cvui::space(12);
				trackbar("Balance", 200, &HIST_THRESH_BALANCE, 0, 1, 1);	
			}
			
			cvui::space(12);
			trackbar("Skirt", 200, &OUTSIDE_SKIRT, 0, 20, 1);

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
}

/**
 * List available properties helper function.
 */
void listTcamProperties(TcamCamera &cam)
{
	//Get a list of all supported properties and print it out
	auto properties = cam.get_camera_property_list();
	std::cout << "Properties:" << std::endl;
	for(auto &prop : properties)
	{
		std::cout << prop->to_string() << std::endl;
	}
}

/**
 * Callback called for new images by the internal appsink.
 *
 * Called from a TcamCamera object using the "set_new_frame_callback" method.
 */
GstFlowReturn getFrameTcamCallback(GstAppSink *appsink, gpointer data)
{
	int width, height ;
	const GstStructure *str;

	//Cast gpointer to ImageStatus*
	ImageStatus *pdata = (ImageStatus*)data;
	pdata->counter++;

	//The following lines demonstrate, how to acces the image data in the GstSample.
	GstSample *sample = gst_app_sink_pull_sample(appsink);
	GstBuffer *buffer = gst_sample_get_buffer(sample);
	GstMapInfo info;

	gst_buffer_map(buffer, &info, GST_MAP_READ);
	
	if(info.data != NULL) 
	{
		//info.data contains the image data as blob of unsigned char 
		GstCaps *caps = gst_sample_get_caps(sample);

		//Get a string containg the pixel format, width and height of the image        
		str = gst_caps_get_structure(caps, 0);    

		if(strcmp(gst_structure_get_string (str, "format"),"BGRx") == 0)  
		{
			//Now query the width and height of the image
			gst_structure_get_int(str, "width", &width);
			gst_structure_get_int(str, "height", &height);

			//Create a cv::Mat, copy image data into that and save the image.
			pdata->frame.create(height, width, CV_8UC(4));
			memcpy(pdata->frame.data, info.data, width*height*4);

			processFrame(pdata->frame);
		}
	}
	
	//Calling Unref is important!
	gst_buffer_unmap (buffer, &info);
	gst_sample_unref(sample);

	//Set our flag of new image to true, so our main thread knows about a new image.
	return GST_FLOW_OK;
}

int main(int argc, char** argv)
{
	gst_init(&argc, &argv);

	VideoCapture cap;
	TcamCamera cam("46810320");

	ImageStatus status;
	status.counter = 0;

	if(INPUT_SOURCE == TCAM_CAMERA)
	{
		//Set video format, resolution and frame rate
		cam.set_capture_format("BGRx", FrameSize{1920, 1080}, FrameRate{30,1});

		//Register a callback to be called for each new frame
		cam.set_new_frame_callback(getFrameTcamCallback, &status);
		
		//Start the camera
		cam.start();
	}
	else if(INPUT_SOURCE == USB_CAMERA)
	{
		if(!cap.open(0))
		{
			cout << "Cork: Webcam not available." << endl;
		}

		//Set resolution
		cap.set(CAP_PROP_FRAME_WIDTH, 1280);
		cap.set(CAP_PROP_FRAME_HEIGHT, 720);

		if(cap.get(CAP_PROP_FRAME_HEIGHT) != 720 || cap.get(CAP_PROP_FRAME_WIDTH)!=1280)
		{
			cout << "Cork: Unable to set webcam resolution to 1280x720." << endl;
		}

		//Set exposure
		//cap.set(CAP_PROP_AUTO_EXPOSURE, 0);
		//cap.set(CAP_PROP_EXPOSURE, -100);

		//Focus
		//cap.set(CAP_PROP_FOCUS, 0);
	}
	else if(INPUT_SOURCE == IP_CAMERA)
	{
		if(!cap.open(IP_CAMERA_ADDRESS))
		{
			cout << "Cork: IP Camera not available." << endl;
		}
	}

	//Prepare output window
	cvui::init(WINDOW_NAME);

	while(true)
	{
		//Get image
		if(INPUT_SOURCE == FILE)
		{
			status.frame = readImageFile(fnumber);
			processFrame(status.frame);
		}
		else if(INPUT_SOURCE == USB_CAMERA || INPUT_SOURCE == IP_CAMERA)
		{
			if(cap.isOpened())
			{
				cap >> status.frame;
				processFrame(status.frame);
			}
		}

		//Keyboard input
		int key = waitKey(1);
		if(key != -1)
		{
			if(key == KEY_ESC)
			{
				return 0;
			}
			else if(INPUT_SOURCE == FILE)
			{
				if(key == KEY_LEFT)
				{
					readImageFile(--fnumber);
					cout << "Cork: Previous image, " << fnumber << endl;
				}
				else if(key == KEY_RIGHT)
				{
					readImageFile(++fnumber);
					cout << "Cork: Next image, " << fnumber << endl;
				}
			}
		}
	}

	if(INPUT_SOURCE == TCAM_CAMERA)
	{
		cam.stop();
	}

	return 0;
}
