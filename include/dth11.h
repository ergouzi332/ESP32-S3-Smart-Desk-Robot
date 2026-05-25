#ifndef DTH11_H
#define DTH11_H
#include <DHT.h> 

/*温湿度全局变量*/
typedef struct
{
    float temperature;//温度
    float humidity;//湿度
}DTH11_Typedef;
extern DTH11_Typedef DTH11_State;
/*DTH11对象全局变量*/
extern DHT dht11;

void DTH11_Init(void);
void readDHT11(void);

#endif
