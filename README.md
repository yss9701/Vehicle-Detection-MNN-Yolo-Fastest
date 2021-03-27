# Vehicle-Detection-MNN-Yolo-Fastest

#### 介绍
本项目为“openEuler”高校开发者大赛--城市关键路径机动车流量智能化监控项目。本项目利用RK3399与HopeEdge OS，通过深度学习方法对车辆进行实时检测，利用MNN深度学习推理框架与OpenCV实现车辆检测模型的移动端部署，利用检测结果，根据检测目标的欧氏距离判断前后帧检测车辆是否为同一辆车，对车辆进行计数。

![2](https://github.com/yss9701/Vehicle-Detection-MNN-Yolo-Fastest/raw/main/img/1.jpg)

#### 软件架构

目录说明：

./lib为部署于开发板的动态链接库，需放置于开发板/usr/lib目录下，或放置于其他目录下并[指定搜索路径](https://blog.csdn.net/zong596568821xp/article/details/90297360)。

 ./src为Qt工程，其中.pro文件需根据主机实际路径更改。

./script为脚本，包括编译脚本、视频处理脚本等。

./model为MNN转换后的模型与量化后的模型及MNN编译产物。

./demo为车辆检测结果展示，视频左上角展示出车辆计数与检测帧率。

硬件环境：Firefly-RK3399开发板

开发环境：Ubuntu16.04，Qt 5.12.9，gcc-linaro-4.9.4-2017.01-x86_64_aarch64-linux-gnu，MobaXterm 11.0

模型训练：TensorFlow-gpu 2.2

模型部署：MNN 1.0.2，OpenCV 3.4.9，protobuf 3.12.0，gcc/g++ 4.9


#### 安装教程

1、利用TensorFlow2.2，搭建[Yolo-Fastest](https://github.com/dog-qiuqiu/Yolo-Fastest)模型。将UA-DETRAC数据集处理为VOC格式，对模型进行训练，后对检测头进行剪枝，压缩模型，重训练恢复精度，将模型结构保存与参数分开保存，并进行模型固化，产生pb文件，具体过程可参考[此项目](https://github.com/yss9701/Ultra96-Yolov4-tiny-and-Yolo-Fastest)。

2、在Ubuntu16.04中安装[交叉编译工具链](https://releases.linaro.org/components/toolchain/binaries/4.9-2017.01/aarch64-linux-gnu/), [安装qt](https://blog.csdn.net/hl1796/article/details/90205218), 安装GCC工具链与[protobuf](https://blog.csdn.net/qq_45835827/article/details/105490115)。

3、在主机端源码[编译MNN](https://www.yuque.com/mnn/cn)，编译产物为模型转换与模型量化工具。

4、在主机端源码交叉编译MNN，编译产物为MNN动态链接库。

5、在主机端源码交叉编译OpenCV，编译产物为OpenCV动态链接库，编译脚本为./script/build_3399.sh。

6、利用./script/pb.py进行模型输入尺寸固化，利用MNN进行[模型转换与量化](https://www.yuque.com/mnn/cn/model_convert)，将量化模型通过MobaXterm传送到开发板执行目录。

6、利用Qt Creator编程，交叉编译产生可执行文件，利用MobaXterm传送到开发板。

7、利用./script/video2img.py进行视频处理，将处理过的图像传送到开发板执行目录，执行可执行文件，得到检测与计数结果。

8、利用./script/img2cv.py对检测过的图像处理，合成视频。

#### 使用说明

1、烧写[HopeEdge OS镜像](http://download.hopeinfra.com/HopeEdge/Images/FireFly-RK3399/)至开发板。

2、通过MobaXterm利用SSH登录开发板。

3、准备容量8G以上的U盘，挂载于开发板/mnt目录。

4、将Qt编译产生的可执行文件放至开发板/mnt目录。

5、利用脚本处理视频为图像帧。

6、将处理过的视频帧放至开发板/mnt/img3目录。

7、利用脚本将处理过的图像合成视频。

#### **参考**

[MNN](https://github.com/alibaba/MNN)

[MNN-Yolo-Fastest](https://github.com/geekzhu001/Yolo-Fastest-MNN)

