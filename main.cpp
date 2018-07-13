#include "MPU6050.h"
#include "mbed.h"

#include "event.h"
#include <stdint.h>

MPU6050 mpu;
InterruptIn inter(PC_9);

bool refresh = false;

void func()
{
    refresh = true;
}

int main()
{
    // Interrupte when PC_9 rising
    inter.rise(func);

    // MPU6050 Init
    printf("Initializing I2C devices...\r\n");
    mpu.initialize();
    // Enable Interrupt Pin on MPU6050
    mpu.setIntDataReadyEnabled(true);
    // Interrupt Pin would be HIGH in 50ms if no-one read Ready Data
    mpu.setInterruptLatchClear(true);
    // OutputFreq = 8Mhz / (rate + 1) -> 1.6KHz
    mpu.setRate(4);
    // GyroRange = ±500°/s
    mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_500);
    // AccelRange = ±8G
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);


    printf("Testing device connections...\r\n");
    printf(mpu.testConnection() ? "MPU6050 connection successful\r\n" : "MPU6050 connection failed\r\n");

    // Read data once and drop it.
    if (mpu.getIntDataReadyStatus()) {
        int16_t a[6];
        mpu.getMotion6(a, a + 1, a + 2, a + 3, a + 4, a + 5);
    }

    // Event instance init
    Event_Instance_t instance;

    instance.swing_threshold_start = 0.2f;
    instance.swing_threshold_end = 0.2f;
    instance.swing_window = 50;

    instance.slash_threshold_start = 2.2f;
    instance.slash_threshold_end = 2.2f;
    instance.slash_window = 30;

    instance.clash_threshold_start = 2.2f;
    instance.clash_threshold_end = 2.2f;
    instance.clash_window = 5;

    instance.stab_threshold_start = 0.8f;
    instance.stab_threshold_end = 0.8f;
    instance.stab_window = 50;

    instance.spin_threshold_start = 50;
    instance.spin_threshold_end = 50;
    instance.spin_circle_time = 300;
    instance.spin_counter = 4;

    while (1) {
        static int count = 0;
        static int event_cnt[Event_None] = { 0 };

        // New Data
        if (refresh) {
            int16_t a[6];
            float b[6];

            // clear flag
            refresh = false;
            // count Inc
            count++;
            // get data(type:float)
            mpu.getMotion6(a, a + 1, a + 2, a + 3, a + 4, a + 5);
            for (int i = 0; i < 3; i++) {
                b[i] = float(a[i]) * 16.0f / 0x10000;
                b[i + 3] = float(a[i + 3]) * 1000.0f / 0x10000;
            }
            // get Event
            Event_t event = getEvent(&instance, b, b + 3);
            // Output Event
            if (event != Event_None) {
                event_cnt[event]++;
                printf("[%s]:\t%d\r\n", getEventName(event), event_cnt[event]);
            }
            
        }
        // Output Sample Freq
        if (count > 2048) {
            uint32_t time = getSystemTime_ms();
            static uint32_t lastTime = 0;
            printf("Hz:%.2f\r\n", float(count) * 1000.0f / (time - lastTime));
            lastTime = time;
            count = 0;
        }
    }
}
