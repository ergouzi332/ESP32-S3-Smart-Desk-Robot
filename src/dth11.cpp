#include "dth11.h"
#include <Arduino.h>
#include <DHT.h>

#define DHT_PIN 18//DATA引脚
#define DHT_TYPE DHT11
DTH11_Typedef DTH11_State;//DTH11全局变量定义  
DHT dht11(DHT_PIN, DHT_TYPE);

void DTH11_Init(void) 
{

    dht11.begin();
}

void readDHT11(void) 
{
    DTH11_State.temperature = dht11.readTemperature();
    DTH11_State.humidity = dht11.readHumidity();
}