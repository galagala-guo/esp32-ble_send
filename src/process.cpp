#include "Arduino.h"
#include "process.h"

static QueueSetHandle_t g_xQueueSetProcess;

static QueueHandle_t g_xQueueADS1115;
//static QueueHandle_t g_xQueueNJW1159;
static QueueHandle_t g_xQueueFFT;

// static QueueHandle_t g_xQueueBtsend;
static QueueHandle_t g_xQueueBtreceive;

// static QueueHandle_t g_xQueueBlesend;
static QueueHandle_t g_xQueueBlereceive;

void ProcessAds1115Data()
{
    VOLTAGE_DataPacket_t VOLTAGE_DataPacket;
    // 从队列中取出数据包
    xQueueReceive(g_xQueueADS1115, &VOLTAGE_DataPacket, 0);
    // 根据类型判断是温度还是电压发送给ble从机和手机端
    switch (VOLTAGE_DataPacket.DataID)
    {
    case TYPE_TEMPERATURE: // 温度数据
        // ble发送数据
        ble_send(VOLTAGE_DataPacket.data, sizeof(VOLTAGE_DataPacket.data), CHAR_TEMPERATURE_UUID);
        // 蓝牙发送数据
        BT_send(VOLTAGE_DataPacket.data, sizeof(VOLTAGE_DataPacket.data));
        break;
    case TYPE_VOLTAGE: // 电压数据
        // ble发送数据
        ble_send(VOLTAGE_DataPacket.data, sizeof(VOLTAGE_DataPacket.data), CHAR_VOLTAGE_UUID);
        // 蓝牙发送数据
        BT_send(VOLTAGE_DataPacket.data, sizeof(VOLTAGE_DataPacket.data));
        break;
    default:
        break;
    }
}

void ProcessFFTData()
{
    FFT_DataPacket_t FFT_DataPacket;
    // 从队列中取出数据包
    xQueueReceive(g_xQueueADS1115, &FFT_DataPacket, 0);
    // ble发送数据
    ble_send(FFT_DataPacket.data, sizeof(FFT_DataPacket.data), CHAR_AUDIO_FFT_UUID);
    // 蓝牙发送数据
    BT_send(FFT_DataPacket.data, sizeof(FFT_DataPacket.data));
}

void ProcessBleReceiveData()
{
    BLE_DataPacket_t BLE_DataPacket;
    xQueueReceive(g_xQueueBlereceive, &BLE_DataPacket, 0);
    if (BLE_DataPacket.DataID == TYPE_AUDIO_VOLUME)
    {
        NJW1159_setVolume(0, BLE_DataPacket.data[0]);
    }
}

void ProcessBtReceiveData()
{
    BT_DataPacket_t BT_DataPacket;
    xQueueReceive(g_xQueueBlereceive, &BT_DataPacket, 0);
    //根据接收到的数据的id类型进行相应的处理
}

void process_init()
{

    g_xQueueSetProcess = xQueueCreateSet(PROCESS_QUEUE_LEN);

    g_xQueueADS1115 = GetQueueads1115();
    g_xQueueFFT = GetQueuefft();

    // g_xQueueBlesend = GetQueueblesend();
    g_xQueueBlereceive = GetQueueblereceive();

    // g_xQueueBtsend = GetQueueBTsend();
    g_xQueueBtreceive = GetQueueBTrecv();

    xQueueAddToSet(g_xQueueADS1115, g_xQueueSetProcess);
    xQueueAddToSet(g_xQueueFFT, g_xQueueSetProcess);

    // xQueueAddToSet(g_xQueueBlesend, g_xQueueSetProcess);
    xQueueAddToSet(g_xQueueBlereceive, g_xQueueSetProcess);

    // xQueueAddToSet(g_xQueueBtsend, g_xQueueSetProcess);
    xQueueAddToSet(g_xQueueBtreceive, g_xQueueSetProcess);
}

void process(void)
{
    QueueSetMemberHandle_t xQueueHandle;
    while (1)
    {
        xQueueHandle = xQueueSelectFromSet(g_xQueueSetProcess, portMAX_DELAY);
        if (xQueueHandle)
        {
            if (xQueueHandle == g_xQueueADS1115)
            {
                // 处理ads1115的数据
                ProcessAds1115Data();
            }
            else if (xQueueHandle == g_xQueueFFT)
            {
                // 处理fft的数据
                ProcessFFTData();
            }
            // else if (xQueueHandle == g_xQueueBlesend)
            // {
            //     // 处理低功耗蓝牙发送数据
            // }
            else if (xQueueHandle == g_xQueueBlereceive)
            {
                // 处理低功耗蓝牙接收数据
                ProcessBleReceiveData();
            }
            // else if (xQueueHandle == g_xQueueBtsend)
            // {
            //     // 处理蓝牙发送数据
            // }
            else if (xQueueHandle == g_xQueueBtreceive)
            {
                // 处理蓝牙接收数据
                ProcessBtReceiveData();
            }
        }
    }
}
