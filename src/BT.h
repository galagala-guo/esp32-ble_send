#ifndef BT_h
#define BT_h

#define BTrecv_QUEUE_LEN 5
#define BTsend_QUEUE_LEN 10
#define BT_Name "ESP32_BT"

#define TYPE_AUDIO_VOLUME 4

// enum BTDataType {
//   //TYPE_NONE = 0,
//   //TYPE_TEMPERATURE = 1,
//   //TYPE_VOLTAGE = 2,
//   //TYPE_AUDIO_FFT = 3,
//   //TYPE_AUDIO_VOLUME = 4
// };

// 数据包结构
struct BT_DataPacket_t {
  uint8_t DataID; 
  //BTDataType type;
  uint8_t data[1];
};

void BT_init(String bt_name);
void BT_send(uint8_t *data,size_t length);
//void BT_work(void);
QueueHandle_t GetQueueBTrecv(void);
QueueHandle_t GetQueueBTsend(void);
//BluetoothSerial GetBluetoothSerial(void);

#endif