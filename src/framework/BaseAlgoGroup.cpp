#include "../utils/Utils.h"
#include "../utils/logger.h"
#include "BaseAlgoGroup.h"
#include "AlgoManager.h"
#include "../utils/TimeCost.h"
#include "../utils/CrashCatch.h"


BaseAlgoGroup::BaseAlgoGroup()
{
}

BaseAlgoGroup::~BaseAlgoGroup()
{
}

// 设置算法组的参数，如果第一次调用则创建相关算法对象
ErrorCode BaseAlgoGroup::SetParams(const json &algo_group_cfg)
{
    m_type_id = algo_group_cfg["type_id"];
    m_type_name = algo_group_cfg["type_name"];
    json pre_algo_param_list = algo_group_cfg["pre_algo_params"];
    json algo_param_list = algo_group_cfg["algo_params"];
    ErrorCode e_code = ErrorCode::OK;

    for (auto pre_algo_param : pre_algo_param_list)
    {
        e_code = SetAlgoParam(pre_algo_param, true);
        CHECK_ERRORCODE_RETURN(e_code)
    }

    for (auto algo_param : algo_param_list)
    {
        e_code = SetAlgoParam(algo_param, false);
        CHECK_ERRORCODE_RETURN(e_code)
    }

    // 预处理算法按照算法编号“algo_index”进行排序，串行执行
    std::sort(m_pre_list.begin(), m_pre_list.end(), [=](BaseAlgo *a, BaseAlgo *b) { return a->GetAlgoIndex(m_type_id) < b->GetAlgoIndex(m_type_id); });

    return ErrorCode::OK;
}

/** 设置算法参数，如果是第一次设置则创建算法对象，否则更新算法参数
* @algo_param：算法参数json
* @is_preprocess: 是否是预处理算法
*/
ErrorCode BaseAlgoGroup::SetAlgoParam(const json &algo_param, bool is_preprocess)
{
    std::string algo_name = Utils::GetProperty(algo_param, "algo_name", std::string("unknown"));
    if (algo_name == "unknown")
    {
        LOGE("Algo parameter wrong. Must specify [algo_name]!")
        return ErrorCode::WRONG_PARAM;
    }

    // 如果算法对象不存在则创建后再设置参数
    BaseAlgo *pAlgo = AlgoManager::get_instance()->GetAlgo(algo_name);
    if (pAlgo == nullptr)
    {
        pAlgo = AlgoManager::CreateAlgo(algo_name);

        if (pAlgo == nullptr)
        {
            LOGE("Create [Algo] fail. Invalid algo name:{}", algo_name);
            return ErrorCode::WRONG_PARAM;
        }

        AlgoManager::get_instance()->AddAlgo(pAlgo);
    }

    if (is_preprocess)
        m_pre_list.emplace_back(pAlgo);
    else
        m_algo_list.emplace_back(pAlgo);

    // 设置对应type_id的算法参数
    pAlgo->SetParam(m_type_id, algo_param);
    
    return ErrorCode::OK;
}

FinalResultPtr BaseAlgoGroup::RunGroup(InferTaskPtr task)
{
    LOGI("Run Group <{}> Start.", m_type_name);
    FinalResultPtr final_result = std::make_shared<stFinalResult>();
    final_result->image_info = task->image_info;
    final_result->results = {
        {"class_list", json::array()},
        {"status", "OK"},
        {"shapes", json::array()}
    };

    TimeCost group_tc;
    group_tc.start();

    std::vector<AlgoResultPtr> pre_results;
    for (auto preAlgo : m_pre_list)
    {
        TimeCost tc;
        tc.start();
        
        AlgoResultPtr pre_result;

        try{
            LOGI("Run PreAlgo <{}> Start.", preAlgo->GetName());
            pre_result = preAlgo->RunAlgo(task);
            LOGI("Run PreAlgo <{}> End. cost:{}(ms)", preAlgo->GetName(), tc.get_cost_time());
        } catch (const nlohmann::json::exception& e) {
            LOGE("Json exception: {}", e.what());
            CrashCatch::PrintExceptionStackTrace();
        }  catch (const cv::Exception& e) {
            LOGE("OpenCV exception: {}", e.what());
            CrashCatch::PrintExceptionStackTrace();
        }
        catch (const std::exception& e) {
            LOGE("Unkown exception: {}", e.what());
            CrashCatch::PrintExceptionStackTrace();
        }

        if (!pre_result) {
            final_result->results["status"] = Utils::GetErrorCodeText(ErrorCode::UNKNOWN_ERROR);
            return final_result;
        }

        if (pre_result->status != ErrorCode::OK)
        {
            final_result->results["status"] = Utils::GetErrorCodeText(pre_result->status);
            return final_result;
        }
        pre_results.emplace_back(pre_result);
    }

    std::vector<std::future<AlgoResultPtr>> future_results;
    InferenceEngine *pEngine = InferenceEngine::get_instance();

    for (auto algo : m_algo_list)
    {
        future_results.emplace_back(pEngine->GetThreadPool()->enqueue([=, &pre_results]()
                                                                        {
                TimeCost tc;
                tc.start();
                AlgoResultPtr rst;
                try{
                    LOGI("Run Algo <{}> Start.", algo->GetName());
                    rst = algo->RunAlgo(task, pre_results);
                    LOGI("Run Algo <{}> End. cost:{}(ms)", algo->GetName(), tc.get_cost_time());
                } catch (const nlohmann::json::exception& e) {
                    LOGE("Json exception: {}", e.what());
                    CrashCatch::PrintExceptionStackTrace();
                }  catch (const cv::Exception& e) {
                    LOGE("OpenCV exception: {}", e.what());
                    CrashCatch::PrintExceptionStackTrace();
                }
                catch (const std::exception& e) {
                    LOGE("Unkown exception: {}", e.what());
                    CrashCatch::PrintExceptionStackTrace();
                }
                
                return rst; 
            }));
    }

    for (auto && result : future_results)
    {
        AlgoResultPtr ar = result.get();
        if (!ar) {
            LOGE("Algo no result!!");
            final_result->results["status"] = Utils::GetErrorCodeText(ErrorCode::UNKNOWN_ERROR);
            return final_result;
        }
        
        if (ar->status != ErrorCode::OK) {
            final_result->results["status"] = Utils::GetErrorCodeText(ar->status);
            return final_result;
        }
        
        // final_result->results["shapes"].emplace_back(ar->result_info);
        if (ar->result_info.is_null()) {
            continue;
        }

        if (ar->result_info.is_array()) {
            for (auto item : ar->result_info) {
                final_result->results["shapes"].emplace_back(item);
            }
        } else {
            final_result->results["shapes"].emplace_back(ar->result_info);
        }
    }
    
    
    std::string img_name = task->image_info["img_name"];
    LOGI("Run Group <{}> End. cost:{}(ms)", m_type_name, group_tc.get_cost_time());
    return final_result;
}

