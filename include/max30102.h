#ifndef MAX30102_H
#define MAX30102_H

#include <Arduino.h>

//I2C地址
#define MAX30102_I2C_ADDR 0x57

/* MAX30102 全局变量 */
typedef struct
{
    int heartRate; // 心率（BPM）
    int spo2;      // 血氧百分比（0-100）
    bool valid;    // 数据是否有效
}MAX30102_Typedef;
extern MAX30102_Typedef MAX30102_State;

/**
 * @brief 初始化 MAX30102 传感器（基于 I2C 寄存器配置）
 *
 * 执行软复位，设置工作模式（SpO2），配置 LED 强度和 FIFO 指针。
 * 初始化过程中会使用串口打印状态信息以便调试。
 */
void MAX30102_Init(void);

/**
 * @brief 读取 FIFO 样本并更新心率与血氧（SpO2）
 *
 * 本函数从设备 FIFO 读取单个样本（18-bit），根据样本的通道
 *（红/红外交替）将数据写入内部缓存；基于红外信号进行峰值
 *检测计算 BPM，并在缓存样本足够时计算 SpO2。计算结果写入
 *`MAX30102_State`。
 */
void readMAX30102(void);

#endif
