#pragma once

#include <string>

#include <opencv2/core.hpp>

#define CVUI_DISABLE_COMPILATION_NOTICES
#define CVUI_IMPLEMENTATION
#include "lib/cvui.h"

#include "cork_config.hpp"

/**
 * Represents a detected cork.
 */
class cvgui
{
public:

    /**
     * Create a GUI trackbar.
     */
    static void trackbar(const cv::String& theText, int theWidth, double *theValue, double theMin, double theMax, int theSegments = 1, const char *theLabelFormat = "%.1Lf", unsigned int theOptions = 0, double theDiscreteStep = 1)
    {
        cvui::beginRow();
        cvui::text(theText);
        cvui::beginColumn();
        cvui::space(-19);
        cvui::trackbar(theWidth, theValue, theMin, theMax, theSegments, theLabelFormat, theOptions | cvui::TRACKBAR_HIDE_SEGMENT_LABELS | cvui::TRACKBAR_HIDE_MIN_MAX_LABELS, theDiscreteStep);
        cvui::endColumn();
        cvui::endRow();
    }

    /**
     * Create a GUI trackbar.
     */
    static void trackbar(const cv::String& theText, int theWidth, int *theValue, int theMin, int theMax, int theSegments = 1, const char *theLabelFormat = "%.Lf", unsigned int theOptions = 0, int theDiscreteStep = 1)
    {
        cvui::beginRow();
        cvui::text(theText);
        cvui::beginColumn();
        cvui::space(-19);
        cvui::trackbar(theWidth, theValue, theMin, theMax, theSegments, theLabelFormat, theOptions | cvui::TRACKBAR_HIDE_SEGMENT_LABELS | cvui::TRACKBAR_HIDE_MIN_MAX_LABELS, theDiscreteStep);
        cvui::endColumn();
        cvui::endRow();
    }

    /**
     * Draw the configuration editor to a mat.
     */
    static void drawConfigEditor(std::string title, cv::Mat image, CorkConfig *config)
    {
        cvui::beginColumn(image, 10, 10);
        cvui::beginRow();
        cvui::checkbox("Blur Global", &config->blurGlobal);
        cvui::space(12);
        cvui::checkbox("Blur Mask", &config->blurMask);
        cvui::endRow();

        if(config->blurGlobal)
        {
            cvui::space(12);
            cvgui::trackbar("Blur Global Kernel", 200, &config->blurGlobalKSize, 3, 101, 2);
            if(config->blurGlobalKSize % 2 == 0)
            {
                config->blurGlobalKSize++;
            }
        }

        if(config->blurMask)
        {
            cvui::space(12);
            cvgui::trackbar("Blur Mask Kernel", 200, &config->blurMaskKSize, 3, 101, 2);
            if(config->blurMaskKSize % 2 == 0)
            {
                config->blurMaskKSize++;
            }
        }

        cvui::space(12);
        cvui::text("Threshold");

        cvui::space(12);
        cvui::beginRow();
        cvui::checkbox("Automatic", &config->automaticThresh);
        cvui::checkbox("SemiAuto", &config->semiAutoThresh);

        if(config->automaticThresh)
        {
            cvui::space(12);
            if(cvui::checkbox("Otsu", &config->automaticUseOtsuThresh))
            {
                config->automaticUseHistogramThresh = false;
            }
            cvui::space(12);
            if(cvui::checkbox("Histogram", &config->automaticUseHistogramThresh))
            {
                config->automaticUseOtsuThresh = false;
            }
        }
        cvui::endRow();

        if(!config->automaticThresh || config->semiAutoThresh)
        {
            cvui::space(12);
            cvgui::trackbar("Threshold", 200, &config->thresholdValue, 10, 150, 1);
        }

        if(config->semiAutoThresh)
        {
            cvui::space(12);
            cvgui::trackbar("Tolerance", 200, &config->semiAutoThreshTolerance, 0.0, 1.0, 0.01);
        }

        if(config->automaticThresh && config->automaticUseHistogramThresh)
        {
            cvui::space(12);
            cvgui::trackbar("Min-diff", 200, &config->histThreshMinDiff, 5, 100, 1);
            cvui::space(12);
            cvgui::trackbar("Neighborhood", 200, &config->histThreshNeighborhood, 3, 100, 1);
            cvui::space(12);
            cvgui::trackbar("Neigh. Filter", 200, &config->histThreshColorFilter, 1, 100, 1);
            cvui::space(12);
            cvgui::trackbar("Balance", 200, &config->histThreshBalance, 0, 1, 1);
        }

        cvui::space(12);
        cvgui::trackbar("Skirt", 200, &config->outsizeSkirt, 0, 20, 1);

        cvui::space(12);
        cvui::text("Circle");
        cvui::space(12);
        cvgui::trackbar("Spacing", 200, &config->minSpacing, 1, 600, 1);
        cvui::space(12);
        cvgui::trackbar("Canny low", 200, &config->lowCannyThresh, 1, 200, 1);
        cvui::space(12);
        cvgui::trackbar("Canny high", 200, &config->highCannyThresh, 1, 100, 1);
        cvui::space(12);
        cvgui::trackbar("Min size", 200, &config->minSize, 0, 200, 1);
        cvui::space(12);
        cvgui::trackbar("Max size", 200, &config->maxSize, 0, 700, 1);
        cvui::endColumn();

        cvui::update();
        cvui::imshow(title, image);
    }
};
