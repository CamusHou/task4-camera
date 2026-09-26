#include "Camera.h"
#include <iostream>

int main() {
    Camera cam;

    if (!cam.open()) {
        std::cerr << "相机打开失败，程序退出" << std::endl;
        return -1;
    }

    std::cout << "相机已打开，按任意键抓取一帧图像..." <<std::endl;
    std::cin.get();

    cv::Mat frame;
    if (cam.getFrame(frame)) {
        cv::imshow("Camera Frame",frame);
        cv::waitKey(1);
        cv::waitKey(0);
    }
    else {
        std::cerr <<"抓取图像失败"<< std::endl;
    }

    return 0;
}

