#include "Arduino.h"
#include "process.h"

void process_task(void *pt);
void ads1115_task(void *pt);
void fft_task(void *pt);

void setup()
{
    Serial.begin(115200); /* prepare for possible serial debug */
    ads1115_init();

    // NJW1159_init();

    // fftInit();
    // I2S_Init();
    // bandsInit(BANDS);

    // ble_init(BLEServerName);

    // BT_init(BT_Name);

    // process_init();

    // xTaskCreatePinnedToCore(process_task, "process_task", 1024 * 10, NULL, 3, NULL, 1);//创建集中处理任务
    // xTaskCreatePinnedToCore(fft_task, "fft_task", 1024 * 10, NULL, 1, NULL, 1);//创建fft处理任务
    // xTaskCreatePinnedToCore(ads1115_task, "ads1115_task", 1024 * 10, NULL, 1, NULL, 1);//创建ads1115处理任务
}

void loop()
{
}

void process_task(void *pt)
{
    process();
}

void fft_task(void *pt)
{
    fft_work(BANDS);
}

void ads1115_task(void *pt)
{
    ads1115_work();
}
