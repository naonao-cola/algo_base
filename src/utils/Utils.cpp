#include "Utils.h"
#include "Logger.h"

json Utils::ReadJsonFile(std::string filepath)
{
    std::ifstream conf_i(filepath);
    std::stringstream ss_config;
    ss_config << conf_i.rdbuf();
    json jsonObj = json::parse(ss_config.str());
    return std::move(jsonObj);
}


json Utils::ParseJsonText(const char* json_text, bool is_ansi)
{
    std::string utf8_text = is_ansi ? StringConvert::AnsiToUtf8(std::string(json_text)) : std::string(json_text);
    json jsonObj = json::parse(utf8_text);
    return std::move(jsonObj);
}

std::string Utils::DumpJson(json jsonObj, bool toAnsi)
{
    return toAnsi ? StringConvert::Utf8ToAnsi(jsonObj.dump()) : jsonObj.dump();
}

cv::Mat Utils::GenCvImage(unsigned char* img_data, const json& img_info)
{
    int img_w = Utils::GetProperty(img_info, "img_w", 0);
    int img_h = Utils::GetProperty(img_info, "img_h", 0);
    int img_c = Utils::GetProperty(img_info, "img_c", 0);
    if (img_w <= 0 || img_h <=0 || img_c < 1 || img_c > 4) {
        LOGE("Wrong image info!!  img_w:{}, img_h:{} img_c:{}", img_w, img_h, img_c);
        return cv::Mat();
    } else {
        cv::Mat image = cv::Mat(img_h, img_w, Utils::GetCvType(img_c), img_data);
        return image.clone();
    }
}

cv::Mat Utils::CenterCrop(const cv::Mat& image, int width, int height, bool bClone)
{
    width = std::min(width, image.cols);
    height = std::min(height, image.rows);
    int x = (image.cols - width) / 2;
    int y = (image.rows - height) / 2;
    if (bClone) {
        return image(cv::Rect(x, y, width, height)).clone();
    } else {
        return image(cv::Rect(x, y, width, height));
    }
}

int Utils::GetCvType(int img_c)
{
    int cv_type = CV_8UC1;
    switch (img_c)
    {
    case 1:
        cv_type = CV_8UC1;
        break;
    case 2:
        cv_type = CV_8UC2;
        break;
    case 3:
        cv_type = CV_8UC3;
        break;
    case 4:
        cv_type = CV_8UC4;
        break;
    default:
        break;
    }
    return cv_type;
}


std::string Utils::GetErrorCodeText(ErrorCode errCode)
{
    switch (errCode)
    {
    case ErrorCode::OK:
        return "OK";
    case ErrorCode::ABNORMAL_IMAGE:
        return "ABNORMAL_IMAGE";
    case ErrorCode::ABNORMAL_IMG_BRIGHTNESS:
        return "ABNORMAL_IMG_BRIGHTNESS";
    case ErrorCode::ABNORMAL_ANGLE:
        return "ABNORMAL_ANGLE";
    case ErrorCode::NOT_FOUND_TARGET:
        return "NOT_FOUND_TARGET";
    case ErrorCode::OUT_OF_MEMORY:
        return "OUT_OF_MEMORY";
    case ErrorCode::WRONG_PARAM:
        return "WRONG_PARAM";
    case ErrorCode::NOT_READY:
        return "NOT_READY";
    case ErrorCode::TIME_OUT:
        return "TIME_OUT";
    case ErrorCode::DUPLICATE_ALGO_NAME:
        return "DUPLICATE_ALGO_NAME";
    case ErrorCode::INVALID_IMG_DATA:
        return "INVALID_IMG_DATA";
    case ErrorCode::QUEUE_OVERFLOW:
        return "QUEUE_OVERFLOW";
    case ErrorCode::WRONG_STATE:
        return "WRONG_STATE";
    case ErrorCode::UNKNOWN_ERROR:
        return "UNKNOWN_ERROR";
    default:
        return "UNKNOWN_ERROR";
    }
}

json Utils::GenErrorResult(ErrorCode err_code)
{
    return {
        { "class_list", json::array() },
        { "status", Utils::GetErrorCodeText(err_code) },
        { "shapes", json::array() }
    };
}

bool Utils::IsUtf8(const std::string& str)
{
    for (size_t i = 0; i < str.length(); ++i) {
        unsigned char ch = static_cast<unsigned char>(str[i]);
        if (ch <= 0x7F) {
            // 单字节UTF-8字符
            continue;
        } else if (ch >= 0xC2 && ch <= 0xDF) {
            // 双字节UTF-8字符
            if (i + 1 >= str.length() || (str[i + 1] & 0xC0) != 0x80)
                return false;
            ++i;
        } else if (ch >= 0xE0 && ch <= 0xEF) {
            // 三字节UTF-8字符
            if (i + 2 >= str.length() || (str[i + 1] & 0xC0) != 0x80 || (str[i + 2] & 0xC0) != 0x80)
                return false;
            i += 2;
        } else if (ch >= 0xF0 && ch <= 0xF4) {
            // 四字节UTF-8字符
            if (i + 3 >= str.length() || (str[i + 1] & 0xC0) != 0x80 || (str[i + 2] & 0xC0) != 0x80 || (str[i + 3] & 0xC0) != 0x80)
                return false;
            i += 3;
        } else {
            // 非UTF-8字符
            return false;
        }
    }
    return true;
}

std::string Utils::ToLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return result;
}

