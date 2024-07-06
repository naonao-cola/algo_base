#pragma once

#include "Defines.h"
#include "../framework/BaseAlgo.h"
#include "../framework/DataStruct.h"


#ifdef USE_AI_DETECT
class OcrBaseAlgo : public BaseAlgo
{
public:
    OcrBaseAlgo();
    ~OcrBaseAlgo();
    virtual AlgoResultPtr RunAlgo(InferTaskPtr task, std::vector<AlgoResultPtr> pre_results);

protected:
    virtual void TextDet(cv::Mat image, std::vector<OcrDataPtr>& ocrdata_list, const json& params = {});
    virtual void TextRec(cv::Mat image, std::vector<OcrDataPtr>& ocrdata_list, const json& params = {});

    virtual void FilterDetResult(std::vector<OcrDataPtr>& ocrdata_list, const json& params = {});
    virtual void FilterRecResult(std::vector<OcrDataPtr>& ocrdata_list, const json& params = {});
};

#endif
