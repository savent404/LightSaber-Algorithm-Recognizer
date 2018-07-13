#include "event.h"

#ifndef EVENT_LAZY_MODE
#define EVENT_LAZY_MODE 1
#endif

typedef int logic_t;

/**
 * @brief hasMainPower 检测某个浮点数是否是该数组中的绝对值最大值
 * @param pdata 浮点数数组
 * @param index 指定检测的最大值的标号
 * @param maxNum 数组大小
 * @return 0 不是绝对值最大值 / 1 是绝对值最大值
 */
inline static int hasMainPower(float* pdata, int index, int maxNum)
{
    for (int i = 0; i < maxNum; i++) {
        if (index == i)
            continue;
        if (fabsf(pdata[i] >= fabsf(pdata[index])))
            return 0;
    }
    return 1;
}

inline static int u32Greater(uint32_t a, uint32_t b)
{
    return a > b;
}
inline static int u32Litter(uint32_t a, uint32_t b)
{
    return a <= b;
}
inline static int fGreater(float a, float b)
{
    return a > b;
}
inline static int fLitter(float a, float b)
{
    return a <= b;
}

Event_t getEvent(Event_Instance_t* instance, float* a, float* g)
{
    static logic_t bActived[Event_None] = { 0 };
    static uint32_t u32Duration[Event_None] = { 0 };
    static int spin_counter = 0;
    static int spin_circle_counter = 0;
    int i = 0;
    float fScalar_acc = 0;
    float fScalar_g_yandz = 0;
    float fScalar_g = 0;

    uint32_t timeNow = getSystemTime_ms();
    Event_t event = Event_None;

    /** Get scalar of acc */
    for (; i < 3; i++) {
        fScalar_acc += a[i] * a[i];
        fScalar_g += g[i] * g[i];
        if (i != 0)
            fScalar_g_yandz += g[i] * g[i];
    }
    fScalar_acc = sqrtf(fScalar_acc) - 1.0;
    fScalar_g = sqrtf(fScalar_g);
    fScalar_g_yandz = sqrtf(fScalar_g_yandz);

    /** Check Spin */
    if (bActived[Event_Swing] != 0
        && fGreater((timeNow - u32Duration[Event_Swing] - spin_counter * instance->spin_circle_time / 4.0f) * fScalar_g_yandz,
               90.0f)) {
        spin_counter++;
    }
    if (bActived[Event_Swing] != 0
        && bActived[Event_Spin] == 0
        && u32Greater(spin_counter, instance->spin_counter)
        && fGreater(fScalar_g_yandz, instance->spin_threshold_start)
        && fGreater(timeNow - u32Duration[Event_Swing], instance->spin_circle_time / fScalar_g_yandz)) {
        bActived[Event_Spin] = 1;
        u32Duration[Event_Spin] = timeNow;
        event = Event_Spin;
        goto RETURN;
    } else if (bActived[Event_Swing] != 0
        && bActived[Event_Spin] != 0) {
        if (fLitter(fScalar_g_yandz, instance->spin_threshold_end)) {
            bActived[Event_Spin] = 0;
            spin_counter = 1;
            spin_circle_counter = 0;
        } else {
            if (u32Greater(timeNow - u32Duration[Event_Spin], instance->spin_circle_time * (spin_circle_counter + 1))) {
                spin_circle_counter++;
                event = Event_Spin;
                goto RETURN;
            }
        }
    } else if (bActived[Event_Swing] == 0
        && u32Greater(timeNow - u32Duration[Event_Swing], instance->spin_circle_time * 4)) {
        spin_counter = 0;
        spin_circle_counter = 0;
    }

    /** Check Stab */
    if (!bActived[Event_Stab]
        && fGreater(fScalar_acc, instance->stab_threshold_start)
        && hasMainPower(a, 0, 3)) {
        bActived[Event_Stab] = 1;
        u32Duration[Event_Stab] = timeNow;
    } else if (bActived[Event_Stab]
        && fLitter(fScalar_acc, instance->stab_threshold_end)) {
        bActived[Event_Stab] = 0;
        if (u32Greater(timeNow - u32Duration[Event_Stab], instance->stab_window)) {
            u32Duration[Event_Stab] = timeNow;
            event = Event_Stab;
            goto RETURN;
        }
    } else if (bActived[Event_Stab]
        && fGreater(fScalar_g, 300.0f)) {
        bActived[Event_Stab] = 0;
    }

    /** Check Clash */
    if (bActived[Event_Clash] == 0
        && fScalar_acc > instance->clash_threshold_start
        && !hasMainPower(a, 0, 3)) {
        bActived[Event_Clash] = 1;
        u32Duration[Event_Clash] = timeNow;
    } else if (bActived[Event_Clash] != 0
        && fLitter(fScalar_acc, instance->clash_threshold_end)) {
        bActived[Event_Clash] = 0;
        if (u32Litter(timeNow - u32Duration[Event_Clash], instance->clash_window)) {
            u32Duration[Event_Clash] = timeNow;
            event = Event_Clash;
            goto RETURN;
        }
    }

    /** Check Slash */
    if (bActived[Event_Slash] == 0
        && fGreater(fScalar_acc, instance->slash_threshold_start)
        && hasMainPower(a, 0, 3) == 0) {
        bActived[Event_Slash] = 1;
        u32Duration[Event_Slash] = timeNow;
    } else if (bActived[Event_Slash] != 0
        && fLitter(fScalar_acc, instance->slash_threshold_end)) {
        bActived[Event_Slash] = 0;
        if (u32Litter(timeNow - u32Duration[Event_Slash], instance->slash_window)
            && u32Greater(timeNow - u32Duration[Event_Slash], instance->clash_window)) {
            u32Duration[Event_Slash] = timeNow;
            event = Event_Slash;
            goto RETURN;
        }
    }

    /** Check Swing */
    if (bActived[Event_Swing] == 0
        && fGreater(fScalar_acc, instance->swing_threshold_start)) {
        bActived[Event_Swing] = 1;
        u32Duration[Event_Swing] = timeNow;
    } else if (bActived[Event_Swing] != 0
        && fLitter(fScalar_acc, instance->swing_threshold_end)) {
        bActived[Event_Swing] = 0;
        if (u32Greater(timeNow - u32Duration[Event_Swing], instance->swing_window)) {
            u32Duration[Event_Swing] = timeNow;
            event = Event_Swing;
            goto RETURN;
        }
    }

RETURN:
#if EVENT_LAZY_MODE == 1
#endif
    return event;
}

const char* getEventName(Event_t event)
{
    switch (event) {
    case Event_Swing:
        return "Swing";
    case Event_Slash:
        return "Slash";
    case Event_Clash:
        return "Clash";
    case Event_Stab:
        return "Stab";
    case Event_Spin:
        return "Spin";
    case Event_None:
        return "None";
    default:
        return "Unknow";
    }
}
