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
	CameraConfig config;
	ImageStatus status;
	
	cv::VideoCapture cap;
	gsttcam::TcamCamera *cam = nullptr;

	int width = 640;
	int height = 480;

	CameraInput(CameraConfig _config)
	{
		config = _config;
	}

	/**
	 * Start the camera capture.
	 */
	void start()
	{

	}

	/**
	 * Capture frame from the camera.
	 */
	void update()
	{

	}

	/**
	 * Stop the camera capture.
	 */
	void stop()
	{

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