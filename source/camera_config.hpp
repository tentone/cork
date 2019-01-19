#pragma once

#include <string>
#include <fstream>
#include <iostream>

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
        tcamSerial = "";
        ipAddress = "";
        width = 640;
        height = 480;
        originalWidth = 1920;
        originalHeight = 1200;
        videoBackend = cv::CAP_ANY;
    }

    /**
     * Print the camera configuration to cout for debug.
     */
    void debugCout()
    {
        std::cout << this->input << std::endl;
        std::cout << this->usbNumber << std::endl;
        std::cout << this->tcamSerial << std::endl;
        std::cout << this->ipAddress << std::endl;
        std::cout << this->width << std::endl;
        std::cout << this->height << std::endl;
        std::cout << this->originalWidth << std::endl;
        std::cout << this->originalHeight << std::endl;
    }

    /**
     * Write configuration to output stream.
     */
    void saveFile(const std::string &fname)
    {
        std::ofstream out(fname);
        out << this->input << std::endl;
        out << this->usbNumber << std::endl;
        out << (this->tcamSerial.empty() ? "0" : this->tcamSerial) << std::endl;
        out << (this->ipAddress.empty() ? "0" : this->ipAddress) << std::endl;
        out << this->width << std::endl;
        out << this->height << std::endl;
        out << this->originalWidth << std::endl;
        out << this->originalHeight << std::endl;
        out.close();
    }

    static bool fileExists(const std::string &name)
    {
        std::ifstream f(name.c_str());
        return f.good();
    }

    /**
     * Read configuration from file.
     *
     * Returns true if the file was loaded successfully, or false in case of error.
     */
    bool loadFile(const std::string &fname)
    {
        if(!fileExists(fname))
        {
            return false;
        }

        std::ifstream in(fname);
        in >> this->input;
        in >> this->usbNumber;
        in >> this->tcamSerial;
        in >> this->ipAddress;
        in >> this->width;
        in >> this->height;
        in >> this->originalWidth;
        in >> this->originalHeight;
        in.close();

        return true;
    }
};



