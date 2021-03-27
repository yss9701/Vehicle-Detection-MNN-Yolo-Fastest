#!/bin/sh
#SDK_PATH=/opt/pkg/xsdk/SDK/2018.3
#if [ ! -d $SDK_PATH ]; then
#  echo "no dir ${SDK_PATH}"
#  exit 1
#fi

#source ${SDK_PATH}/settings64.sh 
#export CROSS_COMPILE=arm-xilinx-linux-gnueabi
export CROSS_COMPILE=aarch64-linux-gnu
export ZYNQ_CV_BUILD=/opt/pkg/opencv_openeuler
Cur_Dir=$(pwd)
export LD_LIBRARY_PATH=$ZYNQ_CV_BUILD/lib:${LD_LIBRARY_PATH}
export C_INCLUDE_PATH=$ZYNQ_CV_BUILD/include:${C_INCLUDE_PATH}
export CPLUS_INCLUDE_PATH=$ZYNQ_CV_BUILD/include:${CPLUS_INCLUDE_PATH}
export PKG_CONFIG_PATH=$ZYNQ_CV_BUILD/lib/pkgconfig:${PKG_CONFIG_PATH}
#export PKG_CONFIG_LIBDIR=/opt/Xilinx/SDK/2015.4/gnu/arm/lin/arm-xilinx-linux-gnueabi/lib
export PKG_CONFIG_LIBDIR=/usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_aarch64-linux-gnu/aarch64-linux-gnu/lib64
OPENCV_SRC=opencv-3.4.9
TMP_DIR=zynq_src_tmp

sudo mkdir $ZYNQ_CV_BUILD
sudo chmod 777 $ZYNQ_CV_BUILD

