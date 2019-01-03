#-------------------------------------------------
#
# Project created by QtCreator 2019-01-01T16:22:59
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = corkselector
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

QMAKE_CXXFLAGS += -pthread #-std=c++0x

INCLUDEPATH += \
    /usr/local/include/opencv4 \
    /usr/include/gstreamer-1.0 \
    /usr/include/gobject-introspection-1.0 \
    /usr/include/glib-2.0 \
    /usr/lib/x86_64-linux-gnu/glib-2.0/include \
    /home/cork/tiscamera/examples/cpp/common

LIBS += \
    -L/usr/local/lib \
    -L/usr/lib/glib-2.0/include \
    -pthread


#Use "pkg-config --list-all | grep -i {name}" to search packages.
PKGCONFIG += \
    opencv \
    tcam \
    gstreamer-1.0 \
    gstreamer-app-1.0 \
    gstreamer-video-1.0

SOURCES += \
    main.cpp \
    mainwindow.cpp
#    /home/cork/tiscamera/examples/cpp/common/tcamcamera.cpp

HEADERS += \
    camera_config.hpp \
    camera_input.hpp \
    cork.hpp \
    cork_analyser.hpp \
    cork_config.hpp \
    mainwindow.hpp \
    image_status.hpp \
    threshold.hpp \
    lib/cvui.h \
    cvgui.hpp

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
