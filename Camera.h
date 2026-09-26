#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>
#include <MvCameraControl.h>//这是海康SDK的头文件

class Camera {
    public:
        Camera();
        //构造函数：初始化，找相机，创建句柄
        ~Camera();
        //析构函数：自动释放资源
        bool open();
        //打开相机开始取流
        bool getFrame(cv::Mat&outImg);
        //抓取一帧，转成Opencv Mat
        void close();
        //关闭相机
    private:
        void* m_handle;
        //相机句柄，私有
        bool m_isGrabbing;
        //标记是否正在取流

};

#endif



