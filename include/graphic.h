#ifndef GRAPHIC_H
#define GRAPHIC_H
#include <U8g2lib.h>

void drawWifiConnected1(U8G2_SSD1306_128X64_NONAME_F_HW_I2C &u8g2);//已连接1
void drawWifiConnected2(U8G2_SSD1306_128X64_NONAME_F_HW_I2C &u8g2);//已连接2
void drawWifiConnecting1(U8G2_SSD1306_128X64_NONAME_F_HW_I2C &u8g2);//未连接1
void  drawWifiConnecting2(U8G2_SSD1306_128X64_NONAME_F_HW_I2C &u8g2);//未连接2
void drawWeather(U8G2_SSD1306_128X64_NONAME_F_HW_I2C &u8g2);//天气
void drawTime(U8G2_SSD1306_128X64_NONAME_F_HW_I2C &u8g2);//时间
void drawDTH11(U8G2_SSD1306_128X64_NONAME_F_HW_I2C &u8g2);//温湿度
void drawPreaseConnect(U8G2_SSD1306_128X64_NONAME_F_HW_I2C &u8g2);//字体:请连接WiFi
void drawHeartSpO2(U8G2_SSD1306_128X64_NONAME_F_HW_I2C &u8g2, int countdownSeconds, bool showFinal, int animatedHeartRate, bool showHeartRate = true); // 显示心率与倒计时
void drawBatteryLevel(U8G2_SSD1306_128X64_NONAME_F_HW_I2C &u8g2);//电量

#endif
