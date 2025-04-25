#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) && defined(EXPORT_API)
#define __export __declspec(dllexport)
#elif defined(__GNUC__) && ((__GNUC__ >= 4) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3))
#define __export __attribute__((visibility("default")))
#else
#define __export
#endif


typedef void (*ResultCallbackFunc)(const char* img_info_json, const char* result_json);
typedef void (*LogCallbackFunc)(int level, const char* log_msg);



/**
 * tapp对象初始化，返回对象句柄指针
 */
__export void* tapp_init();


/**
 * 算法运行通用参数配置，如并行线程数，优先级，日志级别等
 * @common_config_json: json配置参数, 线程配置，日志输出等
 */
__export int tapp_common_config(void* handle, const char* common_config_json);

/**
 * 检测算法具体参数、判定条件配置
 */
__export int tapp_algo_config(void* handle, const char* algo_config_json);

/**
 * 注册结果回调函数，回调结果以json字符串型式返回
 */
__export void tapp_register_result_callback(void* handle, ResultCallbackFunc callback);

/**
 * 注册算法日志回调函数，回调参数中包含日志级别（debug=0, verbose=1, info=2, warning=3, error=4, fatal=5), 日志内容字符串
 */
__export void tapp_register_log_callback(void* handle, LogCallbackFunc callback);

/**
 * 图片异步推理接口，结果通过回调函数返回
 *
 * @img_data： 图片数据，如果读本地图片方式，该参数可以为空，图片路径通过img_info_json设置
 * @img_info_json：图片信息
 */
__export int tapp_run(void* handle, unsigned char* img_data, const char* img_info_json);


/**
 * 图片同步推理接口
 *
 * @img_data： 图片数据，如果读本地图片方式，该参数可以为空，图片路径通过img_info_json设置
 * @img_info_json：图片信息
 */
__export const char* tapp_sync_run(void* handle, unsigned char* img_data, const char* img_info_json);

// 销毁tapp对象相关资源
__export void tapp_destroy(void* handle);

#ifdef __cplusplus
}
#endif
