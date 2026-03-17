#include "Arduino.h"
#include <NJW1159.h>
static QueueHandle_t g_xQueuenjw1159;


QueueHandle_t GetQueuenjw1159(void)
{
  return g_xQueuenjw1159;
}

// 初始化函数：设置引脚模式并初始化状态
void NJW1159_init() {
  pinMode(NJW1159_DATA, OUTPUT);
  pinMode(NJW1159_CLOCK, OUTPUT);
  pinMode(NJW1159_LATCH, OUTPUT);
  
  digitalWrite(NJW1159_LATCH, LOW);
  digitalWrite(NJW1159_CLOCK, LOW);
  digitalWrite(NJW1159_DATA, LOW);
  g_xQueuenjw1159 = xQueueCreate(njw1159_QUEUE_LEN, sizeof(VOLUME_DataPacket_t));
}

void sendData(uint16_t data) {
  digitalWrite(NJW1159_LATCH, LOW);  // 开始传输
  for (int i = 15; i >= 0; i--) {  // MSB先发
    digitalWrite(NJW1159_DATA, (data >> i) & 0x01);  // 设置DATA位
    delayMicroseconds(T_SU);        // 数据建立时间
    digitalWrite(NJW1159_CLOCK, HIGH);
    delayMicroseconds(T_CWH);       // CLOCK高电平宽度
    digitalWrite(NJW1159_CLOCK, LOW);
    delayMicroseconds(T_CWL);       // CLOCK低电平宽度
  }
  digitalWrite(NJW1159_LATCH, HIGH);   // 锁存数据
  delayMicroseconds(T_LATCH);
  digitalWrite(NJW1159_LATCH, LOW);
}

void NJW1159_setVolume(uint8_t channel,uint8_t volume) {
  // 构建16位数据
  uint16_t data = 0;
  uint8_t volBits = (volume <= 95) ? volume : 0x7F;  // 0-95为衰减值，95以上为静音(0x7F)
  data |= (volBits << 9);   // D15-D9: 7位音量值
  data |= (0 << 8);         // D8: Don't care（固定0）
  data |= (channel << 4);   // D7-D4: 声道选择（0=左, 1=右）
  data |= (NJW1159_ADDRESS & 0x03); // D3-D0: 芯片地址
  
  sendData(data);
}

// 静音函数
void NJW1159_setMute( uint8_t channel) {
  NJW1159_setVolume( channel, 0x7F);  // 0x7F对应静音（D15-D9=1111111）
}

void njw1159_work(void)
{
  VOLUME_DataPacket_t VOLUME_DataPacket;
  while(1)
  {
    xQueueReceive(g_xQueuenjw1159, &VOLUME_DataPacket, portMAX_DELAY);
    NJW1159_setVolume(0,VOLUME_DataPacket.data[0]);
    NJW1159_setVolume(1,VOLUME_DataPacket.data[0]);
  }
}