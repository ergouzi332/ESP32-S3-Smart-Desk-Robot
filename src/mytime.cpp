#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "weather.h"
#include "mytime.h"
#include <time.h>

Time_Typedef Time_State;//时间全局变量定义
long long currentTime;//时间戳

void getTime() 
{
    //检查WiFi是否还连着
    if (WiFi.status() == WL_CONNECTED) 
    {
        HTTPClient http;

        /*连接服务器*/
        String url = "https://f.m.suning.com/api/ct.do";//苏宁
        http.begin(url); //开始连接
        int httpCode = http.GET();//发起 GET 请求
        if (httpCode > 0)//检查服务器返回状态码
        {
            String payload = http.getString();
            JsonDocument  doc;//自动开辟内存        
            DeserializationError error = deserializeJson(doc, payload);//解析成对象树
            
            if (error)//解析失败 
            {
                Serial.print("JSON 解析失败: ");
                Serial.println(error.f_str());
                return;
            }
            currentTime=doc["currentTime"].as<long long>();//时间戳
            /*时间戳转换*/
            struct tm timeinfo;
            time_t now = currentTime / 1000;
            localtime_r(&now, &timeinfo);
            Time_State.year = timeinfo.tm_year + 1900;
            Time_State.month = timeinfo.tm_mon + 1;
            Time_State.day = timeinfo.tm_mday;
            Time_State.hour = timeinfo.tm_hour;
            Time_State.minute = timeinfo.tm_min;
        } 
        else 
        {
            Serial.print("HTTP 请求失败，错误码：");
            Serial.println(http.errorToString(httpCode).c_str());
        }
        http.end();//释放资源
    } 
    else 
    {
        Serial.println("WiFi 未连接，无法获取数据");
    }
}

