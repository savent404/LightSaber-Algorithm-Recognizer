#include "event.h"
#include "mbed.h"

static Timer timer;
static bool fInit = false;

extern "C" uint32_t getSystemTime_ms(void)
{
    if (fInit == false)
    {
        fInit = true;
        timer.start();
    }

    return timer.read_ms();
}
