#include "Arduino.h"
#include "ble.h"
#include <BLEDevice.h>

BLEAddress *pServerAddress = nullptr;
BLEScan *pBLEScan = nullptr;
BLEClient *pClient = nullptr;
BLERemoteService *pRemoteService = nullptr;
BLERemoteCharacteristic *pTemperatureCharacteristic = nullptr;
BLERemoteCharacteristic *pVoltageCharacteristic = nullptr;
BLERemoteCharacteristic *pAudioFFTCharacteristic = nullptr;
BLERemoteCharacteristic *pAudioVolumeCharacteristic = nullptr;

static QueueHandle_t g_xQueueblesend;
static QueueHandle_t g_xQueueblereceive;

bool bleConnected = false;
bool bleConnecting = false;
bool bleScan = false;


void VolumeNotifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic,
                                  uint8_t *pData, size_t length, bool isNotify)
{
  BLE_DataPacket_t BLE_DataPacket;
  for (int i = 0; i < length; i++)
  {
    BLE_DataPacket.data[i] = pData[i];
  }
  xQueueSend(g_xQueueblereceive, &BLE_DataPacket, 0);
  BLE_DataPacket.DataID = TYPE_AUDIO_VOLUME;
}

class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient *pclient)
  {
    pclient->setMTU(83);
    Serial.println("BLE已连接到从设备");
    bleConnected = true;
  }
  void onDisconnect(BLEClient *pclient)
  {
    //delete pServerAddress;
    //pServerAddress = nullptr;
    delete pClient;
    pClient = nullptr;
    pRemoteService = nullptr;
    //pBLEScan = nullptr;
    pTemperatureCharacteristic = nullptr;
    pVoltageCharacteristic = nullptr;
    pAudioFFTCharacteristic = nullptr;
    pAudioVolumeCharacteristic = nullptr;
    Serial.println("BLE与从设备断开连接");
    bleConnected = false;
  }
};

// 连接到服务器
void connectToServer(BLEAddress address)
{
  if (bleConnecting || bleConnected)
    return;

  bleConnecting = true;
  Serial.print("连接从设备: ");
  Serial.println(address.toString().c_str());

  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());

  if (pClient->connect(address))
  {
    Serial.println("BLE连接成功");

    // 获取服务
    pRemoteService = pClient->getService(SERVICE_UUID);
    if (pRemoteService != nullptr)
    {
      // 获取特征
      pTemperatureCharacteristic = pRemoteService->getCharacteristic(CHAR_TEMPERATURE_UUID);
      pVoltageCharacteristic = pRemoteService->getCharacteristic(CHAR_VOLTAGE_UUID);
      pAudioFFTCharacteristic = pRemoteService->getCharacteristic(CHAR_AUDIO_FFT_UUID);
      pAudioVolumeCharacteristic = pRemoteService->getCharacteristic(CHAR_AUDIO_VOLUME_UUID);
      // 注册回调函数
      pAudioVolumeCharacteristic->registerForNotify(VolumeNotifyCallback);
      if (pTemperatureCharacteristic != nullptr && pVoltageCharacteristic != nullptr && pAudioFFTCharacteristic != nullptr && pAudioVolumeCharacteristic != nullptr)
      {
        Serial.println("发现服务和特征");
        bleConnected = true;
      }
      else
      {
        Serial.println("找不到特征");
        pClient->disconnect();
        pClient = nullptr;
      }
    }
    else
    {
      Serial.println("找不到服务");
      pClient->disconnect();
      pClient = nullptr;
    }
  }
  else
  {
    Serial.println("连接失败");
    pClient = nullptr;
  }

  bleConnecting = false;
}

// 扫描回调类
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    if (advertisedDevice.getName() == BLEServerName)
    {
      Serial.println("发现从设备，停止扫描并连接");
      BLEDevice::getScan()->stop();
      connectToServer(advertisedDevice.getAddress());
    }
  }
};

void ble_init(std::string client_name)
{
  g_xQueueblesend = xQueueCreate(bleSend_QUEUE_LEN, sizeof(BLE_DataPacket_t));
  g_xQueueblereceive = xQueueCreate(bleReceive_QUEUE_LEN, sizeof(BLE_DataPacket_t));
  // 初始化BLE
  BLEDevice::init(client_name);

  // 开始扫描
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
}

void ble_param_set(uint16_t Interval, uint16_t Window)
{
  pBLEScan->setInterval(Interval);//1349
  pBLEScan->setWindow(Window);//499
}

void ble_scan_enable()
{
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

QueueHandle_t GetQueueblesend(void)
{
  return g_xQueueblesend;
}

QueueHandle_t GetQueueblereceive(void)
{
  return g_xQueueblesend;
}

// BLERemoteCharacteristic *GetpTemperatureChar(void)
// {
//   return pTemperatureCharacteristic;
// }

// BLERemoteCharacteristic *GetpVoltageChar(void)
// {
//   return pVoltageCharacteristic;
// }

// BLERemoteCharacteristic *GetpAudioFFTChar(void)
// {
//   return pAudioFFTCharacteristic;
// }

void ble_send(uint8_t *data, size_t length, const char *uuid)
{
  BLERemoteCharacteristic *pCharacteristic = nullptr;
  if (pRemoteService != nullptr)
  {
    pCharacteristic = pRemoteService->getCharacteristic(uuid);
    pCharacteristic->writeValue(data, length);
  }
}