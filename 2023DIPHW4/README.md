## Install 
***Opencv-C++ in Ubuntu***
*In the directory of your project*
```
# Install minimal prerequisites (Ubuntu 18.04 as reference)
sudo apt update && sudo apt install -y cmake g++ wget unzip
# Download and unpack sources
wget -O opencv.zip https://github.com/opencv/opencv/archive/4.x.zip
unzip opencv.zip
# Create build directory
mkdir -p build && cd build
```
```
# Configure
# If you do not have administrative privileges or prefer not to install OpenCV system-wide,
cmake -DCMAKE_INSTALL_PREFIX=~/opencv_install ../opencv-4.x

# Build
cmake --build . -j $(nproc) #nproc is the number of threads you want to use

# Intall under ~/opencv_install
make install
```

***Refernce***
https://docs.opencv.org/4.x/d7/d9f/tutorial_linux_install.html
https://www.youtube.com/watch?v=uCKX_FCg9Rk

---

## Quick Start
```
g++ hw4.cpp `pkg-config --cflags --libs opencv4`
./a.out 1
./a.out 2 
```
## Reference
https://docs.opencv.org/3.4/d1/dfd/tutorial_motion_deblur_filter.html