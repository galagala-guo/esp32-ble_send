#include "Arduino.h"
// #include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <ADS1115.h>

Adafruit_ADS1115 ads;

SemaphoreHandle_t ADCSemaphore = NULL;
static QueueHandle_t g_xQueueads1115;

QueueHandle_t GetQueueads1115(void)
{
  return g_xQueueads1115;
}

// std::string formatToTwoDecimals(double num)
// {
//   std::ostringstream oss;
//   oss << std::fixed << std::setprecision(2) << num;
//   return oss.str();
// }

// void IRAM_ATTR ads1115Interrupt()
// {
//   BaseType_t xHigherPriorityTaskWoken = pdFALSE;

//   // 发送信号量通知任务
//   if (ADCSemaphore != NULL)
//   {
//     xSemaphoreGiveFromISR(ADCSemaphore, &xHigherPriorityTaskWoken);
//   }

//   if (xHigherPriorityTaskWoken)
//   {
//     portYIELD_FROM_ISR();
//   }
// }

void ads1115_init(void)
{
  // Start with default values
  uint16_t config =
      ADS1X15_REG_CONFIG_CQUE_1CONV |   // Set CQUE to any value other than
                                        // None so we can use it in RDY mode
      ADS1X15_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
      ADS1X15_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
      ADS1X15_REG_CONFIG_CMODE_TRAD;    // Traditional comparator (default val)
  Wire.begin(I2C_SDA, I2C_SCL);
  // 创建信号量
  // TaskHandle_t ADCSemaphore = xSemaphoreCreateBinary();
  // 创建队列
  g_xQueueads1115 = xQueueCreate(ADS1115_QUEUE_LEN, sizeof(VOLTAGE_DataPacket_t));
  // 配置中断引脚
  pinMode(ADS1115_RDY, INPUT_PULLUP);
  // attachInterrupt(digitalPinToInterrupt(ADS1115_RDY),
  //                 ads1115Interrupt, FALLING);
  // 初始化ads1115
  if (!ads.begin())
  {
    Serial.println("Failed to initialize ADS.");
  }
  // Write config register to the ADC
  ads.writeRegister(ADS1X15_REG_POINTER_CONFIG, config);
  // Set ALERT/RDY to RDY mode.
  ads.writeRegister(ADS1X15_REG_POINTER_HITHRESH, 0x8000);
  ads.writeRegister(ADS1X15_REG_POINTER_LOWTHRESH, 0x0000);
}

uint16_t read_voltage(void)
{
  // Start with default values
  uint16_t res;
  uint16_t config =
      ADS1X15_REG_CONFIG_CQUE_1CONV |   // Set CQUE to any value other than
                                        // None so we can use it in RDY mode
      ADS1X15_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
      ADS1X15_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
      ADS1X15_REG_CONFIG_CMODE_TRAD;    // Traditional comparator (default val)

  // Set 'start single-conversion' bit
  config |= ADS1X15_REG_CONFIG_OS_SINGLE;
  // Set channels
  config |= ADS1X15_REG_CONFIG_MUX_DIFF_1_3;
  // Write config register to the ADC
  ads.writeRegister(ADS1X15_REG_POINTER_CONFIG, config);
  // Set ALERT/RDY to RDY mode.
  ads.writeRegister(ADS1X15_REG_POINTER_HITHRESH, 0x8000);
  ads.writeRegister(ADS1X15_REG_POINTER_LOWTHRESH, 0x0000);

  // Wait for the conversion to complete
  while ((ads.readRegister(ADS1X15_REG_POINTER_CONFIG) & 0x8000) != 0)
    ;
  res = ads.readRegister(ADS1X15_REG_POINTER_CONVERT);
  return res;
}

uint16_t read_temperature(void)
{
  // Start with default values
  uint16_t res;
  uint16_t config =
      ADS1X15_REG_CONFIG_CQUE_1CONV |   // Set CQUE to any value other than
                                        // None so we can use it in RDY mode
      ADS1X15_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
      ADS1X15_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
      ADS1X15_REG_CONFIG_CMODE_TRAD;    // Traditional comparator (default val)

  // Set 'start single-conversion' bit
  config |= ADS1X15_REG_CONFIG_OS_SINGLE;
  // Set channels
  config |= ADS1X15_REG_CONFIG_MUX_DIFF_0_3;
  // Write config register to the ADC
  ads.writeRegister(ADS1X15_REG_POINTER_CONFIG, config);
  // Set ALERT/RDY to RDY mode.
  ads.writeRegister(ADS1X15_REG_POINTER_HITHRESH, 0x8000);
  ads.writeRegister(ADS1X15_REG_POINTER_LOWTHRESH, 0x0000);

  // Wait for the conversion to complete
  while ((ads.readRegister(ADS1X15_REG_POINTER_CONFIG) & 0x8000) != 0)
    ;
  res = ads.readRegister(ADS1X15_REG_POINTER_CONVERT);
  return res;
}

// void ads1115_work(void)
// {
//   int16_t res;
//   float voltage = 0;
//   uint8_t i = 0;
//   VOLTAGE_DataPacket_t VOLTAGE_DataPacket;
//   while (1)
//   {
//     if (xSemaphoreTake(ADCSemaphore, portMAX_DELAY) == pdTRUE)
//     {
//       res = ads.readRegister(ADS1X15_REG_POINTER_CONVERT);
//       VOLTAGE_DataPacket.data[i] = (res >> 8) & 0xFF;
//       VOLTAGE_DataPacket.data[i + 1] = res & 0xFF;
//       VOLTAGE_DataPacket.DataID = TYPE_VOLTAGE;
//       voltage = res * MULTIPLIER / 1000.0f * 3.0f;
//       i = i + 2;
//     }
//     if (i >= 20)
//     {
//       xQueueSend(g_xQueueads1115, &VOLTAGE_DataPacket, 0);
//       i = 0;
//       memset(VOLTAGE_DataPacket.data, 0, 32);
//       VOLTAGE_DataPacket.DataID = 0;
//     }
//     vTaskDelay(50);
//   }
// }

void ads1115_work(void)
{
  int16_t res;
  // float voltage = 0;
  uint8_t i = 0;
  bool switch_flag = 0;
  VOLTAGE_DataPacket_t VOLTAGE_DataPacket;
  TEMPERATURE_DataPacket_t TEMPERATURE_DataPacket;
  while (1)
  {
    if (!switch_flag)
    {
      res = read_voltage();
      VOLTAGE_DataPacket.data[i] = (res >> 8) & 0xFF;
      VOLTAGE_DataPacket.data[i + 1] = res & 0xFF;
      VOLTAGE_DataPacket.DataID = TYPE_VOLTAGE;
      i = i + 2;
      if (i >= 20)
      {
        xQueueSend(g_xQueueads1115, &VOLTAGE_DataPacket, 0);
        i = 0;
        switch_flag = 1;
        memset(VOLTAGE_DataPacket.data, 0, 32);
        VOLTAGE_DataPacket.DataID = 0;
      }
    }
    else if (switch_flag)
    {
      res = read_temperature();
      TEMPERATURE_DataPacket.data[i] = (res >> 8) & 0xFF;
      TEMPERATURE_DataPacket.data[i + 1] = res & 0xFF;
      TEMPERATURE_DataPacket.DataID = TYPE_TEMPERATURE;
      xQueueSend(g_xQueueads1115, &TEMPERATURE_DataPacket, 0);
      memset(TEMPERATURE_DataPacket.data, 0, 2);
      TEMPERATURE_DataPacket.DataID = 0;
      switch_flag = 0;
    }
    vTaskDelay(50);
  }
}