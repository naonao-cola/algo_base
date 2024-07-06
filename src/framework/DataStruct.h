#pragma once

#include "Defines.h"
#include <AIRuntimeInterface.h>


class OcrData
{
public:
    OcrData();
    ~OcrData();

    void SetPoints(std::vector<stPoint> vPoints);
    void SetPoints(std::vector<cv::Point> vPoints);
    cv::Point2i GetCenter();
    int GetWidth();
    int GetHeight();
    float GetAspectRatio();
    json ToJsonResult(int offset_x = 0, int offset_y = 0);

    cv::Mat GetCropImage(const cv::Mat& image);
    void DrawResult(const cv::Mat& image, bool bDrawText);

    
    cv::Point2i points[4];      // 推理图片上的文字框4个点
    std::string labelName;
    std::string text;           // 识别结果 utf8
    float conf = 0;             // 置信度
    int state = 0;
};

using OcrDataPtr = std::shared_ptr<OcrData>;