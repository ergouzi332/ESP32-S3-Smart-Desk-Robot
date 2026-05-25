#include <Arduino.h>
#include <WiFi.h>
#include "mywifi.h"

uint8_t wifi_connected=0;//全局变量

void wifi_task_loop(void)//判断热点连接
{
    if (WiFi.status() == WL_CONNECTED)
    {
        wifi_connected = 1;
    }
    else
    {
        wifi_connected = 0;
    }
}

void wifi_connect_start(void)//热点连接函数
{
    WiFi.mode(WIFI_STA);          //连接别人WiFi
    WiFi.setAutoReconnect(true); // 开启底层自动重连
    WiFi.persistent(true);       // 将配置保存在 flash 中
    WiFi.begin("Xiaomi 14", "20041119");
}