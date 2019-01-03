#-------------------------------------------------
# Cork Selector
# Project created by QtCreator
#-------------------------------------------------
QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = corkselector
TEMPLATE = app

# The following define makes your compiler emit warnings if you use any feature of Qt which has been marked as deprecated
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += c++11
QMAKE_CXXFLAGS += -pthread

LIBS += -L/usr/local/lib \
    -lopencv_calib3d \
    -lopencv_core \
    -lopencv_dnn \
    -lopencv_features2d \
    -lopencv_flann \
    -lopencv_highgui \
    -lopencv_imgcodecs \
    -lopencv_imgproc \
    -lopencv_ml \
    -lopencv_objdetect \
    -lopencv_photo \
    -lopencv_shape \
    -lopencv_stitching \
    -lopencv_superres \
    -lopencv_video \
    -lopencv_videoio \
    -lopencv_videostab \
    -lgstreamer-1.0 \
    -lgobject-2.0 \
    -lglib-2.0 \
    -ltcamprop \
    -lgirepository-1.0 \
    -lgstapp-1.0 \
    -lgstbase-1.0 \
    -lgstvideo-1.0

INCLUDEPATH += \
    /usr/local/include/opencv4 \
    /usr/include/gstreamer-1.0 \
    /usr/include/glib-2.0 \
    /usr/include/gobject-introspection-1.0 \
    /usr/lib/x86_64-linux-gnu/glib-2.0/include \
    /usr/include/orc-0.4 \
    /home/cork/tiscamera/examples/cpp/common

#PkgConfig packages to be used
#Use "pkg-config --list-all | grep -i {name}" to search packages.
#Use "pkg-config --cflags opencv" to check include dirs
#Use "pkg-config --cflags opencv" to check linker flags

#PKGCONFIG += \
#    tcam \
#    gstreamer-1.0 \
#    gstreamer-app-1.0 \
#    gstreamer-video-1.0 \
#    gobject-2.0 \
#    gmodule-2.0 \
#    gthread-2.0 \
#    glib-2.0

#Source files in the project
SOURCES += \
    /home/cork/tiscamera/examples/cpp/common/tcamcamera.cpp \
    main.cpp \
    mainwindow.cpp

#Header files of the project
HEADERS += \
    /home/cork/tiscamera/examples/cpp/common/tcamcamera.h \
    lib/cvui.h \
    camera_config.hpp \
    camera_input.hpp \
    cork.hpp \
    cork_analyser.hpp \
    cork_config.hpp \
    mainwindow.hpp \
    image_status.hpp \
    threshold.hpp \
    cvgui.hpp

#QT form files in the project
FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
