#include <Arduino.h>
#include "driver/i2s.h"
#include "FFT.h"

// #include <esp_err.h>
// I2S_BITS_PER_SAMPLE_32BIT,
I2S_DIN_INFO i2sDin;
static QueueHandle_t g_xQueuefft;

QueueHandle_t GetQueuefft(void)
{
  return g_xQueuefft;
}

void I2S_Init()
{
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = 0,
      .dma_buf_count = 16,
      .dma_buf_len = 32,
      .use_apll = false,
#if ESP_ARDUINO_VERSION_MAJOR >= 2
      .fixed_mclk = 0,
#endif
  };

  i2s_pin_config_t i2s_pin_config = {
#if ESP_ARDUINO_VERSION_MAJOR >= 2
      .mck_io_num = ((ESP32_I2S_MASTER == 1) ? I2S_MCK : I2S_PIN_NO_CHANGE),
#endif
      .bck_io_num = 33,                  // BIT CLOCK(TO PIN 8 BCK IN PCM1808)
      .ws_io_num = 26,                   // LR CLOCK(TO PIN 7 LRCK IN PCM1808)
      .data_out_num = I2S_PIN_NO_CHANGE, //(NOT USED)
      .data_in_num = 34                  // DATA IN(FROM PIN 9 DOUT IN PCM1808)
  };

  if (ESP_OK == i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL))
  {
    Serial.println("install i2s driver ");
    // while(1);
    // return false;
  }
  if (ESP_OK == i2s_set_pin(I2S_NUM_0, &i2s_pin_config))
  {
    Serial.println("i2s set pin ");
    // while(1);
    // return false;
  }
  i2s_zero_dma_buffer(I2S_NUM_0);
}

// int I2S_Read(int32_t *data, int numData)
// {
//     size_t bytes_read;
//     return i2s_read(I2S_NUM_0, data, numData, &bytes_read, portMAX_DELAY);
// }

bool i2sRead(PI2S_DIN_INFO pDin)
{
  size_t ByteRead = 0;
  esp_err_t Result;
  bool fResult = false;
  Result = i2s_read(I2S_NUM_0, &pDin->bI2SBuf[0], BLOCK_SIZE, &ByteRead, portMAX_DELAY);
  if (Result == ESP_OK && ByteRead == BLOCK_SIZE)
  {
    int iPos = BLOCK_SIZE * pDin->iWaveBufWritePtr;
    memcpy(&pDin->bWaveBuf[iPos], &pDin->bI2SBuf[0], BLOCK_SIZE);
    pDin->iWaveBufWritePtr++;
    if (pDin->iWaveBufWritePtr >= DMA_DESC_NUM)
    {
      pDin->iWaveBufWritePtr = 0;
      fResult = true;
    }
  }
  else
  {
    Serial.printf("i2s_read() : Result[%d] ByteRead[%d]\n", Result, ByteRead);
  }
  return fResult;
}

void fftDataSet(void)
{
  int16_t *psAdrs = (int16_t *)&i2sDin.bWaveBuf[0];
  int32_t *plAdrs = (int32_t *)&i2sDin.bWaveBuf[0];
  double dData[2] = {0.0, 0.0};
  for (int i = 0, j = 0; i < FFT_SIZE; i++, j += HARD_CHANNELS)
  {
    if (BIT_PER_SAMPLE == 16)
    {
      dData[0] = (double)((double)psAdrs[j + 0] / (double)0x7FFF);
      dData[1] = (double)((double)psAdrs[j + 1] / (double)0x7FFF);
    }
    else
    {
      dData[0] = (double)((double)plAdrs[j + 0] / (double)0x7FFFFFFF);
      dData[1] = (double)((double)plAdrs[j + 1] / (double)0x7FFFFFFF);
    }
    for (int c = 0; c < DISP_CHANNELS; c++)
    {
      i2sDin.ch[c].dReal[i] = dData[c];
      i2sDin.ch[c].dImag[i] = 0.0;
    }
  }
}

