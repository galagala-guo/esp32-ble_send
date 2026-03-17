#ifndef FFT_h
#define FFT_h
#include <driver/i2s.h>
#include <arduinoFFT.h>

#define ESP32_I2S_MASTER 1 // 1:ESP32 MASTER other:ESP32 SLAVE
#define HARD_CHANNELS 2    // 1:LEFT(MONO) 2:RIGHT&LEFT(STEREO)
#define SAMPLE_RATE 32000
#define BIT_PER_SAMPLE 32 // 16:一个采样数据占16位； 32：一个采样数据占32位；
#define BLOCK_SIZE 256
#define DMA_FRAME_NUM (BLOCK_SIZE / (BIT_PER_SAMPLE / 8) / HARD_CHANNELS)
#define DMA_DESC_NUM 16
#define WAVE_SIZE (BLOCK_SIZE * DMA_DESC_NUM)
#define FFT_SIZE (WAVE_SIZE / (BIT_PER_SAMPLE / 8) / HARD_CHANNELS)
#define FFT_HALF_SIZE (FFT_SIZE / 2)
#define DISP_CHANNELS 1 // 1:LEFT(MONO) 2:RIGHT&LEFT(STEREO)
#define BANDS 8
#define DB_RANGE 45
#define DATA_RANGE 100
#define ZERO_DB 42.797582 // 42.797582 = 10 * log10( 138 * 138 )
#define DB_RANGE_VALUE 99

#define I2S_DIN 34
#define I2S_BCK 33
#define I2S_LRCK 26
#define I2S_MCK 0

#define FFT_QUEUE_LEN 10

#define TYPE_AUDIO_FFT 3

// enum FFTDataType {
//   TYPE_NONE = 0,
//   TYPE_TEMPERATURE = 1,
//   TYPE_VOLTAGE = 2,
//   TYPE_AUDIO_FFT = 3,
//   TYPE_AUDIO_VOLUME = 4
// };

// 数据包结构
struct FFT_DataPacket_t {
  uint8_t DataID; 
  //FFTDataType type;
  uint8_t data[BANDS];
};


typedef struct TAG_I2S_DIN_INFO
{
    uint8_t bI2SBuf[BLOCK_SIZE];
    int iWaveBufWritePtr;
    uint8_t bWaveBuf[WAVE_SIZE];
    int iBands;
    double *dBandFreq;
    struct
    {
        ArduinoFFT<double> FFT;
        double *dReal;
        double *dImag;
        int *iBandData;
        int *iBandPeak;
        struct timeval tv;
    } ch[DISP_CHANNELS];
} I2S_DIN_INFO, *PI2S_DIN_INFO;

void I2S_Init();
bool bandsInit( int iBands );
bool fftInit(void);
void fft_work(uint8_t ibands);
QueueHandle_t GetQueuefft(void);
#endif
