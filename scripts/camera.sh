#!/bin/bash

echo "------------------------------------"
echo "       Imaging Source Camera"
echo "       Build enviroment setup"
echo "------------------------------------"

#Check root
echo " - Checking root permissions."
if (( $EUID > 0 )); then
	echo "------------------------------------"
	echo "        Failed (Run as root)"
	echo "------------------------------------"
	exit
fi

#Go to home
echo " - Navigate to home"
cd ~

#Build dependencies
echo " - Get build dependencies"
sudo apt-get install -y git g++ cmake pkg-config libudev-dev libudev1 libtinyxml-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libglib2.0-dev libgirepository1.0-dev libusb-1.0-0-dev libzip-dev uvcdynctrl python-setuptools libxml2-dev libpcap-dev libaudit-dev libnotify-dev autoconf intltool gtk-doc-tools python3-setuptools

#Runtime dependencies
echo " - Get runtime dependencies"
sudo apt-get install -y gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly libxml2 libpcap0.8 libaudit1 libnotify4 python3-pyqt5 python3-gi

#Clone repository
echo " - Clone tiscamera repository"
git clone --recursive https://github.com/TheImagingSource/tiscamera.git
cd tiscamera
mkdir build
cd build

#Build opencv
cmake options
#-DBUILD_ARAVIS=<ON/OFF>
#Build tiscamera with support for aravis devices.
#-DBUILD_GST_1_0=<ON/OFF>
#Build gstreamer 1.0 plugins.
#-DBUILD_TOOLS=<ON/OFF>
#Build additional tools for camera interaction.
#-DBUILD_V4L2=<ON/OFF>
#Build tiscamera with suppoort for v4l2 devices.
#-DBUILD_LIBUSB=<ON/OFF>
#Build tiscamera with suppoort for v4l2 devices.
#-DCMAKE_INSTALL_PREFIX
#Installation target prefix (defaults to /usr/local)

echo " - Build tiscamera"
cmake -DBUILD_ARAVIS=ON -DBUILD_GST_1_0=ON -DBUILD_TOOLS=ON -DBUILD_V4L2=ON -DCMAKE_INSTALL_PREFIX=/usr ..
make
sudo make install

#Without ARAVIS
#cmake -DBUILD_ARAVIS=OFF -DBUILD_GST_1_0=ON -DBUILD_TOOLS=ON -DBUILD_V4L2=ON -DCMAKE_INSTALL_PREFIX=/usr ..

