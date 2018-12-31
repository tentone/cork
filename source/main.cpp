#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <thread>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>

#include "tcamcamera.h"

#include "threshold.hpp"
#include "config.hpp"
#include "cork_analyser.hpp"

#include "gui.hpp"

#include "input/camera_input.hpp"
#include "input/camera_config.hpp"
#include "input/image_status.hpp"

#define KEY_ESC 27
#define KEY_LEFT 81
#define KEY_RIGHT 83
#define KEY_UP 82
#define KEY_DOWN 84
#define KEY_W 119
#define KEY_S 115

#define PI 3.14159265359

//File range
#define IMAGES_START 0
#define IMAGES_COUNT 20

//File number
bool corkFound = false;
double corkDefect = 0.0;

/**
 * Cork detector configuration.
 */
Configuration config;

/**
 * Camera input configuration.
 */
CameraConfig cameraConfig;


bool DEBUG = true;
std::string WINDOW = "Cork";


bool saveNextFrame = false;
int saveFrameCounter = 0;

/**
 * Process a frame captured from the camera.
 */
void processFrame(cv::Mat &image)
{
	if(saveNextFrame)
	{
		std::cout << "Save frame" << std::endl;

		saveNextFrame = false;
		cv::imwrite("./" + std::to_string(saveFrameCounter++) + ".png", image);
	}

	//Deblur the image
	if(config.blurGlobal)
	{
		cv::medianBlur(image, image, config.blurGlobalKSize);
	}
	
	cv::Mat gray;

	//Split color channels
	if(config.splitColorChannels)
	{
		cv::Mat bgr[3];
		cv::split(image, bgr);

		//Use the gree channel
		gray = bgr[1];
	}
	//Convert image to grayscale
	else
	{
		cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
	}

	//Detect circles
	std::vector<cv::Vec3f> circles;
	cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 1, config.minSpacing, config.lowCannyThresh, config.highCannyThresh, config.minSize, config.maxSize);

	bool found = circles.size() > 0;

	//Iterate all found circles
	for(size_t i = 0; i < circles.size(); i++)
	{
		cv::Vec3i c = circles[i];
		cv::Point center = cv::Point(c[0], c[1]);
		int radius = c[2];
		
		//Check if fully inside of the image
		if(radius > center.x || radius > center.y || radius + center.x > gray.cols || radius + center.y > gray.rows)
		{
			continue;
		}

		//Create the roi
		cv::Rect roi_rect = cv::Rect(center.x - radius, center.y - radius, radius * 2, radius * 2);
		cv::Mat roi = cv::Mat(gray, roi_rect);
		
		//Circle mask for the roi
		cv::Mat mask = cv::Mat(roi.rows, roi.cols, roi.type(), cv::Scalar(255, 255, 255));
		cv::circle(mask, cv::Point(roi.rows / 2, roi.cols / 2), radius - config.outsizeSkirt, cv::Scalar(0, 0, 0), -1, 8, 0);

		//Binarize the roi
		cv::Mat roi_bin;

		if(config.blurMask)
		{
			cv::medianBlur(roi, roi, config.blurMaskKSize);
		}

		if(config.automaticThresh)
		{
			if(config.automaticUseOtsuThresh)
			{
				double thresh = Threshold::otsuMask(roi, mask);
				std::cout << "Otsu Automatic threshold: " << thresh << std::endl;
				cv::threshold(roi, roi_bin, thresh, 255, cv::THRESH_BINARY);
			}
			else// if(config.automaticUseHistogramThresh)
			{
				double thresh = Threshold::histogram(roi, mask, config.histThreshMinDiff, config.histThreshNeighborhood, config.histThreshColorFilter, config.histThreshBalance);
				std::cout << "Histogram automatic threshold: " << thresh << std::endl;
				cv::threshold(roi, roi_bin, thresh, 255, cv::THRESH_BINARY);
			}
		}
		else if(config.semiAutoThresh)
		{
			double thresh = Threshold::otsuMask(roi, mask);
			thresh = (thresh * config.semiAutoThreshTolerance) + (config.thresholdValue * (1 - config.semiAutoThreshTolerance));

			std::cout << "Semi Automatic threshold: " << thresh << std::endl;
			cv::threshold(roi, roi_bin, thresh, 255, cv::THRESH_BINARY);
		}
		else
		{
			cv::threshold(roi, roi_bin, config.thresholdValue, 255, cv::THRESH_BINARY);
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

		//std::cout << "cv::Points: " << points << std::endl;
		//std::cout << "Resolution: " << (roi_bin.rows * roi_bin.cols) << std::endl;
		//std::cout << "Count: " << count << std::endl;
		//std::cout << "Area: " << area << std::endl;
		//std::cout << "Defect: " << defect << "%" << std::endl;

		//Draw debug information
		if(DEBUG)
		{
			int channels = image.channels();

			//Draw defect
			for(int i = 0; i < roi_bin.rows; i++)
			{
				for(int j = 0; j < roi_bin.cols; j++)
				{
					int t = (i * roi_bin.cols + j);

					if(roi_bin.data[t] > 0)
					{
						int k = ((i + roi_rect.y) * image.cols + (j + roi_rect.x)) * channels;

						image.data[k + 2] = (unsigned char) 255;
					}
				}
			}
			//Cicle position
			cv::circle(image, center, 1, cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
			cv::circle(image, center, radius, cv::Scalar(0, 255, 000), 1, cv::LINE_AA);
			cv::putText(image, std::to_string(defect) + "%", center, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255));
		}
	}

	if(DEBUG)
	{
		cvgui::drawConfigEditor(WINDOW, image, config);
	}

	//Keyboard input
	int key = cv::waitKey(1);
	if(key != -1)
	{
		if(key == KEY_ESC)
		{
			saveNextFrame = true;
		}
		
		if(cameraConfig.input == CameraConfig::FILE)
		{
			if(key == KEY_LEFT)
			{
				
			}
			else if(key == KEY_RIGHT)
			{
				
			}
		}
	}
}

