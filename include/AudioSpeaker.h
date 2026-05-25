#ifndef AUDIO_SPEAKER_H
#define AUDIO_SPEAKER_H
#include <Arduino.h>
#include "driver/i2s.h"

/*引脚定义*/
#define SPEAKER_I2S_BCK  16
#define SPEAKER_I2S_WS   15
#define SPEAKER_I2S_DO   17
#define SPEAKER_I2S_PORT I2S_NUM_1 // 使用端口1，避开麦克风的端口0

/* 函数声明 */
esp_err_t init_speaker_module(uint32_t sample_rate);
size_t speaker_play(const int16_t* data, size_t size);


#endif
