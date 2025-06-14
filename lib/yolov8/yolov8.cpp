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

#include "yolov8.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include "common.h"
#include "file_utils.h"

#include <rga/RgaApi.h>
#include <rga/im2d.h>
#include <rga/rga.h>

static void dump_tensor_attr(rknn_tensor_attr* attr)
{
    /*
    printf("  index=%d, name=%s, n_dims=%d, dims=[%d, %d, %d, %d], n_elems=%d, size=%d, fmt=%s, type=%s, qnt_type=%s, "
           "zp=%d, scale=%f\n",
           attr->index, attr->name, attr->n_dims, attr->dims[3], attr->dims[2], attr->dims[1], attr->dims[0],
           attr->n_elems, attr->size, get_format_string(attr->fmt), get_type_string(attr->type),
           get_qnt_type_string(attr->qnt_type), attr->zp, attr->scale);
    */
}

static int letter_box(rga_buffer_t* input_buffer, rga_buffer_t* output_buffer, int model_input_size)
{
    if (!input_buffer || !output_buffer || !output_buffer->vir_addr)
    {
        printf("Invalid input or output buffer\n");
        if (!input_buffer)
            printf("input_buffer is null\n");
        if (!output_buffer)
            printf("output_buffer is null\n");
        if (output_buffer && !output_buffer->vir_addr)
            printf("output_buffer->vir_addr is null\n");
        return -1;
    }

    // printf("Input buffer: width=%d, height=%d, wstride=%d, hstride=%d, format=%d\n",
    //        input_buffer->width,
    //        input_buffer->height,
    //        input_buffer->wstride,
    //        input_buffer->hstride,
    //        input_buffer->format);
    // printf("Output buffer: width=%d, height=%d, wstride=%d, hstride=%d, format=%d\n",
    //        output_buffer->width,
    //        output_buffer->height,
    //        output_buffer->wstride,
    //        output_buffer->hstride,
    //        output_buffer->format);

    int   input_width  = input_buffer->width;
    int   input_height = input_buffer->height;
    float ratio        = fminf((float) model_input_size / input_width, (float) model_input_size / input_height);

    int new_width  = round(ratio * input_width);
    int new_height = round(ratio * input_height);

    // printf("New dimensions: width=%d, height=%d\n", new_width, new_height);

    int height_padding = 0;
    int width_padding  = 0;
    int top            = 0;
    int bottom         = 0;
    int left           = 0;
    int right          = 0;
    if (new_width >= new_height)
    {
        height_padding = new_width - new_height;
        if ((height_padding % 2) == 0)
        {
            top    = (int) ((float) (height_padding / 2));
            bottom = (int) ((float) (height_padding / 2));
        }
        else
        {
            top    = (int) ((float) (height_padding / 2));
            bottom = (int) ((float) (height_padding / 2)) + 1;
        }
    }
    else
    {
        width_padding = new_height - new_width;
        if ((width_padding % 2) == 0)
        {
            left  = (int) ((float) (width_padding / 2));
            right = (int) ((float) (width_padding / 2));
        }
        else
        {
            left  = (int) ((float) (width_padding / 2));
            right = (int) ((float) (width_padding / 2)) + 1;
        }
    }

    // printf("Padding: top=%d, bottom=%d, left=%d, right=%d\n", top, bottom, left, right);

    // Create output buffer
    output_buffer->width    = model_input_size;
    output_buffer->height   = model_input_size;
    output_buffer->wstride  = (model_input_size + 15) & ~15;  // RGA stride (pixels)
    output_buffer->hstride  = model_input_size;
    output_buffer->format   = RK_FORMAT_RGB_888;
    output_buffer->fd       = -1;
    output_buffer->phy_addr = 0;

    // Create resize buffer
    rga_buffer_t resize_dst = {};
    resize_dst.width        = new_width;
    resize_dst.height       = new_height;
    resize_dst.wstride      = (new_width + 15) & ~15;  // RGA stride (pixels)
    resize_dst.hstride      = new_height;
    resize_dst.format       = RK_FORMAT_RGB_888;
    resize_dst.fd           = -1;
    resize_dst.phy_addr     = 0;
    resize_dst.vir_addr     = (uint8_t*) output_buffer->vir_addr + (top * output_buffer->wstride + left) * 3;

    // printf("Resize buffer: width=%d, height=%d, wstride=%d, hstride=%d, format=%d\n",
    //         resize_dst.width,
    //         resize_dst.height,
    //         resize_dst.wstride,
    //         resize_dst.hstride,
    //         resize_dst.format);

    // First clear the output buffer to black
    if (output_buffer->vir_addr)
    {
        memset(output_buffer->vir_addr, 0, output_buffer->wstride * output_buffer->hstride * 3);
    }

    // Perform resize
    IM_STATUS status = imresize(*input_buffer, resize_dst, 0, 0, IM_SYNC);
    if (status != IM_STATUS_SUCCESS && status != IM_STATUS_NOERROR)
    {
        printf("RGA resize failed: %d\n", status);
        return -1;
    }

    return 0;
}

