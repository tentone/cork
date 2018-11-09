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
#apt-get update
#apt-get upgrade -y


#Remove any previous installations of x264
echo " - Remove old installations of x264."
apt-get remove -y x264 libx264-dev

#Intall dependencies
echo " - Installing depedencies."

#Build tools
apt-get install -y build-essential cmake unzip pkg-config git checkinstall yasm

#Image IO
apt-get install -y libjpeg8-dev libjasper-dev libpng12-dev

#Video IO
apt-get install -y libjpeg-dev libpng-dev libtiff-dev libxvidcore-dev libx264-dev libavcodec-dev
apt-get install -y libavformat-dev libswscale-dev libdc1394-22-dev libxine2-dev libv4l-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev
apt-get install -y libtbb-dev libfaac-dev libmp3lame-dev libtheora-dev install libvorbis-dev libopencore-amrnb-dev libopencore-amrwb-dev x264 v4l-utils
apt-get install -y libtiff5-dev

#Optional dependencies
apt-get install -y libprotobuf-dev protobuf-compiler
apt-get install -y libgoogle-glog-dev libgflags-dev
apt-get install -y libgphoto2-dev libeigen3-dev libhdf5-dev doxygen

#GKT for GUI
apt-get install -y libgtk-3-dev qt5-default

#Math Optimizations
apt-get install -y libatlas-base-dev gfortran

#Go to home
echo " - Navigate to home"
cd ~

#Python 3 support
echo " - Install Python 3 support"
apt-get install -y python3-dev
wget https://bootstrap.pypa.io/get-pip.py
python3 get-pip.py
pip install virtualenv virtualenvwrapper
rm -rf ~/get-pip.py ~/.cache/pip

#Update bashrc
echo "export WORKON_HOME=$HOME/.virtualenvs" >> ~/.bashrc
echo "export VIRTUALENVWRAPPER_PYTHON=/usr/bin/python3" >> ~/.bashrc
echo "source /usr/local/bin/virtualenvwrapper.sh" >> ~/.bashrc
source ~/.bashrc
mkvirtualenv cv -p python3

#Get the openCV code
echo " - Clone opencv repository"
git clone https://github.com/opencv/opencv.git
git clone https://github.com/opencv/opencv_contrib.git

#Build opencv
echo " - Build opencv"
cd opencv
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D INSTALL_PYTHON_EXAMPLES=ON -D INSTALL_C_EXAMPLES=ON -D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib/modules -D PYTHON_EXECUTABLE=~/.virtualenvs/cv/bin/python -D BUILD_EXAMPLES=ON ..
make -j4
make install
sudo ldconfig

echo ""