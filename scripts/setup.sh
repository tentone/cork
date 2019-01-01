#!/bin/bash

echo "------------------------------------"
echo "           Cork detector"
echo "      Build enviroment setup"
echo "------------------------------------"

#Check root
echo " - Checking root permissions."
if (( $EUID > 0 )); then
	echo "------------------------------------"
	echo "        Failed (Run as root)"
	echo "------------------------------------"
	exit
fi

#Update aptitude repository
echo " - Update packages."
#apt-get -qq update
#apt-get -qq upgrade -y

#Remove old versions
#echo " - Remove old installations of x264."
#apt-get -qq remove -y x264 libx264-dev

#Install ROS
echo " - Installing ROS"
#sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'
#apt-key adv --keyserver hkp://ha.pool.sks-keyservers.net:80 --recv-key 421C365BD9FF1F717815A3895523BAEEB01FA116
#apt-get -qq update
#apt-get install ros-melodic-desktop-full

echo " - Installing NodeJS"
#apt-get -qq install nodejs npm

#Intall dependencies
echo " - Installing depedencies."

#Build tools
#apt-get -qq install -y build-essential cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
#apt-get -qq install -y python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev

#Image IO
#apt-get -qq install -y libjpeg8-dev libjasper-dev libpng12-dev

#Video IO
#apt-get -qq install -y libjpeg-dev libpng-dev libtiff-dev libxvidcore-dev libx264-dev libavcodec-dev
#apt-get -qq install -y libavformat-dev libswscale-dev libdc1394-22-dev libxine2-dev libv4l-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev
#apt-get -qq install -y libtbb-dev libfaac-dev libmp3lame-dev libtheora-dev install libvorbis-dev libopencore-amrnb-dev libopencore-amrwb-dev x264 v4l-utils
#apt-get -qq install -y libtiff5-dev

#GKT for GUI
#apt-get -qq install -y libgtk-3-dev qt5-default

#Math Optimizations
#apt-get -qq install -y libatlas-base-dev gfortran

#Go to home
echo " - Navigate to home"
cd ~

#Python 3 support
#echo " - Install Python 3 support"
#apt-get -qq install -y python3-dev python3-pip
#pip3 install virtualenv virtualenvwrapper

#Update bashrc
#echo "export WORKON_HOME=$HOME/.virtualenvs" >> ~/.bashrc
#echo "export VIRTUALENVWRAPPER_PYTHON=/usr/bin/python3" >> ~/.bashrc
#echo "source /usr/local/bin/virtualenvwrapper.sh" >> ~/.bashrc
#source ~/.bashrc
#mkvirtualenv cv -p python3

#Get the openCV code
echo " - Clone opencv repository"
git clone https://github.com/opencv/opencv.git
git clone https://github.com/opencv/opencv_contrib.git

#Build opencv
echo " - Build opencv"
cd opencv
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_V4L=ON -D INSTALL_PYTHON_EXAMPLES=O -d WITH_OPENGL=ON -D WITH_QT=ON -D WITH_OPENCL=ON -D INSTALL_C_EXAMPLES=ON -D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib/modules -D PYTHON_EXECUTABLE=~/.virtualenvs/cv/bin/python -D BUILD_EXAMPLES=ON ..
make -j4
make install