mkdir $TMP_DIR
cp -r -f $Cur_Dir/src_pkg/* $Cur_Dir/$TMP_DIR
cd $Cur_Dir/$TMP_DIR/v4l-utils-1.12.5
./bootstrap.sh
./configure --prefix=$ZYNQ_CV_BUILD --host=${CROSS_COMPILE} --without-jpeg --with-udevdir=$ZYNQ_CV_BUILD/lib/udev
make -j8
make install

cd $Cur_Dir/$TMP_DIR/zlib-1.2.11
export CC=${CROSS_COMPILE}-gcc
./configure --prefix=$ZYNQ_CV_BUILD --shared
make
make install

cd $Cur_Dir/$TMP_DIR/jpeg-9b
./configure --prefix=$ZYNQ_CV_BUILD --host=${CROSS_COMPILE} --enable-shared
make
make install


cd $Cur_Dir/$TMP_DIR/libpng-1.6.21
./configure --prefix=$ZYNQ_CV_BUILD --host=${CROSS_COMPILE} --with-pkgconfigdir=$ZYNQ_CV_BUILD/lib/pkgconfig LDFLAGS=-L$ZYNQ_CV_BUILD/lib CFLAGS=-I$ZYNQ_CV_INSTALL/include
make
make install


cd $Cur_Dir/$TMP_DIR/x264-snapshot-20160131-2245-stable 
./configure --host=aarch64-linux --cross-prefix=${CROSS_COMPILE}- --enable-shared --prefix=$ZYNQ_CV_BUILD
make
make install

cd $Cur_Dir/$TMP_DIR/xvidcore/build/generic
./configure --prefix=$ZYNQ_CV_BUILD --host=${CROSS_COMPILE} --disable-assembly
make
make install


cd $Cur_Dir/$TMP_DIR/tiff-4.0.6
./configure --prefix=$ZYNQ_CV_BUILD --host=${CROSS_COMPILE} --enable-shared LDFLAGS=-L$ZYNQ_CV_BUILD/lib CFLAGS=-I$ZYNQ_CV_BUILD/include
make
make install


cd $Cur_Dir/$TMP_DIR/ffmpeg-2.8.6
./configure --prefix=$ZYNQ_CV_BUILD --enable-shared --disable-static --enable-gpl --enable-cross-compile --arch=arm64 --disable-stripping --target-os=linux --enable-libx264 --enable-libxvid --cross-prefix=${CROSS_COMPILE}- --enable-swscale --extra-cflags=-I$ZYNQ_CV_BUILD/include --extra-ldflags=-L$ZYNQ_CV_BUILD/lib --disable-asm
make -j8
make install

cd $Cur_Dir/$TMP_DIR/zbar-0.10
 ./configure --prefix=$ZYNQ_CV_BUILD --host=${CROSS_COMPILE} --enable-shared --without-jpeg --without-python --without-gtk --without-qt --disable-video --without-imagemagick
 LDFLAGS=-L$ZYNQ_CV_BUILD/lib CFLAGS=-I$ZYNQ_CV_BUILD/include
 make
 make install


cd $Cur_Dir/$TMP_DIR/$OPENCV_SRC
mkdir build
cd build
echo "set( CMAKE_SYSTEM_NAME Linux )" > toolchain.make
echo "set( CMAKE_SYSTEM_PROCESSOR arm )" >> toolchain.make
echo "set( CMAKE_C_COMPILER ${CROSS_COMPILE}-gcc )" >> toolchain.make
echo "set( CMAKE_CXX_COMPILER ${CROSS_COMPILE}-g++ )" >> toolchain.make
echo "set( CMAKE_INSTALL_PREFIX $ZYNQ_CV_BUILD )" >> toolchain.make
echo "set( CMAKE_FIND_ROOT_PATH $ZYNQ_CV_BUILD )" >> toolchain.make
echo "set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )" >> toolchain.make
echo "set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )" >> toolchain.make
echo "set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )" >> toolchain.make
cmake -D CMAKE_TOOLCHAIN_FILE=toolchain.make \
  -D BUILD_DOCS=OFF  \
  -D BUILD_opencv_nonfree=OFF \
  -D BUILD_JPEG=OFF  \
  -D BUILD_PACKAGE=OFF \
  -D BUILD_PERF_TESTS=OFF \
  -D BUILD_PNG=OFF  \
  -D BUILD_SHARED_LIBS=ON \
  -D BUILD_TESTS=OFF   \
  -D BUILD_TIFF=OFF  \
  -D BUILD_WITH_DEBUG_INFO=OFF  \
  -D BUILD_ZLIB=OFF   \
  -D BUILD_opencv_apps=OFF  \
  -D BUILD_opencv_calib3d=ON \
  -D BUILD_opencv_videostab=ON \
  -D BUILD_opencv_world=OFF \
  -D WITH_1394=OFF  \
  -D WITH_CUDA=OFF  \
  -D WITH_CUFFT=OFF  \
  -D WITH_EIGEN=OFF  \
  -D WITH_GIGEAPI=OFF  \
  -D WITH_GPHOTO2=OFF  \
  -D WITH_GTK=OFF  \
  -D WITH_GSTREAMER=OFF   \
  -D WITH_JASPER=OFF   \
  -D WITH_LIBV4L=OFF \
  -D WITH_MATLAB=OFF \
  -D WITH_OPENCL=OFF \
  -D WITH_OPENCLAMDBLAS=OFF \
  -D WITH_OPENCLAMDFFT=OFF \
  -D WITH_OPENEXR=OFF \
  -D WITH_PTHREADS_PF=OFF \
  -D WITH_PVAPI=OFF \
  -D WITH_V4L=ON  \
  -D WITH_JPEG=ON   \
  -D WITH_PNG=ON   \
  -D WITH_TIFF=ON   \
  -D WITH_FFMPEG=ON  \
  -D WITH_WEBP=OFF $Cur_Dir/$TMP_DIR/$OPENCV_SRC
#ccmake . 
cp -r -f $ZYNQ_CV_BUILD/lib/* $Cur_Dir/$TMP_DIR/$OPENCV_SRC/build/lib
make -j8
sudo make install
cd $Cur_Dir
