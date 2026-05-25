#include <Arduino.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <Wire.h>
#include "mywifi.h"//WiFi判断
#include "graphic.h"
#include "weather.h"
#include "AudioSpeaker.h"
#include "mytime.h"
#include "dth11.h"
#include "max30102.h"
#include "battery.h"

/*SU-03T1的串口引脚*/
#define SU03T_RX_PIN 42
#define SU03T_TX_PIN 41
HardwareSerial SerialSU03T(2);//创建ESP32的2号硬件串口，取名为SerialSU03T

/*OLED的I2C引脚*/
#define I2C_SDA 21
#define I2C_SCL 20
/*OLED的尺寸*/
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
/*u8g2对象接管OLED*/
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE,I2C_SCL, I2C_SDA);//R0,屏幕旋转0度，U8G2接管硬件I2C

/*状态机定义*/
typedef enum {
  STATE_IDLE,          // 空闲 
  STATE_SHOW_TIME,     // 显示时间
  STATE_SHOW_WEATHER,  // 显示天气
  STATE_SHOW_HR,       // 显示心率
  STATE_SHOW_HR_RESULT, // 显示心率结果
  STATE_SHOW_ENV,      // 显示温湿度
  STATE_SHOW_BATTERY   // 显示电量
} SystemState;
SystemState currentState = STATE_IDLE; //当前状态，初始空闲
/*记录上一次的状态*/
SystemState lastState = STATE_SHOW_WEATHER;
static int hrSampleCount = 0;
static int hrSampleSum = 0;
static int hrFinalAverage = 0;