static int scale_coords(detect_result_group_t* detect_result_group, int img_width, int img_height, int model_size)
{
    for (int i = 0; i < detect_result_group->count; i++)
    {
        detect_result_t* det_result = &(detect_result_group->results[i]);

        int x1 = det_result->box.left;
        int y1 = det_result->box.top;
        int x2 = det_result->box.right;
        int y2 = det_result->box.bottom;

        if (img_width >= img_height)
        {
            int   image_max_len = img_width;
            float gain;
            gain               = (float) model_size / image_max_len;
            int resized_height = img_height * gain;
            int height_pading  = (model_size - resized_height) / 2;
            y1                 = (y1 - height_pading);
            y2                 = (y2 - height_pading);
            x1                 = int(x1 / gain);
            y1                 = int(y1 / gain);
            x2                 = int(x2 / gain);
            y2                 = int(y2 / gain);

            det_result->box.left   = x1;
            det_result->box.top    = y1;
            det_result->box.right  = x2;
            det_result->box.bottom = y2;
        }
        else
        {
            int   image_max_len = img_height;
            float gain;
            gain              = (float) model_size / image_max_len;
            int resized_width = img_width * gain;
            int width_pading  = (model_size - resized_width) / 2;
            x1                = (x1 - width_pading);
            x2                = (x2 - width_pading);
            x1                = int(x1 / gain);
            y1                = int(y1 / gain);
            x2                = int(x2 / gain);
            y2                = int(y2 / gain);

            det_result->box.left   = x1;
            det_result->box.top    = y1;
            det_result->box.right  = x2;
            det_result->box.bottom = y2;
        }
    }

    return 0;
}

// int init_yolov8_model(const char *model_path, rknn_app_context_t *app_ctx)
int yolov8_detect_init(const char*                     model_path,
                       rknn_app_context_t*             app_ctx,
                       int                             obj_class_num,
                       float                           nms_thresh,
                       float                           box_thresh,
                       const std::vector<std::string>& labels)
{
    int          ret;
    int          model_len = 0;
    char*        model_data;
    rknn_context ctx = 0;

    // Set detection parameters
    app_ctx->obj_class_num = obj_class_num;
    app_ctx->nms_thresh    = nms_thresh;
    app_ctx->box_thresh    = box_thresh;
    app_ctx->labels        = labels;

    // Load model
    model_len = read_data_from_file(model_path, &model_data);
    if (model_data == NULL)
    {
        printf("load_model fail!\n");
        return -1;
    }

    ret = rknn_init(&ctx, model_data, model_len, 0);
    if (ret < 0)
    {
        printf("rknn_init fail! ret=%d\n", ret);
        return -1;
    }
    free(model_data);

    // Get Model Input Output Number
    rknn_input_output_num io_num;
    ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (ret != RKNN_SUCC)
    {
        printf("rknn_query fail! ret=%d\n", ret);
        return -1;
    }
    // printf("model input num: %d, output num: %d\n", io_num.n_input, io_num.n_output);

    // Get Model Input Info
    // printf("input tensors:\n");
    rknn_tensor_attr input_attrs[io_num.n_input];
    memset(input_attrs, 0, sizeof(input_attrs));
    for (int i = 0; i < io_num.n_input; i++)
    {
        input_attrs[i].index = i;
        ret                  = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
        if (ret != RKNN_SUCC)
        {
            printf("rknn_query fail! ret=%d\n", ret);
            return -1;
        }
        // dump_tensor_attr(&(input_attrs[i]));
    }

    // Get Model Output Info
    // printf("output tensors:\n");
    rknn_tensor_attr output_attrs[io_num.n_output];
    memset(output_attrs, 0, sizeof(output_attrs));
    for (int i = 0; i < io_num.n_output; i++)
    {
        output_attrs[i].index = i;
        ret                   = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
        if (ret != RKNN_SUCC)
        {
            printf("rknn_query fail! ret=%d\n", ret);
            return -1;
        }
        dump_tensor_attr(&(output_attrs[i]));
    }

    // Set to context
    app_ctx->rknn_ctx = ctx;

    // TODO
    if (output_attrs[0].qnt_type == RKNN_TENSOR_QNT_AFFINE_ASYMMETRIC && output_attrs[0].type == RKNN_TENSOR_UINT8)
    {
        app_ctx->is_quant = true;
    }
    else
    {
        app_ctx->is_quant = false;
    }

    app_ctx->io_num      = io_num;
    app_ctx->input_attrs = (rknn_tensor_attr*) malloc(io_num.n_input * sizeof(rknn_tensor_attr));
    memcpy(app_ctx->input_attrs, input_attrs, io_num.n_input * sizeof(rknn_tensor_attr));
    app_ctx->output_attrs = (rknn_tensor_attr*) malloc(io_num.n_output * sizeof(rknn_tensor_attr));
    memcpy(app_ctx->output_attrs, output_attrs, io_num.n_output * sizeof(rknn_tensor_attr));

    if (input_attrs[0].fmt == RKNN_TENSOR_NCHW)
    {
        printf("model is NCHW input fmt\n");
        app_ctx->model_channel = input_attrs[0].dims[2];
        app_ctx->model_height  = input_attrs[0].dims[1];
        app_ctx->model_width   = input_attrs[0].dims[0];
    }
    else
    {
        printf("model is NHWC input fmt\n");
        app_ctx->model_height  = input_attrs[0].dims[2];
        app_ctx->model_width   = input_attrs[0].dims[1];
        app_ctx->model_channel = input_attrs[0].dims[0];
    }
    /*
    printf("model input height=%d, width=%d, channel=%d\n",
           app_ctx->model_height, app_ctx->model_width, app_ctx->model_channel);
    */

    return 0;
}

