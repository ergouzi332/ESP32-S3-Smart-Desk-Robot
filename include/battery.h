#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>

// 只有两个函数
void GetBattery_Init(void);
void GetCur_Power(void);

// 外部调用的最终电量
extern uint16_t CurBattery;

#endif