/*setup初始化*/
void setup() 
{
    Serial.begin(115200);//USB串口初始化

    SerialSU03T.begin(9600, SERIAL_8N1, SU03T_RX_PIN, SU03T_TX_PIN);//SU-03T1串口初始化

    u8g2.begin();//初始化OLED屏幕驱动
    u8g2.setBusClock(400000);//设置硬件I2C的时钟频率为400kHz
    u8g2.clearBuffer();//清空屏幕缓冲区
    wifi_connect_start();//WiFi连接(断掉自动重连)

    setenv("TZ", "CST-8", 1);//东八区时间
    tzset();//立即生效

    DTH11_Init();//DHT11初始化
    MAX30102_Init(); // MAX30102 初始化
    GetBattery_Init();   //ADC初始化
}
/*语音解析*/
void parseVoiceCommand() 
{
  if (Serial.available()) 
  {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();//去掉字符串头尾的空格、换行、回车等多余垃圾字符

    if (cmd == "AA") currentState = STATE_SHOW_TIME;//查看时间
    else if (cmd == "BB") currentState = STATE_SHOW_WEATHER;//查看天气
    else if (cmd == "CC") currentState = STATE_SHOW_HR;//测量心率
    else if (cmd == "DD") currentState = STATE_SHOW_ENV;//测量温湿度
    else if (cmd == "EE") currentState = STATE_SHOW_BATTERY;//查看电量
  }
}
/*状态机执行*/
void runStateMachine() {
  switch (currentState) {
    /*空闲状态*/
    case STATE_IDLE:
    {
        static unsigned long blinkTimer;//闪烁文字计时
        static bool showText = true;//文字闪烁状态切换
        static unsigned long idleRefresh;//空闲界面刷新计时
        /*刚进入空闲状态获取计时*/
         if (currentState != lastState) {
            blinkTimer = millis();
            idleRefresh = millis();
            showText = true;
            u8g2.clearBuffer();
        }
        /*WIFI连接每700ms闪烁文字*/
        if (wifi_connected)
        {
            if (millis() - blinkTimer >= 700) 
            {
                blinkTimer = millis();
                showText = !showText;
                u8g2.clearBuffer();
                if (showText) 
                {
                    drawWifiConnected2(u8g2); // 显示文字 + 图标 + 表情
                } else {
                    drawWifiConnected1(u8g2); // 只显示图标 + 表情（文字消失）
                }
                u8g2.sendBuffer();
            }
        } else /*WIFI未连接，每秒刷新屏幕，减少发烫*/
        {
            if (millis() - idleRefresh >= 1000) 
            {
                idleRefresh = millis();
                u8g2.clearBuffer();
                drawWifiConnecting1(u8g2);
                u8g2.sendBuffer();
            }
        }
      break;
    }
    /*显示时间状态*/
    case STATE_SHOW_TIME:
    {
      static unsigned long start_time_time;
        /*刚进入状态的时候获取计时*/
        if (currentState != lastState) 
        {
          start_time_time = millis();
          if (wifi_connected)
          {
            /*绘制时间(包含WiFi已连接图标)*/
            u8g2.clearBuffer();
            getTime();//获取时间数据
            drawTime(u8g2);
            u8g2.sendBuffer();
          }
          else
          {
            u8g2.clearBuffer();
            drawPreaseConnect(u8g2);//请连接WiFi文字
            drawWifiConnecting2(u8g2);//WiFi未连接图标
            u8g2.sendBuffer();
          }
        }

        if (millis() - start_time_time >= 4000)//停留四秒
        {
            currentState = STATE_IDLE;
        }
      break;
    }
    /*显示天气状态*/
    case STATE_SHOW_WEATHER:
    {
        static unsigned long start_time_werther;
        /*刚进入状态的时候获取计时*/
        if (currentState != lastState) 
        {
          start_time_werther = millis();
          if (wifi_connected)
          {
            /*绘制天气(包含WiFi已连接图标)*/
            u8g2.clearBuffer();
            getWeather();//HTTP获取天气数据
            drawWeather(u8g2);
            u8g2.sendBuffer();
          }
          else
          {
            u8g2.clearBuffer();
            drawPreaseConnect(u8g2);//请连接WiFi文字
            drawWifiConnecting2(u8g2);//WiFi未连接图标
            u8g2.sendBuffer();
          }
        }

        if (millis() - start_time_werther >= 4000)//停留四秒
        {
            currentState = STATE_IDLE;
        }
      break;
    }
    /*显示心率（正在检测）*/
    case STATE_SHOW_HR:
    {
      static unsigned long start_time_hr;
      /* 刚进入状态时启动测量并记录起始时间 */
      if (currentState != lastState)
      {
        start_time_hr = millis();
        hrSampleCount = 0;
        hrSampleSum = 0;
        hrFinalAverage = 0;
        // 首次立即读取一次
        readMAX30102();
        u8g2.clearBuffer();
        drawHeartSpO2(u8g2, 10, false, 0);
        u8g2.sendBuffer();
      }
      // 在该状态持续期间，定期读取并更新显示
      // 每次循环都读取一次（readMAX30102 内部会根据 FIFO 返回数据）
      readMAX30102();
      if (MAX30102_State.valid && MAX30102_State.heartRate > 0)
      {
        hrSampleCount++;//有效的心率次数
        hrSampleSum += MAX30102_State.heartRate;//心率数值，全部加起来的总和
      }
      unsigned long elapsed = millis() - start_time_hr;
      int remaining = 10 - (int)(elapsed / 1000);
      if (remaining < 0) remaining = 0;
      if(wifi_connected)
      {
      u8g2.clearBuffer();
      drawHeartSpO2(u8g2, remaining, false, 0);
      u8g2.sendBuffer();
      }
      else
      {
      u8g2.clearBuffer();
      drawHeartSpO2(u8g2, remaining, false, 0);
      drawWifiConnecting2(u8g2);//WiFi未连接图标
      u8g2.sendBuffer();
      }

      // 测量 10 秒后进入结果显示状态
      if (elapsed >= 10000)
      {
        if (hrSampleCount > 0) {
          hrFinalAverage = (hrSampleSum + hrSampleCount / 2) / hrSampleCount;
        } else {
          hrFinalAverage = MAX30102_State.heartRate;
        }
        currentState = STATE_SHOW_HR_RESULT;
      }
      break;
    }
    /*显示心率结果*/
    case STATE_SHOW_HR_RESULT:
    {
      static unsigned long result_start_ms;
      static int displayValue;
      if (currentState != lastState)
      {
        result_start_ms = millis();
        displayValue = hrFinalAverage > 0 ? hrFinalAverage : 0;
      }

      unsigned long elapsed = millis() - result_start_ms;
      bool showRate = ((elapsed / 500) % 2) == 0; // 0.5s 闪烁一次

      if(wifi_connected)
      {
      u8g2.clearBuffer();
      drawHeartSpO2(u8g2, 0, true, displayValue, showRate);
      u8g2.sendBuffer();
      }
      else
      {
      u8g2.clearBuffer();
      drawHeartSpO2(u8g2, 0, true, displayValue, showRate);
      drawWifiConnecting2(u8g2);//WiFi未连接图标
      u8g2.sendBuffer();
      }

      if (elapsed >= 2000) {
        currentState = STATE_IDLE;
      }
      break;
    }
    /*显示温湿度*/
    case STATE_SHOW_ENV:
    {
        static unsigned long start_time_dth11;
        /*刚进入状态的时候获取计时*/
        if (currentState != lastState) 
        {
            start_time_dth11 = millis();
            if (wifi_connected)
            {
            /*绘制温湿度*/
            u8g2.clearBuffer();
            readDHT11();//读取DHT11数据
            drawDTH11(u8g2);//绘制温湿度
            u8g2.sendBuffer();
            }
            else
            {
            /*绘制温湿度*/
            u8g2.clearBuffer();
            readDHT11();//读取DHT11数据
            drawDTH11(u8g2);//绘制温湿度
            drawWifiConnecting2(u8g2);//WiFi未连接图标
            u8g2.sendBuffer();
            }
        }
        if (millis() - start_time_dth11 >= 4000)//停留四秒
        {
            currentState = STATE_IDLE;
        }
      break;
    }
    /*查看电量*/
    case STATE_SHOW_BATTERY:
    {
      static unsigned long start_time_battery;

      if (currentState != lastState) {
          start_time_battery = millis();
      }

      GetCur_Power();

      // 实时刷新屏幕
      u8g2.clearBuffer();

      if (wifi_connected) 
      {
        drawBatteryLevel(u8g2);
        u8g2.sendBuffer();
      } else 
      {
        drawBatteryLevel(u8g2);
        drawWifiConnecting2(u8g2);//WiFi未连接图标
        u8g2.sendBuffer();
      }


      // 4秒后退出
      if (millis() - start_time_battery >= 4000) {
          currentState = STATE_IDLE;
      }
      break;
      }
  }
}

void loop() 
{

    wifi_task_loop();//判断WiFi状态
    parseVoiceCommand();//监听语音指令
    SystemState prevState = currentState; // 保存进入本次循环时的状态
    runStateMachine();//执行状态机

    lastState = prevState; // 记录上一次（进入本次循环时）的状态，供下一次循环比较

}