int yolov8_detect_release(rknn_app_context_t* app_ctx)
{
    if (app_ctx->input_attrs != NULL)
    {
        free(app_ctx->input_attrs);
        app_ctx->input_attrs = NULL;
    }
    if (app_ctx->output_attrs != NULL)
    {
        free(app_ctx->output_attrs);
        app_ctx->output_attrs = NULL;
    }
    if (app_ctx->rknn_ctx != 0)
    {
        rknn_destroy(app_ctx->rknn_ctx);
        app_ctx->rknn_ctx = 0;
    }
    return 0;
}

int yolov8_detect_run(rknn_app_context_t* app_ctx, rga_buffer_t* input_buffer, detect_result_group_t* od_results)
{
    int         ret;
    rknn_input  inputs[app_ctx->io_num.n_input];
    rknn_output outputs[app_ctx->io_num.n_output];

    memset(od_results, 0x00, sizeof(*od_results));
    memset(inputs, 0, sizeof(inputs));
    memset(outputs, 0, sizeof(outputs));

    // Initialize letter buffer if not already initialized
    if (!app_ctx->letter_buffer.vir_addr)
    {
        app_ctx->letter_buffer.width    = app_ctx->model_width;
        app_ctx->letter_buffer.height   = app_ctx->model_height;
        app_ctx->letter_buffer.wstride  = (app_ctx->model_width + 15) & ~15;
        app_ctx->letter_buffer.hstride  = app_ctx->model_height;
        app_ctx->letter_buffer.format   = RK_FORMAT_RGB_888;
        app_ctx->letter_buffer.fd       = -1;
        app_ctx->letter_buffer.phy_addr = 0;

        size_t letter_buffer_size = app_ctx->letter_buffer.wstride * app_ctx->letter_buffer.hstride * 3;
        app_ctx->letter_buffer_data.resize(letter_buffer_size, 0);
        app_ctx->letter_buffer.vir_addr = app_ctx->letter_buffer_data.data();

        if (!app_ctx->letter_buffer.vir_addr)
        {
            printf("Failed to allocate letter buffer\n");
            return -1;
        }

        printf("Letter buffer initialized: width=%d, height=%d, wstride=%d, hstride=%d, format=%d\n",
               app_ctx->letter_buffer.width,
               app_ctx->letter_buffer.height,
               app_ctx->letter_buffer.wstride,
               app_ctx->letter_buffer.hstride,
               app_ctx->letter_buffer.format);
    }

    ret = letter_box(input_buffer, &app_ctx->letter_buffer, app_ctx->model_width);
    if (ret < 0)
    {
        printf("letter_box failed\n");
        return -1;
    }

    // Set Input Data
    inputs[0].index = 0;
    inputs[0].type  = RKNN_TENSOR_UINT8;
    inputs[0].fmt   = RKNN_TENSOR_NHWC;
    inputs[0].size  = app_ctx->model_width * app_ctx->model_height * app_ctx->model_channel;
    inputs[0].buf   = app_ctx->letter_buffer.vir_addr;

    ret = rknn_inputs_set(app_ctx->rknn_ctx, app_ctx->io_num.n_input, inputs);
    if (ret < 0)
    {
        printf("rknn_input_set fail! ret=%d\n", ret);
        return -1;
    }

    // Run
    ret = rknn_run(app_ctx->rknn_ctx, nullptr);
    if (ret < 0)
    {
        printf("rknn_run fail! ret=%d\n", ret);
        return -1;
    }

    // Get Output
    memset(outputs, 0, sizeof(outputs));
    for (int i = 0; i < app_ctx->io_num.n_output; i++)
    {
        outputs[i].index      = i;
        outputs[i].want_float = (!app_ctx->is_quant);
    }
    ret = rknn_outputs_get(app_ctx->rknn_ctx, app_ctx->io_num.n_output, outputs, NULL);
    if (ret < 0)
    {
        printf("rknn_outputs_get fail! ret=%d\n", ret);
        return -1;
    }

    // Post Process
    post_process(app_ctx, outputs, od_results);

    // Scale coordinates
    scale_coords(od_results, input_buffer->width, input_buffer->height, app_ctx->model_width);

    // Release rknn output
    rknn_outputs_release(app_ctx->rknn_ctx, app_ctx->io_num.n_output, outputs);

    return ret;
}

