#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "weather.h"

Weather_Typedef Weather_State;//天气全局变量定义

void getWeather() 
{
    // 1. 检查 WiFi 是否还连着
    if (WiFi.status() == WL_CONNECTED) 
    {
        HTTPClient http;

        /*连接服务器*/
        String url = "http://api.seniverse.com/v3/weather/now.json?key=S-lO9bnn-doJ7qLet&location=wenZhou&language=zh-Hans&unit=c";//知心天气
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
            /*提取解析键*/
            JsonObject now = doc["results"][0]["now"];
            JsonObject location = doc["results"][0]["location"];
            /*解析成功的存入全局天气结构体*/
            strcpy(Weather_State.cityName,location["name"]);//城市名称
            strcpy(Weather_State.weatherText,now["text"]);//天气
            Weather_State.temperature=now["temperature"].as<int>();//气温
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
