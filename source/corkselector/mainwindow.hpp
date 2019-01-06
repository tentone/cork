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

    ~MainWindow();

    /**
     * QT window object.
     */
    Ui::MainWindow *ui;


    double defectA = -1.0;
    double defectB = -1.0;

    CorkConfig configA;
    CorkConfig configB;
};
