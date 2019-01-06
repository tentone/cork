#pragma once

#include <opencv2/core.hpp>

/**
 * Image status structure to be passed to the TCAM callback function.
 */
class ImageStatus
{
public:
    int counter = 0;
    bool busy = false;
    cv::Mat frame;
    cv::Mat resized;
};
