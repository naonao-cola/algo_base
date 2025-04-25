/**
 * @FilePath     : /algo_base/src/framework/BaseAlgoGroup.h
 * @Description  :
 * @Author       : naonao
 * @Date         : 2025-03-24 10:11:55
 * @Version      : 0.0.1
 * @LastEditors  : naonao
 * @LastEditTime : 2025-04-25 10:57:23
 * @Copyright (c) 2025 by G, All Rights Reserved.
 **/
#pragma once
#include "BaseAlgo.h"
#include "ErrorDefine.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <vector>

using json = nlohmann::json;

class ThreadPool;
class InferenceEngine;

// 算法组基类，对应一张图片的所有算法
class BaseAlgoGroup
{
public:
    BaseAlgoGroup();
    virtual ~BaseAlgoGroup();

    ErrorCode              SetParams(const json& algo_group_cfg);
    virtual FinalResultPtr RunGroup(InferTaskPtr task);
    // virtual json MergeResults(InferTaskPtr task);
    std::string GetTypeID()
    {
        return m_type_id;
    };
    std::string GetTypeName()
    {
        return m_type_name;
    };

private:
    ErrorCode SetAlgoParam(const json& algo_param, bool is_preprocess);

private:
    DCLEAR_ALGO_GROUP_REGISTER(BaseAlgoGroup)
    std::vector<BaseAlgo*> m_algo_list;
    std::vector<BaseAlgo*> m_pre_list;
    std::string            m_type_name;
    std::string            m_type_id;
};