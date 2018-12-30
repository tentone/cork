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