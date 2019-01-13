#pragma once

#include <QMainWindow>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>

#include "cork_config.hpp"
#include "cork_analyser.hpp"
#include "camera_input.hpp"
#include "camera_config.hpp"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * Input object for the camera A.
     */
    CameraInput *cameraInputA = nullptr;

    /**
     * Input object for the camera B.
     */
    CameraInput *cameraInputB = nullptr;

    /**
     * Explicit window destructor.
     */
    ~MainWindow();

    /**
     * Start the cameras capture.
     *
     * The cameras have to be initialized first.
     */
    void startCapture();

    /**
     * Stop the cameras capture.
     *
     * The cameras have to be initialized and running.
     */
    void stopCapture();

    /**
     * Create the camera capture handlers.
     */
    void createCaptureHandlers();

    /**
     * Delete the camera capture handlers objects.
     */
    void deleteCaptureHandlers();

    /**
     * Initilize the GUI element and fill values to be selected.
     */
    void initializeGUI();

    /**
     * QT window object.
     */
    Ui::MainWindow *ui;
};
