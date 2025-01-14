#include <iostream>
#include <opencv2/opencv.hpp>
#include <co/unitest.h>
#include <co/time.h>
#include <co/cout.h>
#include <nlohmann/json.hpp>
#include "../src/utils/MemoryPool.h"

using json = nlohmann::json;

namespace test {
    DEF_test(MEMORYPOOL) {
         //DEF_case(memory_accuracy) {
         //    json profile ={
	        //             {"block_size_offset",2},

         //                {"block_size_0",2},
	        //             {"block_count_0",100},

         //                {"block_size_1",4},
         //                {"block_count_1",100},

         //                {"block_size_2",8},
         //                {"block_count_2",100},

         //                {"block_size_3",16},
         //                {"block_count_3",100},

         //                {"block_size_4",32},
         //                {"block_count_4",4}
         //                };
         //    MemoryPool* pool_obj = MemoryPool::get_instance();
         //    pool_obj->Initialize(profile);
         //    std::vector<cv::Mat> img_vec;
         //    int repeat_count = 100;
         //    cv::RNG rng(12345);
         //    for (int i = 0; i < repeat_count; i++) {
         //        cv::Mat img;
         //        pool_obj->CreateMat(img,4000, 4000, CV_8UC1);
         //        img_vec.emplace_back(img);
         //    }
         //    for (int i = 0; i < repeat_count; i++) {
         //        pool_obj->ReleaseMat(img_vec[i]);
         //    }
         //    EXPECT_EQ(pool_obj->new_count, 0);
         //    EXPECT_EQ(pool_obj->memoryBlocks_.size(), 404);
         //}//DEF_case(memory_accuracy)

        //DEF_case(memory_time){
        //         int repeat_count = 3000;
        //         json profile ={
	       //             {"block_size_offset",2},
        //                {"block_size_0",4},
	       //             {"block_count_0",repeat_count}
        //                };

        //            MemoryPool* pool_obj = MemoryPool::get_instance();
        //            pool_obj->Initialize(profile);
        //        auto fun1 = [&](){
        //            std::vector<cv::Mat> img_vec;
        //            for (int i = 0; i < repeat_count; i++) {
        //                cv::Mat img;
        //                pool_obj->CreateMat(img,2000, 2000, CV_8UC1, false);
        //                img_vec.emplace_back(img);
        //            }
        //            for (int i = 0; i < repeat_count; i++) {
        //                pool_obj->ReleaseMat(img_vec[i]);
        //            }
        //        };
        //        auto fun2=[&](){
        //            std::vector<cv::Mat> img_vec;
        //            for (int i = 0; i < repeat_count; i++) {
        //                cv::Mat tmp;
        //                tmp.create(2000, 2000, CV_8UC1);
        //                img_vec.emplace_back(tmp);
        //               
        //            }
        //            for (int i = 0; i < repeat_count; i++) {
        //               img_vec[i].release();
        //            }
        //        };
        //        size_t requiredSize = static_cast<size_t>(2000 * 2000 * CV_ELEM_SIZE(CV_8UC1));
        //        auto fun3 =[&](){
        //             std::vector<uchar*> img_vec;
        //            for (int i = 0; i < repeat_count; i++) {
        //                uchar* tmp = new uchar(requiredSize);
        //                img_vec.emplace_back(tmp);
        //            }
        //            for (int i = 0; i < repeat_count; i++) {
        //               delete[] img_vec[i];
        //            }

        //        };
        //        co::Timer t;
        //        std::thread th_1(fun1);
        //        th_1.join();
        //        int64 time_1 = t.us();

        //        t.restart();
        //        std::thread th_2(fun2);
        //        th_2.join();
        //        int64 time_2 = t.us();

        //        t.restart();
        //        std::thread th_3(fun3);
        //        th_3.join();
        //        int64 time_3 = t.us();

        //        char msg[1024];
        //        sprintf(msg, "MemoryPool time: %lld us\n", time_1);
        //        std::cout<<text::green(msg);
        //        sprintf(msg, "OpenCv time: %lld us\n",time_2);
        //        std::cout<<text::green(msg);
        //        sprintf(msg, "System new time: %lld us\n",time_3);
        //        std::cout<<text::green(msg);
        //        EXPECT_GE(time_2, time_1);
        //}//DEF_case(memory_time)
        DEF_case(memory_thread) {
            int repeat_count = 2000;
            json profile = {
                   {"block_size_offset",2},
                   {"block_size_0",4},
                   {"block_count_0",repeat_count}
            };

            MemoryPool* pool_obj = MemoryPool::get_instance();
            pool_obj->Initialize(profile);
            auto fun1 = [&]() {
                std::vector<cv::Mat> img_vec;
                for (int i = 0; i < 400; i++) {
                    cv::Mat img;
                    pool_obj->CreateMat(img, 2000, 2000, CV_8UC1, false);
                    img_vec.emplace_back(img);
                }
                for (int i = 0; i < 400; i++) {
                    pool_obj->ReleaseMat(img_vec[i]);
                }
            };
            auto fun2 = [&]() {
                std::vector<cv::Mat> img_vec;
                for (int i = 0; i < 400; i++) {
                    cv::Mat tmp;
                    tmp.create(2000, 2000, CV_8UC1);
                    img_vec.emplace_back(tmp);

                }
                for (int i = 0; i < 400; i++) {
                    img_vec[i].release();
                }
            };
            std::vector<std::thread> thread_vec;
            co::Timer t;
            for (int i = 0; i < 5;i++) {
                thread_vec.emplace_back(std::thread(fun1));
            }
            for (int i = 0; i < 5; i++) {
                thread_vec[i].join();
            }
            int64 time_1 = t.us();


            thread_vec.clear();
            t.restart();
            for (int i = 0; i < 5; i++) {
                thread_vec.emplace_back(std::thread(fun2));
            }
            for (int i = 0; i < 5; i++) {
                thread_vec[i].join();
            }
            int64 time_2 = t.us();

           

            char msg[1024];
            sprintf(msg, "MemoryPool time: %lld us\n", time_1);
            std::cout << text::green(msg);
            sprintf(msg, "OpenCv time: %lld us\n", time_2);
            std::cout << text::green(msg);
            
            EXPECT_GE(time_2, time_1);
        }//DEF_case(memory_thread)
    }//DEF_test
}//namespace test

