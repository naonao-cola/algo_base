#pragma once

#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include "StringConvert.h"
#include "nlohmann/json.hpp"
#include <exception>
#include "../framework/Defines.h"
#include "../framework/ErrorDefine.h"


using json = nlohmann::json;

class Utils
{
public:
    static json ReadJsonFile(std::string filepath);
    static json ParseJsonText(const char* json_text, bool is_ansi=true);
    static std::string DumpJson(json jsonObj, bool toAnsi=true);
    static cv::Mat GenCvImage(unsigned char* img_data, const json& img_info);
    static int GetCvType(int img_c);

    static cv::Mat CenterCrop(const cv::Mat& image, int width, int height, bool bClone=false);

    static bool IsUtf8(const std::string& str);
    static std::string ToLowerCase(const std::string& str);

    static std::string GetErrorCodeText(ErrorCode errCode);
    static json GenErrorResult(ErrorCode err_code);

    template<typename T>
    static T GetProperty(const json& json_obj, const std::string& key, const T& def_val)
    {
        if (json_obj.contains(key)) {
            return json_obj[key].get<T>();
        } else {
            return def_val;
        }
    }
};



