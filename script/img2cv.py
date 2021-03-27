# encoding:utf-8
import cv2
import os

# 图片路径
#im_dir = 'C:/Users/Administrator/Desktop/img_test/'
im_dir = 'E:/img_5/'
# 输出视频路径
video_dir = '2.avi'
# 帧率
fps = 15
# 图片数
num = 5199
# 图片尺寸
img_size = (1920, 1080)

# fourcc = cv2.cv.CV_FOURCC('M','J','P','G')#opencv2.4
fourcc = cv2.VideoWriter_fourcc('M', 'J', 'P', 'G')  # opencv3.0
videoWriter = cv2.VideoWriter(video_dir, fourcc, fps, img_size)

for i in range(1, num):
    #im_name = os.path.join(im_dir, 'MVI_20011__img'+str(i).zfill(5)+'.jpg')
    im_name = os.path.join(im_dir, str(i).zfill(5) + '.jpg')
    frame = cv2.imread(im_name)
    videoWriter.write(frame)
    print(im_name)

videoWriter.release()
print('finish')