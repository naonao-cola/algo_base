#include <iostream>
#include <filesystem>
#include "Interface.h"
#include "utils/StringConvert.h"
#include "utils/Utils.h"
#include "utils/Logger.h"
#include "utils/CrashCatch.h"
#include "nlohmann/json.hpp"
#include "framework/InferenceEngine.h"
#include "framework/ErrorDefine.h"
#include "tv_license.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

#if USE_LICENSE
const char* get_hardware_id()
{
    return TVLicense::Instance()->GetHWID();
}

int tapp_license_verify(void* handle) {

    InferenceEngine* pEngine = static_cast<InferenceEngine*>(handle);
    return (int)pEngine->LicenseVerify();
}
#endif

int tapp_package(const char* out_model_path, const char* origin_model_dir, const char* model_name, const char* model_type, 
						  int input_w, int input_h, int input_c, unsigned int major_version, unsigned int minor_version)
{
    return 0;
}


void* tapp_init()
{
    InferenceEngine* pEngine = InferenceEngine::get_instance();
    ErrorCode ec = pEngine->Init();
    if (ec != ErrorCode::OK) {
        LOGE("tapp_init fail!");
        pEngine->Destroy();
        return nullptr;
    }
    return (int*)pEngine;
}

void tapp_common_config(void* handle, const char* common_config_json)
{
    std::string utf8_str = StringConvert::AnsiToUtf8(std::string(common_config_json));
    json config = json::parse(utf8_str);
    InferenceEngine* pEngine = static_cast<InferenceEngine*>(handle);
    pEngine->ConfigSystemParams(config);
}

void tapp_algo_config(void* handle, const char* algo_config_json)
{
    std::string utf8_str = StringConvert::AnsiToUtf8(std::string(algo_config_json));
    json config = json::parse(utf8_str);
    InferenceEngine* pEngine = static_cast<InferenceEngine*>(handle);
    pEngine->ConfigAlgoParams(config);
}

 void tapp_register_result_callback(void* handle, ResultCallbackFunc callback)
 {
    InferenceEngine* pEngine = static_cast<InferenceEngine*>(handle);
    pEngine->RegisterResultCallback(callback);
 }

 void tapp_register_log_callback(void* handle, LogCallbackFunc callback)
 {
    InferenceEngine* pEngine = static_cast<InferenceEngine*>(handle);
    pEngine->RegisterLogCallback(callback);
 }

 int tapp_run(void* handle, unsigned char* img_data, const char* img_info)
 {
    int ret = 0;
    LOGI("tapp_run() start.")
    InferenceEngine* pEngine = static_cast<InferenceEngine*>(handle);

    if (pEngine == nullptr) {
        std::stringstream ss;
        ss << "0x" << std::setfill('0') << std::setw(sizeof(void*) * 2) << std::hex << reinterpret_cast<uintptr_t>(handle);
        LOGE("Invalide Handle. {}", ss.str());
        return (int)ErrorCode::INVALID_HANDLE;
    }

    try {

#if USE_LICENSE
        ret = (int)pEngine->LicenseVerify();
        if (ret != 0)
        {
            LOGE("License verify fail!! ErrorCode:{}", ret);
            return ret;
        }
#endif

        InferTaskPtr task = std::make_shared<stInferTask>();
        task->image_info = Utils::ParseJsonText(img_info);
        if (img_data != nullptr) {
            task->image = Utils::GenCvImage(img_data, task->image_info);
            task->img_data = task->image.data;
        } else {
            std::string img_path = Utils::GetProperty(task->image_info, "img_path", std::string(""));
            if (fs::exists(img_path)) {
                LOGI("Read Image path: {}", img_path);
                task->image = cv::imread(img_path);
            }
        }

        if (task->image.empty()) {
            LOGE("Gen image obj fail! Wrong image parameter!!");
            return int(ErrorCode::WRONG_PARAM);
        }

        // 提交推理任务
        pEngine->CommitInferTask(task);
        LOGI("tapp_run() End.")
        return ret;
    } catch (const nlohmann::json::exception& e) {
        LOGE("Json exception: {}", e.what());
        CrashCatch::PrintExceptionStackTrace();
    }  catch (const cv::Exception& e) {
        LOGE("OpenCV exception: {}", e.what());
        CrashCatch::PrintExceptionStackTrace();
    } catch (const std::exception& e) {
        LOGE("Unkown exception: {}", e.what());
        CrashCatch::PrintExceptionStackTrace();
    }

    LOGI("tapp_run() Fail!!")
    return (int)ErrorCode::UNKNOWN_ERROR;;
 }

const char* tapp_sync_run(void* handle, unsigned char* img_data, const char* img_info)
 {
    int ret = 0;
    InferenceEngine* pEngine = static_cast<InferenceEngine*>(handle);

    if (pEngine == nullptr) {
        std::stringstream ss;
        ss << "0x" << std::setfill('0') << std::setw(sizeof(void*) * 2) << std::hex << reinterpret_cast<uintptr_t>(handle);
        LOGE("Invalide Handle. {}", ss.str());
        return Utils::DumpJson(Utils::GenErrorResult(ErrorCode::INVALID_HANDLE));;
    }

    try {
#if USE_LICENSE
        ret = (int)pEngine->LicenseVerify();
        if (ret != 0)
        {
            LOGE("tapp_run fail!! ErrorCode:{}", ret);
            return Utils::DumpJson(Utils::GenErrorResult(ErrorCode::LICENSE_ERROR));
        }
#endif
        InferTaskPtr task = std::make_shared<stInferTask>();
        task->image_info = Utils::ParseJsonText(img_info);
        task->image = Utils::GenCvImage(img_data, task->image_info);
        task->img_data = task->image.data;
        if (task->image.empty()) {
            LOGE("Generate image from buffer fail!!");
            return Utils::DumpJson(Utils::GenErrorResult(ErrorCode::INVALID_IMG_DATA));
        }
        return Utils::DumpJson(pEngine->SyncRunInferTask(task));
    } catch (const nlohmann::json::exception& e) {
        LOGE("Json exception: {}", e.what());
        CrashCatch::PrintExceptionStackTrace();
    }  catch (const cv::Exception& e) {
        LOGE("OpenCV exception: {}", e.what());
        CrashCatch::PrintExceptionStackTrace();
    } catch (const std::exception& e) {
        LOGE("Unkown exception: {}", e.what());
        CrashCatch::PrintExceptionStackTrace();
    }

    return Utils::DumpJson(Utils::GenErrorResult(ErrorCode::UNKNOWN_ERROR));
 }

void tapp_destroy(void* handle)
{
    InferenceEngine* pEngine = static_cast<InferenceEngine*>(handle);
    pEngine->Destroy();
}