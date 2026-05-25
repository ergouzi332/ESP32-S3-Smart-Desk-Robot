#ifndef MYTIME_H
#define MYTIME_H

void getTime();

typedef struct
{
    int year;//年
    int month;//月
    int day;//日
    int hour;//时
    int minute;//分
}Time_Typedef;
extern Time_Typedef Time_State;


#endif