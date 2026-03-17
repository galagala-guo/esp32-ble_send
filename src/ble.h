#ifndef ble_h
#define ble_h

#define BLEServerName "ESP32_ble" // 服务端名称

#define SERVICE_UUID "3558e6ce-a193-4e70-ba29-ea130c09570e"
#define CHAR_TEMPERATURE_UUID "2870d7bf-2a54-4af2-ae67-5e8a83566c23"
#define CHAR_VOLTAGE_UUID "23e0b592-15ec-4049-a72b-6bad6018ffdf"
#define CHAR_AUDIO_FFT_UUID "89280797-1d8b-44d5-868a-3f11ef8b0ec0"
#define CHAR_AUDIO_VOLUME_UUID "e99773d3-120d-4e7c-ac31-0f41af470719"

#define bleSend_QUEUE_LEN 20
#define bleReceive_QUEUE_LEN 5


#define TYPE_AUDIO_VOLUME 4
// enum BLEDataType
// {
//   //TYPE_NONE = 0,
//   //TYPE_TEMPERATURE = 1,
//   //TYPE_VOLTAGE = 2,
//   //TYPE_AUDIO_FFT = 3,
//   //TYPE_AUDIO_VOLUME = 4
// };

// 数据包结构
struct BLE_DataPacket_t
{
  uint8_t DataID;
  //BLEDataType type;
  uint8_t data[1];
};

// QueueHandle_t GetQueueblesend_cpp(void);
QueueHandle_t GetQueueblesend(void);
QueueHandle_t GetQueueblereceive(void);
void ble_init(std::string client_name);
void ble_send(uint8_t *data, size_t length, const char *uuid);

#endif