int yolov8_detect_run_raw(rknn_app_context_t* app_ctx, const FrameDataInfo& frame_info, detect_result_group_t* od_results)
{
    if (!app_ctx || !od_results)
    {
        printf("Invalid context or results pointer\n");
        return -1;
    }

    // Create input buffer for NV12 data
    rga_buffer_t input_buffer = {};
    input_buffer.width        = frame_info.width;
    input_buffer.height       = frame_info.height;
    input_buffer.wstride      = frame_info.wstride;
    input_buffer.hstride      = frame_info.hstride;
    input_buffer.format       = RK_FORMAT_YCbCr_420_SP;
    input_buffer.vir_addr     = frame_info.pBuf;
    input_buffer.fd           = -1;
    input_buffer.phy_addr     = 0;

    if (!input_buffer.vir_addr)
    {
        printf("Invalid input buffer address\n");
        return -1;
    }

    // Create BGR buffer for color conversion
    rga_buffer_t bgr_buffer = {};
    bgr_buffer.width        = frame_info.width;
    bgr_buffer.height       = frame_info.height;
    bgr_buffer.wstride      = (frame_info.width + 15) & ~15;
    bgr_buffer.hstride      = frame_info.height;
    bgr_buffer.format       = RK_FORMAT_RGB_888;
    bgr_buffer.fd           = -1;
    bgr_buffer.phy_addr     = 0;

    // Allocate BGR buffer memory
    size_t               bgr_buffer_size = bgr_buffer.wstride * bgr_buffer.hstride * 3;
    std::vector<uint8_t> bgr_buffer_data(bgr_buffer_size, 0);
    bgr_buffer.vir_addr = bgr_buffer_data.data();

    if (!bgr_buffer.vir_addr)
    {
        printf("Failed to allocate BGR buffer\n");
        return -1;
    }

    // Convert NV12 to BGR
    IM_STATUS status = imcvtcolor(input_buffer, bgr_buffer, input_buffer.format, bgr_buffer.format, IM_SYNC);
    if (status != IM_STATUS_SUCCESS && status != IM_STATUS_NOERROR)
    {
        printf("RGA color conversion failed: %d\n", status);
        return -1;
    }

    if (!app_ctx->letter_buffer.vir_addr)
    {
        app_ctx->letter_buffer.width    = app_ctx->model_width;
        app_ctx->letter_buffer.height   = app_ctx->model_height;
        app_ctx->letter_buffer.wstride  = (app_ctx->model_width + 15) & ~15;
        app_ctx->letter_buffer.hstride  = app_ctx->model_height;
        app_ctx->letter_buffer.format   = RK_FORMAT_RGB_888;
        app_ctx->letter_buffer.fd       = -1;
        app_ctx->letter_buffer.phy_addr = 0;

        size_t letter_buffer_size = app_ctx->letter_buffer.wstride * app_ctx->letter_buffer.hstride * 3;
        app_ctx->letter_buffer_data.resize(letter_buffer_size, 0);
        app_ctx->letter_buffer.vir_addr = app_ctx->letter_buffer_data.data();

        if (!app_ctx->letter_buffer.vir_addr)
        {
            printf("Failed to allocate letter buffer\n");
            return -1;
        }
    }

    // Run detection with BGR buffer
    return yolov8_detect_run(app_ctx, &bgr_buffer, od_results);
}