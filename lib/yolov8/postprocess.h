#ifndef _RKNN_YOLOV8_DEMO_POSTPROCESS_H_
#define _RKNN_YOLOV8_DEMO_POSTPROCESS_H_

#include "rknn_api.h"

#include <stdint.h>
#include <vector>

#define OBJ_NAME_MAX_SIZE 64
#define OBJ_NUMB_MAX_SIZE 128

typedef struct _BOX_RECT
{
    int left;
    int right;
    int top;
    int bottom;
} BOX_RECT;

typedef struct
{
    char     name[OBJ_NAME_MAX_SIZE];
    BOX_RECT box;
    float    prop;
    int      cls_id;
} detect_result_t;

typedef struct
{
    int             id;
    int             count;
    detect_result_t results[OBJ_NUMB_MAX_SIZE];
} detect_result_group_t;

int  init_post_process();
void deinit_post_process();
int  post_process(rknn_app_context_t* app_ctx, void* outputs, detect_result_group_t* od_results);

#endif  //_RKNN_YOLOV8_DEMO_POSTPROCESS_H_
