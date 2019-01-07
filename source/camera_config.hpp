#pragma once

#include <string>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

/**
 * Stores the camera configuration to be used.
 */
class CameraConfig
{
public:
    /**
     * Use to capture image from files.
     */
    const static int FILE = 0;

    /**
     * Used to capture image from USB devices or from video devices listed under /dev/video#.
     */
    const static int USB = 1;

    /**
     * Use for IP cameras.
     */
    const static int IP = 2;

    /**
     * TCam is used for cameras made by TIS (The Imaging Source).
     *
     * Uses their own backend for image capture.
     */
    const static int TCAM = 3;

    /**
     * Indicates the camera input source.
     */
    int input;

    /**
     * The USB camera number to use.
     *
     * Used when input source set to USB.
     */
    int usbNumber;

    /**
     * Serial number of the camera to use.
     *
     * Used when input source set to TCAM.
     */
    std::string tcamSerial;

    /**
     * Full IP address, username and password of the camera.
     *
     * Used when input source set to IP.
     *
     * Examples rtsp://admin:123456@192.168.0.10:554/live/ch0 or rtsp://192.168.0.124:8080/video/h264
     */
    std::string ipAddress;

    /**
     * Output image width.
     */
    int width;

    /**
     * Output image height.
     */
    int height;

    /**
     * Original video input width.
     *
     * Used for TCam image capture.
     */
    int originalWidth;

    /**
     * Original video input height.
     *
     * Used for TCam image capture.
     */
    int originalHeight;

    /**
     * OpenCV video capture backend to use.
     */
    int videoBackend;

    CameraConfig()
    {
        input = CameraConfig::TCAM;
        usbNumber = 0;
        tcamSerial = "46810320";
        ipAddress = "rtsp://admin:123456@192.168.0.10:554/live/ch0";
        width = 768; //TCam uses 768
        height = 480;
        originalWidth = 1920;
        originalHeight = 1200;
        videoBackend = cv::CAP_ANY;
    }
};
