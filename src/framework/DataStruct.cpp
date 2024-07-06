#include "DataStruct.h"
#include "../utils/Logger.h"

OcrData::OcrData()
{
    for (int i = 0; i < 4; i++) {
        this->points[i] = cv::Point2i(0, 0);
    }
    this->conf = 0;
}

OcrData::~OcrData()
{

}

void OcrData::SetPoints(std::vector<stPoint> vPoints)
{
    if (vPoints.size() != 4) {
        LOGE("Wrong points size. {}", vPoints.size());
        return;
    }
    for (int i = 0; i < vPoints.size(); i++) {
        this->points[i].x = vPoints[i].x;
        this->points[i].y = vPoints[i].y;
    }
}

void OcrData::SetPoints(std::vector<cv::Point> vPoints)
{
    if (vPoints.size() != 4) {
        LOGE("Wrong points size. {}", vPoints.size());
        return;
    }
    for (int i = 0; i < vPoints.size(); i++) {
        this->points[i].x = vPoints[i].x;
        this->points[i].y = vPoints[i].y;
    }
}

cv::Mat OcrData::GetCropImage(const cv::Mat& srcimage)
{
    std::vector<std::vector<int>> box = {
        {
            {(int)points[0].x, (int)points[0].y}, 
            {(int)points[1].x, (int)points[1].y},
            {(int)points[2].x, (int)points[2].y},
            {(int)points[3].x, (int)points[3].y},
        }
    };

    std::vector<int> x_vec{ box[0][0], box[1][0], box[2][0], box[3][0] };
    std::vector<int> y_vec{ box[0][1], box[1][1], box[2][1], box[3][1] };
    int x_min = *std::min_element(x_vec.begin(), x_vec.end());
    int x_max = *std::max_element(x_vec.begin(), x_vec.end());

    int y_min = *std::min_element(y_vec.begin(), y_vec.end());
    int y_max = *std::max_element(y_vec.begin(), y_vec.end());
    if (x_max - x_min < 3 || y_max - y_min < 3)
        return cv::Mat();
    
    cv::Mat image;
    srcimage.copyTo(image);
    std::vector<std::vector<int>> points = box;


    int x_collect[4] = { box[0][0], box[1][0], box[2][0], box[3][0] };
    int y_collect[4] = { box[0][1], box[1][1], box[2][1], box[3][1] };
    int left         = int(*std::min_element(x_collect, x_collect + 4));
    int right        = int(*std::max_element(x_collect, x_collect + 4));
    int top          = int(*std::min_element(y_collect, y_collect + 4));
    int bottom       = int(*std::max_element(y_collect, y_collect + 4));

    cv::Mat img_crop;
    image(cv::Rect(left, top, right - left, bottom - top)).copyTo(img_crop);

    for (int i = 0; i < points.size(); i++) {
        points[i][0] -= left;
        points[i][1] -= top;
    }

    int img_crop_width  = int(sqrt(pow(points[0][0] - points[1][0], 2) + pow(points[0][1] - points[1][1], 2)));
    int img_crop_height = int(sqrt(pow(points[0][0] - points[3][0], 2) + pow(points[0][1] - points[3][1], 2)));

    cv::Point2f pts_std[4];
    pts_std[0] = cv::Point2f(0., 0.);
    pts_std[1] = cv::Point2f(img_crop_width, 0.);
    pts_std[2] = cv::Point2f(img_crop_width, img_crop_height);
    pts_std[3] = cv::Point2f(0.f, img_crop_height);

    cv::Point2f pointsf[4];
    pointsf[0] = cv::Point2f(points[0][0], points[0][1]);
    pointsf[1] = cv::Point2f(points[1][0], points[1][1]);
    pointsf[2] = cv::Point2f(points[2][0], points[2][1]);
    pointsf[3] = cv::Point2f(points[3][0], points[3][1]);

    cv::Mat M = cv::getPerspectiveTransform(pointsf, pts_std);

    cv::Mat dst_img;
    cv::warpPerspective(img_crop, dst_img, M, cv::Size(img_crop_width, img_crop_height), cv::BORDER_REPLICATE);

    if (float(dst_img.rows) >= float(dst_img.cols) * 1.5) {
        cv::Mat srcCopy = cv::Mat(dst_img.rows, dst_img.cols, dst_img.depth());
        cv::transpose(dst_img, srcCopy);
        cv::flip(srcCopy, srcCopy, 0);
        return srcCopy;
    }
    else {
        return dst_img;
    }
}

json OcrData::ToJsonResult(int offset_x, int offset_y)
{
    json result = {
       {"label", labelName},
       {"shapeType", "polygon"},
       {"points", json::array()},
       {"result", {{"text", text}, {"confidence", conf}}},
    };

    for (int i = 0; i < 4; i++) {
        result["points"].push_back({points[i].x + offset_x, points[i].y + offset_y});
    }
    return result;
}

cv::Point2i OcrData::GetCenter()
{
    int cent_x = (points[0].x + points[1].x + points[2].x + points[3].x) / 4 + 0.5;
    int cent_y = (points[0].y + points[1].y + points[2].y + points[3].y) / 4 + 0.5;
    return cv::Point2i(cent_x, cent_y);
}

void OcrData::DrawResult(const cv::Mat& image, bool bDrawText)
{
    cv::line(image, cv::Point2i(points[0].x, points[0].y), cv::Point2i(points[1].x, points[1].y), cv::Scalar(200,0,0), 2);
    cv::line(image, cv::Point2i(points[1].x, points[1].y), cv::Point2i(points[2].x, points[2].y), cv::Scalar(0,0,200), 2);
    cv::line(image, cv::Point2i(points[2].x, points[2].y), cv::Point2i(points[3].x, points[3].y), cv::Scalar(0,200,0), 2);
    cv::line(image, cv::Point2i(points[3].x, points[3].y), cv::Point2i(points[0].x, points[0].y), cv::Scalar(0,200,0), 2);

    if (bDrawText) {
        cv::Point2i centPt = GetCenter();
        centPt.y += 20;
        cv::putText(image, text, centPt, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0,0,200));
    }
}

int OcrData::GetWidth()
{
    return sqrt(pow(points[0].x - points[1].x, 2) + pow(points[0].y - points[1].y, 2));
}

int OcrData::GetHeight()
{
    return sqrt(pow(points[2].x - points[1].x, 2) + pow(points[2].y - points[1].y, 2));
}

float OcrData::GetAspectRatio()
{
    int len1 = GetWidth();
    int len2 = GetHeight();
    return max(len1, len2) / min(len1, len2);
}