double ComputeFreqBand(double *pdFreqBuf, int iFFTHalfSize, int iBand, int iBands, double *pdXScale)
{
  int iPoint1 = ceilf(pdXScale[iBand]);
  int iPoint2 = floorf(pdXScale[iBand + 1]);
  double dData = 0;
  double dResult = 0;
  if (iPoint2 < iPoint1)
  {
    dData += pdFreqBuf[iPoint2] * (pdXScale[iBand + 1] - pdXScale[iBand]);
  }
  else
  {
    if (iPoint1 > 0)
    {
      dData += pdFreqBuf[iPoint1 - 1] * ((double)iPoint1 - pdXScale[iBand]);
    }
    for (; iPoint1 < iPoint2; iPoint1++)
    {
      dData += pdFreqBuf[iPoint1];
    }
    if (iPoint2 < iFFTHalfSize)
    {
      dData += pdFreqBuf[iPoint2] * (pdXScale[iBand + 1] - (double)iPoint2);
    }
  }
  if (dData != 0)
  {
    dResult = 10 * log10(dData * dData);
  }
  return dResult;
}

bool MakeLogBand(double *pdFreqBuf, int iFFTHalfSize, int *piBandData, int iDbRange, int iRange, int iBands)
{
  static int iOldBands;
  static double *pdXScale;
  if (iBands <= 0)
  {
    return false;
  }
  if (iOldBands != iBands)
  {
    size_t stSize = sizeof(*pdXScale) * (iBands + 1);
    void *p = (void *)pdXScale;
    if (p == NULL)
    {
      p = malloc(stSize);
    }
    else
    {
      p = realloc(p, stSize);
    }
    if (p == NULL)
      return false;
    pdXScale = (double *)p;
    for (int i = 0; i <= iBands; i++)
    {
      pdXScale[i] = pow(iFFTHalfSize, (double)i / (double)iBands);
      if (pdFreqBuf != NULL && piBandData == NULL && iDbRange == 0)
      {
        int iSamplingRate = iRange;
        pdFreqBuf[i] = iSamplingRate / ((iFFTHalfSize * 2) / pdXScale[i]);
      }
    }
    iOldBands = iBands;
    if (pdFreqBuf != NULL && piBandData == NULL && iDbRange == 0)
    {
      return true;
    }
  }
  for (int i = 0; i < iBands; i++)
  {
    double dData = ComputeFreqBand(pdFreqBuf, iFFTHalfSize, i, iBands, pdXScale);
    dData = (dData / (double)iDbRange) * (double)iRange;
    piBandData[i] = min(max(dData, (double)0), (double)iRange);
  }
  return true;
}

void UpdatePeakBand(int *piBandData, int *piBandPeak, struct timeval *ptvOld, int iBands)
{
  struct timeval tvNow;
  struct timeval tvDiff;
  gettimeofday(&tvNow, NULL);
  timersub(&tvNow, ptvOld, &tvDiff);
  int32_t lCheckTime = tvDiff.tv_sec * 1000 + tvDiff.tv_usec / 1000;
  for (int i = 0; i < iBands; i++)
  {
    if (piBandData[i] > piBandPeak[i])
    {
      piBandPeak[i] = piBandData[i];
    }
    if (lCheckTime > 500)
    {
      *ptvOld = tvNow;
      piBandPeak[i] = 0;
    }
  }
}

