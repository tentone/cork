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

#Intall dependencies
echo " - Installing depedencies."

#Update aptitude repository
apt-get update
apt-get upgrade

#Build tools
apt-get install -y build-essential cmake unzip pkg-config git

#Video IO
apt-get install -y libjpeg-dev libpng-dev libtiff-dev
apt-get install -y libxvidcore-dev libx264-dev

#GKT for GUI
apt-get install -y libgtk-3-dev

#Math Optimizations
apt-get install -y libatlas-base-dev gfortran

#Python 3 support
apt-get install -y python3-dev

#Get the openCV code
echo " - Clone opencv repository"
git clone https://github.com/opencv/opencv.git
git clone https://github.com/opencv/opencv_contrib.git

