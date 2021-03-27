TEMPLATE = app
CONFIG += console c++11 -mfloat-abi=hard
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += /home/yss/mnn_project/MNN/include \
               /opt/pkg/opencv_openeuler/include

LIBS += /opt/pkg/opencv_openeuler/lib/lib*.so* \
        /home/yss/mnn_project/MNN/build_arm1/libMNN.so









SOURCES += \
        main.cpp
