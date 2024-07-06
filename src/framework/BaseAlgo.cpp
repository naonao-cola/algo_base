#include "../utils/Utils.h"
#include "BaseAlgo.h"
#include "InferenceEngine.h"
#include <filesystem>

namespace fs = std::filesystem;

void BaseAlgo::SetParam(const std::string type_id, const json& params)
{
    auto it = m_param_map.find(type_id);
    if (it == m_param_map.end()) {
        m_param_map.insert(std::make_pair(type_id, params));
    } else {
        m_param_map[type_id] = params;
    }
    m_name = Utils::GetProperty(params, "algo_name", std::string("Unknown"));
}

AlgoResultPtr BaseAlgo::RunAlgo(InferTaskPtr task, std::vector<AlgoResultPtr> pre_results)
{
    return std::make_shared<stAlgoResult>();
}

int BaseAlgo::GetAlgoIndex(const std::string type_id)
{
    auto it = m_param_map.find(type_id);
    if (it == m_param_map.end()) {
        return -1;
    } else {
        return Utils::GetProperty(m_param_map[type_id], "algo_index", -1);
    }
}

json BaseAlgo::GetTaskParams(InferTaskPtr task)
{
    std::string type_id = Utils::GetProperty(task->image_info, "type_id", std::string(""));
    auto it = m_param_map.find(type_id);
    if (it == m_param_map.end()) {
        return {};
    } else {
        return m_param_map[type_id];
    }
}

bool BaseAlgo::IsDebug()
{
    json common_cfg = InferenceEngine::get_instance()->GetCommonCfg();
    return Utils::GetProperty(common_cfg, "debug_mode", 0) != 0;
}

void BaseAlgo::SaveDebugImage(const cv::Mat& image, const std::string name, bool isFullPath)
{
    std::string parent_path_str = "./debug_img";
    std::string filename = name;
    if (isFullPath) {
        fs::path fullpath(name);
        parent_path_str = fullpath.parent_path().string();
        filename = fullpath.filename().string();
    }

    fs::path parent_path(parent_path_str);

    if(!fs::exists(parent_path)) {
        fs::create_directories(parent_path);
    }

    cv::imwrite(parent_path_str+"/"+filename, image);

}