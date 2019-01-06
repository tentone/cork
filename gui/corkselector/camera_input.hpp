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

#include "mainwindow.hpp"
#include "camera_config.hpp"
#include "image_status.hpp"

#define MEASURE_PERFORMANCE false

#define USE_THREAD false

/**
 * Handles camera input, uses a configuration object to select the right camera configuration.
 */
class CameraInput
{
public:
    /**
     * Obligatory camera configuration object.
     */
    CameraConfig cameraConfig;

    /**
     * Last captured frame status.
     */
    ImageStatus status;

    /**
     * OpenCV video capture object used to get image from USB or from IP camera.
     */
    cv::VideoCapture *cap = nullptr;

    /**
     * Used to capture data from a TCam.
     *
     * E.g. the ImagingSource camera used.
     */
    gsttcam::TcamCamera *cam = nullptr;

    /**
     * File path and name prefix.
     *
     * File are expected to be named xxx/yyy/zzz/0.jpg
     */
    std::string filePrefix = "data/";

    /**
     * Indicated if the camera input is running or not.
     *
     * Used for the camera capture threads to know when to stop.
     */
    bool running = false;

    /**
     * Current file number.
     */
    int fileNumber = 0;
    int fileStart = 0;
    int fileCount = 20;

    /**
     * Callback function used to process captured frame.
     */
    //void *frameCallback (cv::Mat &mat, MainWindow *context);
    int frameCallback;

    /**
     * Framem callback context.
     */
    MainWindow *context;

    /**
     * Constructor from camera configuration object.
     */
    CameraInput(CameraConfig _cameraConfig)
    {
        cameraConfig = _cameraConfig;
    }

    /**
     * Start the camera capture.
     */
    void start()
    {
        running = true;
        status.counter = 0;

        if(cameraConfig.input == CameraConfig::TCAM)
        {
            int width = 1920;
            int height = 1200;

            status.frame.create(height, width, CV_8UC(4));

            cam = new gsttcam::TcamCamera(cameraConfig.tcamSerial);
            cam->set_capture_format("BGRx", gsttcam::FrameSize{width, height}, gsttcam::FrameRate{50, 1});

            /*
             * Callback called for new images by the internal appsink.
             *
             * Called from a TcamCamera object using the "set_new_frame_callback" method.
             */
            cam->set_new_frame_callback([=] (GstAppSink *appsink, gpointer data) -> GstFlowReturn
            {
                #if MEASURE_PERFORMANCE
                    int64 init = cv::getTickCount();
                #endif

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

                        /*
                        //Crop image to match aspect ratio
                        cv::Rect crop;
                        crop.x = (768 - 640) / 2;
                        crop.y = 0;
                        crop.width = 640;
                        crop.height = 480;
                        pdata->resized = pdata->resized(crop);
                        */

                        //Frame processing callback
                        frameCallback(pdata->resized, context);
                    }
                    else
                    {
                        std::cout << "Cork: Not the expected pixel format." << std::endl;
                    }
                }

                //Clean up, unref and unmap buffers (to prevent leaks).
                gst_buffer_unmap(buffer, &info);
                gst_sample_unref(sample);

                #if MEASURE_PERFORMANCE
                    int64 end = cv::getTickCount();
                    double secs = (end - init) / cv::getTickFrequency();
                    std::cout << "Cork: Processing time was " << secs << " s." << std::endl;
                #endif

                //Set our flag of new image to true, so our main thread knows about a new image.
                return GST_FLOW_OK;
            }, &status);

            cam->start();

            std::shared_ptr<gsttcam::Property> exposureAuto = cam->get_property("Exposure Auto");
            exposureAuto->set(*cam, 0);

            std::shared_ptr<gsttcam::Property> exposureValue = cam->get_property("Exposure");
            exposureValue->set(*cam, 1e3);

            std::shared_ptr<gsttcam::Property> gainAuto = cam->get_property("Gain Auto");
            gainAuto->set(*cam, 0);

