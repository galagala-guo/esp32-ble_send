#ifndef NJW1159_h
#define NJW1159_h

#define NJW1159_ADDRESS 0x00

#define NJW1159_LATCH 12 // 将LATCH定义为数字引脚14
#define NJW1159_CLOCK 13 // 将CLOCK定义为数字引脚13
#define NJW1159_DATA 14  // 将DATA定义为数字引脚12
#define njw1159_QUEUE_LEN 5

// 时序常量（单位：微秒，满足文档最小要求）
#define T_SU 1    // DATA建立时间（最小0.8μs）
#define T_CWH 1   // CLOCK高电平宽度（最小0.8μs）
#define T_CWL 1   // CLOCK低电平宽度（最小0.8μs）
#define T_LATCH 2 // LATCH高电平宽度（最小1.6μs）

#define TYPE_AUDIO_VOLUME 4
// #define MONO 1
// #define SETREO 2
// enum NJW1159DataType {
//   TYPE_NONE = 0,
//   TYPE_TEMPERATURE = 1,
//   TYPE_VOLTAGE = 2,
//   TYPE_AUDIO_FFT = 3,
//   TYPE_AUDIO_VOLUME = 4
// };
// 数据包结构
struct VOLUME_DataPacket_t
{
  uint8_t DataID;
  // NJW1159DataType type;
  uint8_t data[1];
};

void NJW1159_init(void);
void NJW1159_setVolume(uint8_t channel, uint8_t volume);
void njw1159_work(void);

#endif