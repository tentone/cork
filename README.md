# Cork
 - Cork stopper analysis and comparison vision system.
 - Developed using a ImagingSource DFK23UX236 camera.

## Features
 - Automatic cork detection (circle).
 - Automatic threshold ajust using otsu (per cork).
 - Per pixel defect measurement.
 
## To do
 - Background removal
 - Color filtering

## Code Style
 - camelCase for all names (except constants).
 - Beginning with a capital if it's a class, lowercase if not.
 - Constants in ALL_CAPS with underscore separating works.
 - Functions can be written the same as variables, camelCase (you can tell it's a function because it requires () after it).

## Build
 - Install Cmake 3.10+
 - Run the following commands

```
cmake .
./build.sh
```

## Hardware
 - USB Camera

## Dependencies
 - OpenCV 4.0 RC
   - https://opencv.org/opencv-4-0-0-rc.html
 - CVUI
   - https://github.com/Dovyski/cvui
 - Imaging Source Camera software
   - https://github.com/TheImagingSource/tiscamera