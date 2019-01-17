#pragma once

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <thread>

#include <QThread>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>

#include "lib/tcamcamera.h"

#include "mainwindow.hpp"
#include "camera_config.hpp"
#include "image_status.hpp"

//If true print performance measurements into the terminal
#define MEASURE_PERFORMANCE_INPUT false

//If true print automatically crops TCam image to match aspect ratio
#define CROP_TCAM_IMAGE false

//If set true uses self created processing threads for camera input
#define USE_THREAD true

//If set true it skips the next frames until the actual frame is processed
#define USE_FRAME_SKIP true

/**
 * Handles camera input, uses a configuration object to select the right camera configuration.
 */
class CameraInput
{
public:
    /**
     * Obligatory camera configuration object.
     */
    CameraConfig *cameraConfig;

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
     * Thread object used to colled and process camera input.
     *
     * Is used for USB and IP camera modes.
     */
    QThread *thread;

    /**
     * Current file number.
     */
    int fileNumber = 0;
    int fileStart = 0;
    int fileCount = 20;

#if USE_FRAME_SKIP
    /**
     * Indicates if a frame is being processed.
     */
    bool processingFrame = false;
#endif

    /**
     * Callback function used to process captured frame.
     */
    void (*frameCallback)(cv::Mat &mat);

    /**
     * Constructor from camera configuration object.
     */
    CameraInput(CameraConfig *_cameraConfig)
    {
        cameraConfig = _cameraConfig;
    }

