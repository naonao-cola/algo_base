
#include "AlgoA.h"
#include "../framework/InferenceEngine.h"
#include "../utils/logger.h"

#if USE_AI_DETECT
#include <AIRuntimeDataStruct.h>
#include <AIRuntimeInterface.h>
#include <AIRuntimeUtils.h>
#endif   // USE_AI_DETECT
REGISTER_ALGO(AlgoA)

AlgoA::AlgoA() {}

AlgoA::~AlgoA() {}

AlgoResultPtr AlgoA::RunAlgo(InferTaskPtr task, std::vector<AlgoResultPtr> pre_results)
{
    LOGI("AlgoA start run!");

    AlgoResultPtr algo_result = std::make_shared<stAlgoResult>();
    algo_result->result_info  = {
        {"label", "AlgoA_defect"},
        {"shapeType", "rectangle"},
        {"points", {{0, 0}, {150.0, 150.0}}},
        {"result", {{"confidence", 0.78}, {"area", 10000}}},
    };

    LOGI("AlgoA run finished!");
    throw std::runtime_error("AlgoA failed");
    return algo_result;
}