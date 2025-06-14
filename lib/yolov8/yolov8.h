// Copyright (c) 2023 by Rockchip Electronics Co., Ltd. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _RKNN_DEMO_YOLOV8_H_
#define _RKNN_DEMO_YOLOV8_H_

#include "rknn_api.h"
#include "stream_decoder.h"

#include <rga/RgaApi.h>
#include <rga/im2d.h>
#include <rga/rga.h>
#include <string>
#include <vector>

typedef struct
{
    rknn_context          rknn_ctx;
    rknn_input_output_num io_num;
    rknn_tensor_attr*     input_attrs;
    rknn_tensor_attr*     output_attrs;

    int  model_channel;
    int  model_width;
    int  model_height;
    bool is_quant;

    // Letter buffer for RGA operations
    rga_buffer_t         letter_buffer;
    std::vector<uint8_t> letter_buffer_data;

    // Detection parameters
    int                      obj_class_num;
    float                    nms_thresh;
    float                    box_thresh;
    std::vector<std::string> labels;
} rknn_app_context_t;

#include "postprocess.h"

int yolov8_detect_init(const char*                     model_path,
                       rknn_app_context_t*             app_ctx,
                       int                             obj_class_num,
                       float                           nms_thresh,
                       float                           box_thresh,
                       const std::vector<std::string>& labels);

int yolov8_detect_release(rknn_app_context_t* app_ctx);

int yolov8_detect_run(rknn_app_context_t* app_ctx, rga_buffer_t* input_buffer, detect_result_group_t* od_results);

int yolov8_detect_run_raw(rknn_app_context_t* app_ctx, const FrameDataInfo& frame_info, detect_result_group_t* od_results);

#endif  //_RKNN_DEMO_YOLOV8_H_