    /**
     * Start the camera capture.
     */
    void start()
    {
        if(running == true)
        {
            return;
        }

        running = true;
        status.counter = 0;

        if(cameraConfig->input == CameraConfig::TCAM)
        {
            status.frame.create(cameraConfig->originalHeight, cameraConfig->originalWidth, CV_8UC(4));

            cam = new gsttcam::TcamCamera(cameraConfig->tcamSerial);
            cam->set_capture_format("BGRx", gsttcam::FrameSize{cameraConfig->originalWidth, cameraConfig->originalHeight}, gsttcam::FrameRate{50, 1});

            /*
             * Callback called for new images by the internal appsink.
             *
             * Called from a TcamCamera object using the "set_new_frame_callback" method.
             */
            cam->set_new_frame_callback([&] (GstAppSink *appsink, gpointer data) -> GstFlowReturn
            {
                #if USE_FRAME_SKIP
                    if(processingFrame)
                    {
                        return GST_FLOW_OK;
                    }
                    processingFrame = true;
                #endif

                #if MEASURE_PERFORMANCE_INPUT
                    int64 init = cv::getTickCount();
                #endif
                const GstStructure *str;

                //Cast gpointer to ImageStatus*
                ImageStatus *pdata = static_cast<ImageStatus*>(data);
                pdata->counter++;

                //The following lines demonstrate, how to acces the image data in the GstSample.
                GstSample *sample = gst_app_sink_pull_sample(appsink);
                GstBuffer *buffer = gst_sample_get_buffer(sample);
                GstMapInfo info;

                gst_buffer_map(buffer, &info, GST_MAP_READ);

                if(info.data != nullptr)
                {
                    //info.data contains the image data as blob of unsigned char
                    GstCaps *caps = gst_sample_get_caps(sample);

                    //Get a string containg the pixel format, width and height of the image
                    str = gst_caps_get_structure(caps, 0);

                    if(strcmp(gst_structure_get_string(str, "format"), "BGRx") == 0)
                    {
                        //Now query the width and height of the image
                        //int width, height;
                        //gst_structure_get_int(str, "width", &width);
                        //gst_structure_get_int(str, "height", &height);

                        //Create a cv::Mat, copy image data into that and save the image.
                        pdata->frame.data = info.data;

                        if(pdata->frame.empty())
                        {
                            std::cout << "Cork: TCam frame is empty." << std::endl;
                            return GST_FLOW_OK;
                        }

                        #if CROP_TCAM_IMAGE
                            //Crop image if necessary
                            double originalRatio = static_cast<double>(cameraConfig->originalWidth) / static_cast<double>(cameraConfig->originalHeight);
                            double ratio = static_cast<double>(cameraConfig->width) / static_cast<double>(cameraConfig->height);

                            if(originalRatio > ratio)
                            {
                                int targetWidth = static_cast<int>(cameraConfig->originalWidth * (1 - (originalRatio - ratio)));

                                std::cout << "Cork: Crop image " << targetWidth << std::endl;

                                //Crop image to match aspect ratio
                                cv::Rect crop;
                                crop.x = (cameraConfig->originalWidth - targetWidth) / 2;
                                crop.y = 0;
                                crop.width = targetWidth;
                                crop.height = cameraConfig->originalHeight;
                                pdata->frame = pdata->frame(crop);
                            }
                        #endif

                        //Resize image
                        resize(pdata->frame, pdata->resized, cv::Size(cameraConfig->width, cameraConfig->height));

                        //Frame processing callback
                        frameCallback(pdata->resized);
                    }
                    else
                    {
                        std::cout << "Cork: Not the expected pixel format." << std::endl;
                    }
                }

                //Clean up, unref and unmap buffers (to prevent leaks).
                gst_buffer_unmap(buffer, &info);
                gst_sample_unref(sample);

                #if MEASURE_PERFORMANCE_INPUT
                    int64 end = cv::getTickCount();
                    double secs = (end - init) / cv::getTickFrequency();
                    std::cout << "Cork: TCAM Capture + Processing time was " << secs << " s." << std::endl;
                #endif

                #if USE_FRAME_SKIP
                    processingFrame = false;
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
        else if(cameraConfig->input == CameraConfig::USB || cameraConfig->input == CameraConfig::IP)
        {
            cap = new cv::VideoCapture();

            if(cameraConfig->input == CameraConfig::USB)
            {
                if(!cap->open(cameraConfig->usbNumber, cameraConfig->videoBackend))
                {
                    std::cout << "Cork: Webcam not available." << std::endl;
                    return;
                }

                cap->set(cv::CAP_PROP_FRAME_HEIGHT, cameraConfig->height);
                cap->set(cv::CAP_PROP_FRAME_WIDTH, cameraConfig->width);

                if(cap->get(cv::CAP_PROP_FRAME_HEIGHT) != cameraConfig->height || cap->get(cv::CAP_PROP_FRAME_WIDTH) != cameraConfig->width)
                {
                    std::cout << "Cork: Unable to set webcam resolution." << std::endl;
                }
            }
            else if(cameraConfig->input == CameraConfig::IP)
            {
                if(!cap->open(cameraConfig->ipAddress))
                {
                    std::cout << "Cork: IP Camera not available." << std::endl;
                    return;
                }
            }

            #if USE_THREAD
                thread = QThread::create([=]()
                {
                    while(true)
                    {
                        if(cap->isOpened())
                        {
                            *cap >> status.frame;
                            frameCallback(status.frame);
                            wait();
                        }

                        if(!running)
                        {
                            break;
                        }
                    }
                });
                thread->start();
            #endif
        }
        else if(cameraConfig->input == CameraConfig::FILE)
        {
            status.frame = readImageFile(fileNumber);
            frameCallback(status.frame);
        }
    }

    /**
     * Capture frame from the camera.
     */
    void update()
    {
        #if !USE_THREAD
            if(cameraConfig->input == CameraConfig::USB || cameraConfig->input == CameraConfig::IP)
            {
                if(cap->isOpened())
                {
                    *cap >> status.frame;
                    frameCallback(status.frame);
                }
            }
        #endif
    }

    /**
     * Stop the camera capture.
     */
    void stop()
    {
        if(running == false)
        {
            return;
        }

        running = false;

        if(cameraConfig->input == CameraConfig::TCAM)
        {
            cam->stop();
            delete cam;
        }
        else if(cameraConfig->input == CameraConfig::USB || cameraConfig->input == CameraConfig::IP)
        {
            cap->release();
            delete cap;
        }
    }

    /**
     * Read next image file.
     */
    void nextFile()
    {
        if(cameraConfig->input == CameraConfig::FILE)
        {
            status.frame = readImageFile(++fileNumber);
            std::cout << "Cork: Next image, " << fileNumber << std::endl;
            frameCallback(status.frame);
        }
    }

    /**
     * Read previous image from file.
     */
    void previousFile()
    {
        if(cameraConfig->input == CameraConfig::FILE)
        {
            status.frame = readImageFile(--fileNumber);
            std::cout << "Cork: Previous image, " << fileNumber << std::endl;
            frameCallback(status.frame);
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
        std::this_thread::sleep_for(std::chrono::duration<int, std::ratio<1,1000>>(20));
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
