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

/**
 * Handles camera input, uses a configuration object to select the right camera configuration.
 */
class CameraInput
{
public:
	CameraConfig config;
	VideoCapture cap;
	TcamCamera cam;
	ImageStatus status;

	int width = 640;
	int height = 480;

	CameraInput(CameraConfig _config)
	{
		config = _config;
	}

	void start()
	{
		if(config.input == CameraConfiguration::TCAM_CAMERA)
		{
			int width = 1920;
			int height = 1200;

			status.frame.create(height, width, CV_8UC(4));

			cam.set_capture_format("BGRx", FrameSize{width, height}, FrameRate{50, 1});
			cam.set_new_frame_callback(getFrameTcamCallback, &status);
			cam.start();
			
			shared_ptr<Property> exposureAuto = cam.get_property("Exposure Auto");
			exposureAuto->set(cam, 0);
			
			shared_ptr<Property> exposureValue = cam.get_property("Exposure");
			exposureValue->set(cam, 1e3);

			shared_ptr<Property> gainAuto = cam.get_property("Gain Auto");
			gainAuto->set(cam, 0);
			
			shared_ptr<Property> gainValue = cam.get_property("Gain");
			gainValue->set(cam, 30);

			shared_ptr<Property> brightness = cam.get_property("Brightness");
			brightness->set(cam, 50);

			//listTcamProperties(cam);
		}
		else if(config.input == CameraConfiguration::USB_CAMERA)
		{
			if(!cap.open(1))
			{
				std::cout << "Cork: Webcam not available." << std::endl;
			}

			cap.set(CAP_PROP_FRAME_WIDTH, width);
			cap.set(CAP_PROP_FRAME_HEIGHT, height);

			if(cap.get(CAP_PROP_FRAME_HEIGHT) != height || cap.get(CAP_PROP_FRAME_WIDTH) != width)
			{
				std::cout << "Cork: Unable to set usb webcam resolution." << std::endl;
			}
		}
		else if(config.input == CameraConfiguration::IP_CAMERA)
		{
			if(!cap.open(IP_CAMERA_ADDRESS))
			{
				std::cout << "Cork: IP Camera not available." << std::endl;
			}
		}
	}

	void update()
	{
		if(config.input == CameraConfiguration::FILE)
		{
			status.frame = readImageFile(fnumber);
			processFrame(status.frame);
		}
		else if(config.input == CameraConfiguration::USB_CAMERA || config.input == CameraConfiguration::IP_CAMERA)
		{
			if(cap.isOpened())
			{
				cap >> status.frame;
				processFrame(status.frame);
			}
		}
		else if(config.input == CameraConfiguration::TCAM_CAMERA)
		{
			wait();
		}
	}

	void stop()
	{
		if(config.input == CameraConfiguration::TCAM_CAMERA)
		{
			wait();
		}
	}

	static void wait()
	{
		this_thread::sleep_for(chrono::duration<int, ratio<1,1000>>(200));
	}
	
	/**
	 * List available properties helper function.
	 */
	static void listTcamProperties(TcamCamera &cam)
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