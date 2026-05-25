#include "AudioSpeaker.h"
#include <HTTPClient.h>



size_t speaker_play(const int16_t* data, size_t size) {
    size_t bytes_written = 0;
    i2s_write(SPEAKER_I2S_PORT, data, size, &bytes_written, portMAX_DELAY);
    return bytes_written;
}
esp_err_t init_speaker_module(uint32_t sample_rate) 
{
    /*I2S配置结构体*/
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),//播放声音
        .sample_rate = sample_rate,//音频采样率
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,//标准 16 位音频
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,//输出
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,//I2S协议，数据高位优先
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,//中断优先级
        .dma_buf_count = 8,//DMA缓冲区数量
        .dma_buf_len = 128,//每个缓冲区长度
        .use_apll = false,//不使用高精度时钟
        .tx_desc_auto_clear = true //无数据时自动清空DMA
    };

    // 2. 引脚绑定：直接使用上面 define 的宏
    i2s_pin_config_t pin_config = 
    {
        .bck_io_num = SPEAKER_I2S_BCK,//时钟
        .ws_io_num = SPEAKER_I2S_WS,//声道切换
        .data_out_num = SPEAKER_I2S_DO, //数据输出
        .data_in_num = I2S_PIN_NO_CHANGE//不使用输入
    };

    // 3. 安装驱动
    esp_err_t err = i2s_driver_install(SPEAKER_I2S_PORT, &i2s_config, 0, NULL);
    if (err == ESP_OK) {
        err = i2s_set_pin(SPEAKER_I2S_PORT, &pin_config);
    }
    return err;
}

