#include "Camera.h"
#include <iostream>
//构造函数=============================================================================================
Camera::Camera() : m_handle(nullptr),m_isGrabbing(false) {
    //1.初始化SDK
    MV_CC_Initialize();

    //2.枚举相机
    MV_CC_DEVICE_INFO_LIST stDeviceList = {0};
    int nRet = MV_CC_EnumDevices(MV_USB_DEVICE,&stDeviceList);
    std::cerr << "找到相机的数量" << stDeviceList.nDeviceNum << std::endl;
    if (nRet != MV_OK || stDeviceList.nDeviceNum == 0) {
        std::cerr << "找不到相机"<< std::endl;
        return;
    }

    //2.5检查设备是否可访问
    if (MV_CC_IsDeviceAccessible(stDeviceList.pDeviceInfo[0], MV_ACCESS_Exclusive) != true) {
    std::cerr << "设备不可访问，权限不足！" << std::endl;
    return;
    }

    //3.创造句柄
    nRet = MV_CC_CreateHandle(&m_handle,stDeviceList.pDeviceInfo[0]);
    if (nRet != MV_OK) {
        std::cerr << "创建句柄失败" << std::endl;
        m_handle = nullptr;
        
    }
    
}
//析构函数=============================================================================================
Camera::~Camera() {
    close();//对象销毁时，自动关闭相机
}

//打开相机=============================================================================================
bool Camera::open() {
    if (!m_handle) return false;

    //打开设备
    int nRet = MV_CC_OpenDevice(m_handle);
    std::cerr << "OpenDevice 返回码： 0x"<< std::hex << nRet << std::endl;
    if (nRet != MV_OK) {
        std::cerr << "打开设备失败" << std::endl;
        return false;
    }
    // 设置曝光时间（单位微秒，10000 = 10毫秒）
    MVCC_FLOATVALUE stExposure = {0};
    nRet = MV_CC_SetFloatValue(m_handle, "ExposureTime", 10000.0f);
    if (nRet != MV_OK) {
        std::cerr << "设置曝光失败，错误码: 0x" << std::hex << nRet << std::endl;
    }

    // 设置增益（单位 dB，数值越大越亮，但噪点也越多）
    nRet = MV_CC_SetFloatValue(m_handle, "Gain", 10.0f);
    if (nRet != MV_OK) {
        std::cerr << "设置增益失败，错误码: 0x" << std::hex << nRet << std::endl;
    }

    //开始取流
    nRet = MV_CC_StartGrabbing(m_handle);
    std::cerr << "StartGrabbing 返回码:0x" << std::hex << nRet << std::endl;
    if (nRet != MV_OK) {
        std::cerr << "开始取流失败" << std::endl;
        return false;
    }
    m_isGrabbing = true;
    return true;
}

//抓取一帧图像===========================================================================================
bool Camera::getFrame(cv::Mat&outImg) {
    if (!m_isGrabbing) return false;

    //准备一块内存来接受图像数据（分配缓冲区）
    unsigned char* pData = new unsigned char[1440 * 1080 *3];
    MV_FRAME_OUT_INFO_EX stImageInfo = {0};

    //抓取一帧（超时时间1000ms)
    int nRet = MV_CC_GetOneFrameTimeout(m_handle,pData, 1440* 1080 *3,&stImageInfo,1000);
    std::cerr << "GetFrame 返回码:0x" << std::hex << nRet << std::endl;
    if (nRet != MV_OK) {
        std::cerr << "图像获取失败，错误码:0x" << std::hex << nRet << std::endl;
        delete[] pData;
        return false;
    }
     // 3. 申请转换后的图像内存（BGR8）
    unsigned char* pConvertData = new unsigned char[1440 * 1080 * 3];
    MV_CC_PIXEL_CONVERT_PARAM stConvertParam = {0};
    stConvertParam.nWidth = stImageInfo.nWidth;
    stConvertParam.nHeight = stImageInfo.nHeight;
    stConvertParam.pSrcData = pData;
    stConvertParam.nSrcDataLen = stImageInfo.nFrameLen;
    stConvertParam.enSrcPixelType = stImageInfo.enPixelType;
    stConvertParam.enDstPixelType = PixelType_Gvsp_BGR8_Packed;
    stConvertParam.pDstBuffer = pConvertData;
    stConvertParam.nDstBufferSize = 1440 * 1080 * 3;

    // 4. 执行格式转换
    nRet = MV_CC_ConvertPixelType(m_handle, &stConvertParam);
    if (nRet != MV_OK) {
        std::cerr << "像素格式转换失败，错误码: 0x" << std::hex << nRet << std::endl;
        delete[] pData;
        delete[] pConvertData;
        return false;
    }

    // 5. 把转换后的数据变成 OpenCV 的 Mat
    cv::Mat rawImg(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC3, pConvertData);
    outImg = rawImg.clone();

    delete[] pData;
    delete[] pConvertData;
    return true;

    
}

//关闭相机=============================================================================================
void Camera::close() {
    if (m_handle) {
        if (m_isGrabbing) {
            MV_CC_StopGrabbing(m_handle);
                m_isGrabbing = false;
        }
        MV_CC_CloseDevice(m_handle);

        MV_CC_DestroyHandle(m_handle);
            m_handle = nullptr;
    }
    MV_CC_Finalize();
}




