#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  获取ms级时间
 * @return 从开机到现在的时间(单位:ms)
 */
extern uint32_t getSystemTime_ms(void);

typedef enum {
    Event_Swing = 0,
    Event_Slash = 1,
    Event_Clash = 2,
    Event_Stab = 3,
    Event_Spin = 4,
    Event_None = 5,
} Event_t;

typedef struct {
    /** General */
    uint32_t sample_interval;
    /** Event:Swing */
    float swing_threshold_start; // 启动检测的阈值
    float swing_threshold_end; // 结束检测的阈值通常与Start值相同
    uint32_t swing_window; // 检查窗口，当大于该时间才判断为swing

    /** Event:Slash */
    float slash_threshold_start;
    float slash_threshold_end;
    uint32_t slash_window;

    /** Event:Clash */
    float clash_threshold_start;
    float clash_threshold_end;
    uint32_t clash_window;

    /** Event:Stab */
    float stab_threshold_start;
    float stab_threshold_end;
    uint32_t stab_window;

    /** Event:Spin */
    int spin_counter;
    int spin_circle_time;
    float spin_threshold_start;
    float spin_threshold_end;
} Event_Instance_t;

/**
 * @brief  getEvent 获取一个动作事件
 * @param  instance Event识别的实例,包括了配置参数以及缓存变量
 * @param  a 加速度计返回的3个轴向值,分别为x,y,z 单位(与地心引力的比值)
 * @param  g 角速度计返回的3个轴向值,分别为x,y,z 单位(°/s)
 * @return 触发的事件
 */
Event_t getEvent(Event_Instance_t *instance, float *a, float* g);


/**
 * @brief getEventName 获取事件名
 * @param event 事件ID
 * @return 事件名的字符串指针
 */
const char* getEventName(Event_t event);

#ifdef __cplusplus
}
#endif
