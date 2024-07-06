#include "OcrBaseAlgo.h"
#include <windows.h>
#include "../utils/logger.h"
#include "../utils/Utils.h"
#include "../utils/StringConvert.h"
#include <AIRuntimeInterface.h>


#ifdef USE_AI_DETECT

OcrBaseAlgo::OcrBaseAlgo()
{

}

OcrBaseAlgo::~OcrBaseAlgo()
{

}

AlgoResultPtr OcrBaseAlgo::RunAlgo(InferTaskPtr task, std::vector<AlgoResultPtr> pre_results)
{
    LOGI("OcrBaseAlgo start run!");
    json params = GetTaskParams(task);

    AlgoResultPtr algo_result = std::make_shared<stAlgoResult>();

    int x_offset = (task->image.cols - task->image.rows) / 2;
    int crop_wh = task->image.rows;
    cv::Mat crop_img = task->image(cv::Rect(x_offset, 0, crop_wh, crop_wh));
    cv::resize(crop_img, task->image, cv::Size(crop_wh / 3, crop_wh / 3));

    std::vector<OcrDataPtr> ocrdata_list;
    TextDet(task->image, ocrdata_list);
    FilterDetResult(ocrdata_list);
    TextRec(task->image, ocrdata_list);
    FilterRecResult(ocrdata_list);

    algo_result->result_info = json::array();
    for (auto ocrdata : ocrdata_list) {
        algo_result->result_info.emplace_back(ocrdata->ToJsonResult());
    }

    // LOGI("OcrBaseAlgo run finished! {}", algo_result->result_info.dump(2));
    return algo_result;
}

void OcrBaseAlgo::TextDet(cv::Mat image, std::vector<OcrDataPtr>& ocrdata_list, const json& params)
{
    LOGD("TextDet Start.");

    AlgoResultPtr algo_result = std::make_shared<stAlgoResult>();
    TaskInfoPtr _task = std::make_shared<stTaskInfo>();
    _task->imageData  = { image };
    _task->modelId    = 0;
    _task->taskId     = 0;

    ModelResultPtr ocrDetRst = GetAIRuntime()->RunInferTask(_task);

    stModelResult* pResult = ocrDetRst.get();
    if (pResult == nullptr) {
        LOGE("NO result");
    }

    auto item_list = ocrDetRst->itemList[0];    // 只有一张图片
    for (auto& item : item_list) {
        OcrDataPtr ocr_data = std::make_shared<OcrData>();
        ocr_data->SetPoints(item.points);
        ocrdata_list.push_back(ocr_data);
    }

    if (IsDebug()) {
        cv::Mat drawImg = image.clone();
        for (auto ocrdata : ocrdata_list) {
            ocrdata->DrawResult(drawImg, false);
        }
        SaveDebugImage(drawImg, "LabelDetResult.jpg");
    }

    LOGD("TextDet End.");
}

void OcrBaseAlgo::TextRec(cv::Mat image, std::vector<OcrDataPtr>& ocrdata_list, const json& params)
{
    LOGD("TextRec Start.");
    int index = 0;
    for (auto ocrdata : ocrdata_list) {
        index++;
        cv::Mat itemImg = ocrdata->GetCropImage(image);
        TaskInfoPtr _task = std::make_shared<stTaskInfo>();
        _task->imageData  = { itemImg };
        _task->modelId    = 1;
        _task->taskId     = 0;

        ModelResultPtr subRecRst = GetAIRuntime()->RunInferTask(_task);

        if (subRecRst->itemList.size() == 0) {
            continue;
        }

        auto itemList = subRecRst->itemList[0];
        if (itemList.size() == 0) {
            continue;
        }

        ocrdata->text = itemList[0].ocr_str;
        ocrdata->conf = itemList[0].confidence;
    }
    LOGD("TextRec End.");
}

void OcrBaseAlgo::FilterDetResult(std::vector<OcrDataPtr>& ocrdata_list, const json& params)
{
    // auto it = ocrdata_list.begin();
    // while (it != ocrdata_list.end()) {
    //     OcrDataPtr ocrdata = *it;
    //     if (ocrdata->GetAspectRatio() < 6) {
    //         it = ocrdata_list.erase(it);
    //     } else {
    //         ++it;
    //     }
    // }
}

void OcrBaseAlgo::FilterRecResult(std::vector<OcrDataPtr>& ocrdata_list, const json& params)
{
    // auto it = ocrdata_list.begin();
    // while (it != ocrdata_list.end()) {
    //     OcrDataPtr ocrdata = *it;
    //     std::set<char> firstCharSet = {'A', 'B', 'C'};
    //     if (ocrdata->text.length() < 12 ||
    //         firstCharSet.find(ocrdata->text[0]) == firstCharSet.end()) {
    //         LOGD("OcrBaseAlgo skip result: {}, len:{}, score:{}", ocrdata->text, ocrdata->text.length(), ocrdata->conf);
    //         it = ocrdata_list.erase(it);
    //     } else {
    //         ocrdata->labelName = "Catalog";
    //         ++it;
    //     }
    // }
}
#endif