/**
 * Callback called for new images by the internal appsink.
 *
 * Called from a TcamCamera object using the "set_new_frame_callback" method.
 */
GstFlowReturn getFrameTcamCallback(GstAppSink *appsink, gpointer data)
{
	int64 init = cv::getTickCount();

	int width, height;
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

		if(strcmp(gst_structure_get_string(str, "format"), "BGRx") == 0)  
		{
			//Now query the width and height of the image
			gst_structure_get_int(str, "width", &width);
			gst_structure_get_int(str, "height", &height);

			//Create a cv::Mat, copy image data into that and save the image.
			pdata->frame.data = info.data;

			resize(pdata->frame, pdata->resized, cv::Size(768, 480));
			processFrame(pdata->resized);
		}
		else
		{
			std::cout << "Cork: Not the expected pixel format." << std::endl;
		}
	}
	
	//Clean up, unref and unmap buffers (to prevent leaks).
	gst_buffer_unmap(buffer, &info);
	gst_sample_unref(sample);

	int64 end = cv::getTickCount();
	double secs = (end - init) / cv::getTickFrequency();

	std::cout << "Cork: Processing time was " << secs << " s." << std::endl;

	//Set our flag of new image to true, so our main thread knows about a new image.
	return GST_FLOW_OK;
}

void wait()
{
	std::this_thread::sleep_for(std::chrono::duration<int, std::ratio<1,1000>>(200));
}

int main(int argc, char** argv)
{
	gst_init(&argc, &argv);

	cv::VideoCapture cap;
	gsttcam::TcamCamera cam(cameraConfig.tcamSerial);

	ImageStatus status;
	status.counter = 0;

	if(cameraConfig.input == CameraConfig::TCAM)
	{
		int width = 1920;
		int height = 1200;

		status.frame.create(height, width, CV_8UC(4));

		cam.set_capture_format("BGRx", gsttcam::FrameSize{width, height}, gsttcam::FrameRate{50, 1});
		cam.set_new_frame_callback(getFrameTcamCallback, &status);
		cam.start();
		
		std::shared_ptr<gsttcam::Property> exposureAuto = cam.get_property("Exposure Auto");
		exposureAuto->set(cam, 0);
		
		std::shared_ptr<gsttcam::Property> exposureValue = cam.get_property("Exposure");
		exposureValue->set(cam, 1e3);

		std::shared_ptr<gsttcam::Property> gainAuto = cam.get_property("Gain Auto");
		gainAuto->set(cam, 0);
		
		std::shared_ptr<gsttcam::Property> gainValue = cam.get_property("Gain");
		gainValue->set(cam, 30);

		std::shared_ptr<gsttcam::Property> brightness = cam.get_property("Brightness");
		brightness->set(cam, 50);

		//listTcamProperties(cam);
	}
	else if(cameraConfig.input == CameraConfig::USB)
	{
		if(!cap.open(cameraConfig.usbNumber))
		{
			std::cout << "Cork: Webcam not available." << std::endl;
		}

		//cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
		//cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

		if(cap.get(cv::CAP_PROP_FRAME_HEIGHT) != 720 || cap.get(cv::CAP_PROP_FRAME_WIDTH) != 1280)
		{
			std::cout << "Cork: Unable to set webcam resolution to 1280x720." << std::endl;
		}
	}
	else if(cameraConfig.input == CameraConfig::IP)
	{
		if(!cap.open(cameraConfig.ipAddress))
		{
			std::cout << "Cork: IP Camera not available." << std::endl;
		}
	}

	//Prepare output window
	if(DEBUG)
	{
		cvui::init(WINDOW);
	}

	while(true)
	{
		if(cameraConfig.input == CameraConfig::USB || cameraConfig.input == CameraConfig::IP)
		{
			if(cap.isOpened())
			{
				cap >> status.frame;
				processFrame(status.frame);
			}
		}
		else if(cameraConfig.input == CameraConfig::TCAM)
		{
			wait();
		}
	}

	if(cameraConfig.input == CameraConfig::TCAM)
	{
		cam.stop();
	}

	return 0;
}
