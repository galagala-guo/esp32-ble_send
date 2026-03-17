#ifndef ADS1115_h
#define ADS1115_h

#define I2C_SDA 8  // 将SDA更改为数字引脚8
#define I2C_SCL 18 // 将SCL更改为数字引脚9
#define ADS1115_RDY 27
#define ADS1115_QUEUE_LEN 10
#define MULTIPLIER 0.1875F

#define TYPE_TEMPERATURE 1
#define TYPE_VOLTAGE 2

// enum ADS1115DataType {
//   TYPE_NONE = 0,
//   TYPE_TEMPERATURE = 1,
//   TYPE_VOLTAGE = 2,
//   TYPE_AUDIO_FFT = 3,
//   TYPE_AUDIO_VOLUME = 4
// };

// 数据包结构
struct VOLTAGE_DataPacket_t {
  uint8_t DataID; 
  //ADS1115DataType type;
  uint8_t data[32];//一个数2字节，总共打包16个浮点数
};

struct TEMPERATURE_DataPacket_t {
  uint8_t DataID; 
  //ADS1115DataType type;
  uint8_t data[2];//一个数2字节，总共打包16个浮点数
};

void ads1115_init(void);
void ads1115_work(void);
QueueHandle_t GetQueueads1115(void);

#endif