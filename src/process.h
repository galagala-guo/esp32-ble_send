#ifndef process_h
#define process_h

#include "ADS1115.h"
#include "ble.h"
#include "BT.h"
#include "FFT.h"
#include "NJW1159.h"

#define PROCESS_QUEUE_LEN (ADS1115_QUEUE_LEN + bleReceive_QUEUE_LEN + BTrecv_QUEUE_LEN + FFT_QUEUE_LEN)

// enum PROCESSDataType
// {
//     TYPE_TEMPERATURE = 1,
//     TYPE_VOLTAGE = 2,
//     TYPE_AUDIO_FFT = 3
// };

void process_init(void);
void process(void);

#endif