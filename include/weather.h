#ifndef WEATHER_H
#define WEATHER_H

typedef struct
{
    char cityName[10];
    char weatherText[10];
    int temperature;
}Weather_Typedef;

extern Weather_Typedef Weather_State;

void getWeather();

#endif