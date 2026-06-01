#include "Battery.h"
#include <Arduino.h>

#define BATTERY_PIN 4

// 内部变量
float  Battery_Value;
uint16_t temp;
uint32_t temp1 = 0;
uint8_t  Battery_num = 0;

// 对外输出最终平均电量
uint16_t CurBattery = 0;

// 内部ADC读取
static uint16_t Get_ADC(void)
{
    return analogRead(BATTERY_PIN);
}

// 内部获取单次电量
static uint16_t GetBattery(void)
{
    Battery_Value = (3.3 * 4 * Get_ADC() / 4096) * 100 - 300;

    if(Battery_Value < 0)// 限幅：低于3.0V=0%，高于4.2V=100%
        Battery_Value = 0;
    if(Battery_Value > 120) // 4.2V对应计算值为120
        Battery_Value = 100;

    // 线性映射到 0~100%
    return (uint16_t)(Battery_Value * 100 / 120);
}


//初始化
void GetBattery_Init(void)
{
    analogReadResolution(12);
}

//获取平均电量（100次采样）
void GetCur_Power(void)
{
    if(++Battery_num <= 100)
    {
        temp = GetBattery();
        temp1 += temp;
    }
    else
    {
        CurBattery = (uint16_t)(temp1 / 100);
        temp1 = 0;
        Battery_num = 0;
    }
}
