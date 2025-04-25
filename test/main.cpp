/**
 * @FilePath     : /algo_base/test/main.cpp
 * @Description  :
 * @Author       : naonao
 * @Date         : 2025-03-24 10:11:55
 * @Version      : 0.0.1
 * @LastEditors  : naonao
 * @LastEditTime : 2025-04-25 13:19:26
 * @Copyright (c) 2025 by G, All Rights Reserved.
 **/

#include "nlohmann/json.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <sstream>

#include "../src/Interface.h"
#include "../src/utils/StringConvert.h"
#include "fs.h"

namespace fs = std::filesystem;
using namespace std;
using json = nlohmann::json;


static const std::string WORK_DIR   = filesystem::current_path().string();
const int                task_cnt   = 5;
int                      finish_cnt = 0;
std::vector<cv::Mat>     g_img_list;

template<typename T>
static T GetProperty(const json& json_obj, const std::string& key, const T& def_val)
{
    if (json_obj.contains(key)) {
        return json_obj[key].get<T>();
    }
    else {
        return def_val;
    }
}

json ReadJsonFile(std::string filepath)
{
    std::ifstream     conf_i(filepath);
    std::stringstream ss_config;
    ss_config << conf_i.rdbuf();
    json jsonObj = json::parse(ss_config.str());
    return std::move(jsonObj);
}

// std::string DumpJson(json jsonObj, bool toAnsi = true)
// {
//     return toAnsi ? StringConvert::Utf8ToAnsi(jsonObj.dump(2)) : jsonObj.dump(2);
// }


std::string DumpJson(json jsonObj, bool toAnsi = true)
{
    return toAnsi ? StringConvert::Utf8ToAnsi(jsonObj.dump()) : jsonObj.dump();
}

void RecultCallback(const char* img_info_json, const char* result_json)
{
    std::cout << "### [Result_Callback] --------" << std::endl;
    std::cout << "[IMG_INFO]" << img_info_json << std::endl;
    std::cout << "[Result]" << result_json << std::endl;
    finish_cnt++;
}

void LogCallback(int level, const char* log_msg)
{
    std::cout << "[LOG_CALLBACK] [" << level << "]:" << log_msg << std::endl;
}

void* pHandle = nullptr;


void test_single(const std::string& image_file, const std::string& img_1_path)
{
    size_t      lastSlashPos = img_1_path.find_last_of("/\\");
    size_t      start        = (lastSlashPos == std::string::npos) ? 0 : lastSlashPos + 1;
    size_t      dotPos       = img_1_path.find_last_of('.');
    std::string imgName      = img_1_path.substr(start, dotPos - start);

    json    image_info;
    cv::Mat img1;
    img1 = cv::imread(img_1_path);

    image_info = ReadJsonFile(image_file);
    std::cout << "image_info main: " << image_info.dump(2) << std::endl;
    image_info["img_w"]    = img1.cols;
    image_info["img_h"]    = img1.rows;
    image_info["img_c"]    = img1.channels();
    image_info["img_path"] = img_1_path;
    image_info["img_name"] = imgName;
    const char* ret_json   = tapp_sync_run(pHandle, img1.data, DumpJson(image_info).c_str());
    std::cout << "ret_json: " << ret_json << std::endl;
}


int main(int args, char** argv)
{
    // E:\demo\repo\algo_base\config\ocr_algo_cfg.json

    // std::string test_folder = "./test_data/tayin/";
    json common_cfg = ReadJsonFile(R"(E:\demo\repo\algo_base\config\ocr_common_cfg.json)");
    json algo_cfg   = ReadJsonFile(R"(E:\demo\repo\algo_base\config\ocr_algo_cfg.json)");
    json image_info = ReadJsonFile(R"(E:\demo\repo\algo_base\config\image_info.json)");

    std::cout << "CommonConfig main: " << common_cfg.dump(2) << std::endl;
    std::cout << "algo_cfg main: " << algo_cfg.dump(2) << std::endl;
    std::cout << "image_info main: " << DumpJson(image_info) << std::endl;

    pHandle = tapp_init();

    int errCode = tapp_common_config(pHandle, DumpJson(common_cfg).c_str());

    errCode = tapp_algo_config(pHandle, DumpJson(algo_cfg).c_str());


    tapp_register_result_callback(pHandle, RecultCallback);



    std::vector<std::string> img_file_A;
    nao::fl::getAllFormatFiles(R"(E:\demo\cxx\al_base\test_data\rbc\)", img_file_A, "(.*)(BRI_0003_m0.bmp)");


    for (int i = 0; i < img_file_A.size(); i++) {
        test_single(R"(E:\demo\repo\algo_base\config\image_info.json)", img_file_A[i]);
        Sleep(3000);
    }

    Sleep(100);
    std::cout << "Finished count:" << finish_cnt << std::endl;
    std::cout << "-------------------- Begin Destroy." << std::endl;
    tapp_destroy(pHandle);
    std::cout << "==================== End Destroy." << std::endl;
    system("pause");
    return 0;
}