# Cork Stopper CV
 - Cork stopper analysis and comparison vision system, capable of detecting defects on the booth sides of a cork stopper.

 - Developed using a Imaging Source DFK23UX236 camera and Linux.

 - System was triggered using an external machine that pushed the cork trough the camera FOV.

 - Uses a structured RGB light to measure shadows from 3 sides of the cork stopper.

   

### Features
 - Automatic cork detection (based color and circular morphology).
 - Automatic detection parameters adjustment to accommodate different cork sizes in real-time.
 - Per-pixel shadow based defect measurement for a precise area of defect measurement.
 - Color channels are analyzed individually, and the defect detection is based on the RGB shadows.
    - Darker cork usually still reflects RED light, using RED, BLUE and GREEN light from different angles we can check if the shadow produced by the defect (hole) is common to all color channels.
    - Typical system cannot distinguish a hole (defect) from a darker zone (not a defect).
 - In the images bellow we can observe some results where its possible to observe darker zones in the cork not being classified as a defect.

<img src="https://raw.githubusercontent.com/tentone/cork/master/readme/b.png" width="250"><img src="https://raw.githubusercontent.com/tentone/cork/master/readme/c.png" width="250">



### Configuration

- The project can use up to two individual cameras and has support for IP, USB, File image sources.
- The QT interface only support Portuguese language.
- The configuration tab allows to customize the camera parameters used.

<img src="https://raw.githubusercontent.com/tentone/cork/master/readme/gui/a.png" width="250"><img src="https://raw.githubusercontent.com/tentone/cork/master/readme/gui/c.png" width="250">



### Build and Run w/ QMake

 - Setup QT5 and compile OpenCV with QT support
 - Install QT Creator IDE
 - Change the "tiscamera" path under the corkselector.pro file
 - If the glibconfig.h is missing try the following command
 - sudo cp /usr/lib/x86_64-linux-gnu/glib-2.0/include/glibconfig.h /usr/include/glib-2.0/glibconfig.h

```
cmake .
./build.sh
```



### Code Style

- `camelCase` for all names (except constants).
- Beginning with a capital if it's a class, `lowercase` if not.
- Constants in `ALL_CAPS` with underscore separating works.
- Functions can be written the same as variables, `camelCase` (you can tell it's a function because it requires () after it).



### Dependencies
 - OpenCV 4.0 RC (https://opencv.org/opencv-4-0-0-rc.html)
 - CVUI (https://github.com/Dovyski/cvui)
 - Imaging Source Camera software (https://github.com/TheImagingSource/tiscamera)
 - QT (https://www.qt.io/)
