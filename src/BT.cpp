#include "Arduino.h"
#include "BluetoothSerial.h"
#include "BT.h"

BluetoothSerial SerialBT;

static QueueHandle_t g_xQueueBTrecv;
static QueueHandle_t g_xQueueBTsend;

BT_DataPacket_t BT_DataPacket;

void RecvData(const uint8_t *buffer, size_t size)
{
//蓝牙接收逻辑
}

QueueHandle_t GetQueueBTrecv(void)
{
    return g_xQueueBTrecv;
}

QueueHandle_t GetQueueBTsend(void)
{
    return g_xQueueBTsend;
}

// BluetoothSerial GetBluetoothSerial(void)
// {
//     return SerialBT;
// }

void BT_init(String bt_name)
{
    //蓝牙初始化
    SerialBT.onData(RecvData);
    SerialBT.begin(bt_name);
    g_xQueueBTrecv = xQueueCreate(BTrecv_QUEUE_LEN, sizeof(BT_DataPacket));
    g_xQueueBTsend = xQueueCreate(BTsend_QUEUE_LEN, sizeof(BT_DataPacket));
}

void BT_send(uint8_t *data,size_t length)
{
    if(SerialBT.hasClient())
    {
        SerialBT.write(data,length);
    }
}
