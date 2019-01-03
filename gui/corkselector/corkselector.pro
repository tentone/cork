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

INCLUDEPATH += \
    /usr/local/include/opencv4 \
    /usr/include/gstreamer-1.0 \
    /usr/include/glib-2.0 \
    /home/cork/tiscamera/examples/cpp/common
#    /usr/lib/x86_64-linux-gnu/glib-2.0/include
#    /usr/include/gobject-introspection-1.0 \

#LIBS += \
#    -pthread
#    -L/usr/local/lib \
#    -L/usr/lib/glib-2.0/include \

#PkgConfig packages to be used
#Use "pkg-config --list-all | grep -i {name}" to search packages.
PKGCONFIG += \
    opencv \
    tcam \
    gstreamer-1.0 \
    gstreamer-app-1.0 \
    gstreamer-video-1.0 \
    gobject-2.0 \
    gmodule-2.0 \
    gthread-2.0 \
    glib-2.0

#Source files in the project
SOURCES += \
    main.cpp \
    mainwindow.cpp
#    /home/cork/tiscamera/examples/cpp/common/tcamcamera.cpp

#Header files of the project
HEADERS += \
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