            std::shared_ptr<gsttcam::Property> gainValue = cam->get_property("Gain");
            gainValue->set(*cam, 30);

            std::shared_ptr<gsttcam::Property> brightness = cam->get_property("Brightness");
            brightness->set(*cam, 50);
        }
        else if(cameraConfig.input == CameraConfig::USB)
        {
            cap = new cv::VideoCapture();

            if(!cap->open(cameraConfig.usbNumber))
            {
                std::cout << "Cork: Webcam not available." << std::endl;
            }

            cap->set(cv::CAP_PROP_FRAME_HEIGHT, cameraConfig.height);
            cap->set(cv::CAP_PROP_FRAME_WIDTH, cameraConfig.width);

            if(cap->get(cv::CAP_PROP_FRAME_HEIGHT) != cameraConfig.height || cap->get(cv::CAP_PROP_FRAME_WIDTH) != cameraConfig.width)
            {
                std::cout << "Cork: Unable to set webcam resolution." << std::endl;
            }

            #if USE_THREAD
                std::thread thread([=]()
                {
                    while(true)
                    {
                        if(cap->isOpened())
                        {
                            *cap >> status.frame;
                            frameCallback(status.frame, context);
                        }

                        if(!running)
                        {
                            break;
                        }
                    }
                });
            #endif
        }
        else if(cameraConfig.input == CameraConfig::IP)
        {
            cap = new cv::VideoCapture();

            if(!cap->open(cameraConfig.ipAddress))
            {
                std::cout << "Cork: IP Camera not available." << std::endl;
            }

            #if USE_THREAD
                std::thread thread([=]()
                {
                    while(true)
                    {
                        if(cap->isOpened())
                        {
                            *cap >> status.frame;
                            frameCallback(status.frame, context);
                        }

                        if(!running)
                        {
                            break;
                        }
                    }
                });
            #endif
        }
        else if(cameraConfig.input == CameraConfig::FILE)
        {
            status.frame = readImageFile(fileNumber);
            frameCallback(status.frame, context);
        }
    }

    /**
     * Capture frame from the camera.
     */
    void update()
    {
        #if !MEASURE_PERFORMANCE
            if(cameraConfig.input == CameraConfig::USB || cameraConfig.input == CameraConfig::IP)
            {
                if(cap->isOpened())
                {
                    *cap >> status.frame;
                    frameCallback(status.frame, context);
                }
            }
            else
            {
                //wait();
            }
        #else
            //wait();
        #endif
    }

    /**
     * Stop the camera capture.
     */
    void stop()
    {
        running = false;

        if(cameraConfig.input == CameraConfig::TCAM)
        {
            cam->stop();
        }
    }

    /**
     * Read next image file.
     */
    void nextFile()
    {
        if(cameraConfig.input == CameraConfig::FILE)
        {
            status.frame = readImageFile(++fileNumber);
            std::cout << "Cork: Next image, " << fileNumber << std::endl;
            frameCallback(status.frame, context);
        }
    }

    /**
     * Read previous image from file.
     */
    void previousFile()
    {
        if(cameraConfig.input == CameraConfig::FILE)
        {
            status.frame = readImageFile(--fileNumber);
            std::cout << "Cork: Previous image, " << fileNumber << std::endl;
            frameCallback(status.frame, context);
        }
    }

    /**
     * Read image from file.
     */
    cv::Mat readImageFile(int index)
    {
        fileNumber = index;

        if(fileNumber > fileCount)
        {
            fileNumber = fileStart;
        }
        if(fileNumber < fileStart)
        {
            fileNumber = fileCount;
        }

        return cv::imread(filePrefix + std::to_string(fileNumber) + ".jpg", cv::IMREAD_COLOR);
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

        std::cout << "Cork: TCam properties:" << std::endl;

        for(auto &prop : properties)
        {
            std::cout << prop->to_string() << std::endl;
        }
    }
};