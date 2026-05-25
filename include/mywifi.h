#ifndef MYWIFI_H
#define MYWIFI_H


extern uint8_t wifi_connected; //全局变量：WiFi的连接状态

void wifi_task_loop(void);
void wifi_connect_start(void);

#endif