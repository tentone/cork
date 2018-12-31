#pragma once

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

#include "camera_config.hpp"
#include "image_status.hpp"

/**
 * Handles camera input, uses a configuration object to select the right camera configuration.
 */
class CameraInput
{
public:
	CameraConfig cameraConfig;
	ImageStatus status;
	
	cv::VideoCapture cap;
	gsttcam::TcamCamera cam = nullptr;

	CameraInput(CameraConfig _cameraConfig)
	{
		cameraConfig = _cameraConfig;
	}

	/**
	 * Start the camera capture.
	 */
	void start()
	{
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
		}
		else if(cameraConfig.input == CameraConfig::USB)
		{
			if(!cap.open(cameraConfig.usbNumber))
			{
				std::cout << "Cork: Webcam not available." << std::endl;
			}

			/*cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
			cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

			if(cap.get(cv::CAP_PROP_FRAME_HEIGHT) != 720 || cap.get(cv::CAP_PROP_FRAME_WIDTH) != 1280)
			{
				std::cout << "Cork: Unable to set webcam resolution to 1280x720." << std::endl;
			}*/
		}
		else if(cameraConfig.input == CameraConfig::IP)
		{
			if(!cap.open(cameraConfig.ipAddress))
			{
				std::cout << "Cork: IP Camera not available." << std::endl;
			}
		}
	}

	/**
	 * Capture frame from the camera.
	 */
	void update()
	{
		//Get image
		if(cameraConfig.input == CameraConfig::FILE)
		{
			status.frame = readImageFile(fnumber);

			//TODO <PROCESSING CALLBACK>
			//processFrame(status.frame);
		}
		else if(cameraConfig.input == CameraConfig::USB || cameraConfig.input == CameraConfig::IP)
		{
			if(cap.isOpened())
			{
				cap >> status.frame;

				//TODO <PROCESSING CALLBACK>
				//processFrame(status.frame);
			}
		}
		else if(cameraConfig.input == CameraConfig::TCAM)
		{
			wait();
		}
	}

	/**
	 * Stop the camera capture.
	 */
	void stop()
	{
		if(cameraConfig.input == CameraConfig::TCAM)
		{
			cam.stop();
		}
	}

	/**
	 * Callback called for new images by the internal appsink.
	 *
	 * Called from a TcamCamera object using the "set_new_frame_callback" method.
	 */
	static GstFlowReturn getFrameTcamCallback(GstAppSink *appsink, gpointer data)
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
					
				//TODO <ADD CODE HERE>
				//processFrame(pdata->resized);
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

	/**
	 * Put the calling thread to sleep for a bit. 
	 */
	static void wait()
	{
		std::this_thread::sleep_for(std::chrono::duration<int, std::ratio<1,1000>>(200));
	}

	/**
	 * List available properties helper function.
	 */
	static void listTcamProperties(gsttcam::TcamCamera &cam)
	{
		//Get a list of all supported properties and print it out
		auto properties = cam.get_camera_property_list();
		std::cout << "Properties:" << std::endl;
		for(auto &prop : properties)
		{
			std::cout << prop->to_string() << std::endl;
		}
	}
};