bool bandsInit(int iBands)
{
  if (iBands <= 0)
  {
    return false;
  }
  if (i2sDin.iBands == iBands)
  {
    return true;
  }
  i2sDin.iBands = iBands;
  void *p = (void *)i2sDin.dBandFreq;
  size_t stSize = sizeof(*i2sDin.dBandFreq) * (i2sDin.iBands + 1);
  if (p == NULL)
  {
    p = malloc(stSize);
  }
  else
  {
    p = realloc(p, stSize);
  }
  if (p == NULL)
    return false;
  i2sDin.dBandFreq = (double *)p;
  if (!MakeLogBand(&i2sDin.dBandFreq[0], FFT_HALF_SIZE, NULL, 0, SAMPLE_RATE, i2sDin.iBands))
  {
    return false;
  }
  for (int c = 0; c < DISP_CHANNELS; c++)
  {
    p = (void *)i2sDin.ch[c].iBandData;
    stSize = sizeof(*i2sDin.ch[c].iBandData) * i2sDin.iBands;
    if (p == NULL)
    {
      p = malloc(stSize);
    }
    else
    {
      p = realloc(p, stSize);
    }
    if (p == NULL)
      return false;
    i2sDin.ch[c].iBandData = (int *)p;
    for (int i = 0; i < i2sDin.iBands; i++)
    {
      i2sDin.ch[c].iBandData[i] = 0;
    }

    p = (void *)i2sDin.ch[c].iBandPeak;
    stSize = sizeof(*i2sDin.ch[c].iBandPeak) * i2sDin.iBands;
    if (p == NULL)
    {
      p = malloc(stSize);
    }
    else
    {
      p = realloc(p, stSize);
    }
    if (p == NULL)
      return false;
    i2sDin.ch[c].iBandPeak = (int *)p;
    for (int i = 0; i < i2sDin.iBands; i++)
    {
      i2sDin.ch[c].iBandPeak[i] = 0;
    }
  }
  return true;
}

bool fftInit(void)
{

  // 创建队列
  g_xQueuefft = xQueueCreate(FFT_QUEUE_LEN, sizeof(FFT_DataPacket_t));

  void *(*xmalloc)(unsigned int) = (ESP.getFreePsram() == 0 ? &malloc : &ps_malloc);
  size_t stSize = sizeof(double) * FFT_SIZE;
  for (int c = 0; c < DISP_CHANNELS; c++)
  {
    i2sDin.ch[c].dReal = (double *)xmalloc(stSize);
    i2sDin.ch[c].dImag = (double *)xmalloc(stSize);
    if (i2sDin.ch[c].dReal == NULL || i2sDin.ch[c].dImag == NULL)
    {
      return false;
    }
    for (int i = 0; i < FFT_SIZE; i++)
    {
      i2sDin.ch[c].dReal[i] = 0.0;
      i2sDin.ch[c].dImag[i] = 0.0;
    }
    i2sDin.ch[c].FFT = ArduinoFFT<double>(&i2sDin.ch[c].dReal[0], &i2sDin.ch[c].dImag[0], FFT_SIZE, SAMPLE_RATE);
  }
  return true;
}

void fft_work(uint8_t ibands)
{
  FFT_DataPacket_t FFT_DataPacket;
  while (1)
  {
    memset(FFT_DataPacket.data, 0, ibands);
    //FFT_DataPacket.type = TYPE_NONE;
    if (i2sRead(&i2sDin) == true)
    {
      fftDataSet();
      for (int c = 0; c < DISP_CHANNELS; c++)
      {
        i2sDin.ch[c].FFT.dcRemoval();
        i2sDin.ch[c].FFT.windowing(FFT_WIN_TYP_HANN, FFT_FORWARD);
        i2sDin.ch[c].FFT.compute(FFT_FORWARD);
        i2sDin.ch[c].FFT.complexToMagnitude();
        MakeLogBand(&i2sDin.ch[c].dReal[1], FFT_HALF_SIZE, &i2sDin.ch[c].iBandData[0], DB_RANGE, DATA_RANGE, i2sDin.iBands);
        UpdatePeakBand(&i2sDin.ch[c].iBandData[0], &i2sDin.ch[c].iBandPeak[0], &i2sDin.ch[c].tv, i2sDin.iBands);
        for (int i = 0; i < BANDS; i++)
        {
          FFT_DataPacket.data[i] = i2sDin.ch[c].iBandPeak[i];
        }
        //FFT_DataPacket.type = TYPE_AUDIO_FFT;
      }
      xQueueSend(g_xQueuefft, &FFT_DataPacket, 0);
    }
    vTaskDelay(30);
  }
}
