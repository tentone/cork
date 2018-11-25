# Cork
 - Cork stopper analysis and comparison vision system.
 - Developed using a ImagingSource DFK23UX236 camera.

## Features
 - Automatic cork detection (circle).
 - Automatic threshold ajust using otsu (per cork).
 - Per pixel defect measurement.
 
## TODO
 - Background removal
 - Color filtering

## Build
 - Install Cmake 3.10+
 - Run the following commands

```
cmake .
./build.sh
```

## Hardware
 - USB Camera
 - EtherCAT I/O Module

## Dependencies
 - OpenCV 4.0 RC
   - https://opencv.org/opencv-4-0-0-rc.html
 - CVUI
   - https://github.com/Dovyski/cvui
 - SOEM (Simple OpenSrouce EtherCat Master)
   - https://github.com/OpenEtherCATsociety/SOEM
 - Imaging Source Camera software
   - https://github.com/TheImagingSource/tiscamera