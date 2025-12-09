#ifndef _ESP8266_H_
#define _ESP8266_H_





#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志

void ESP8266_Init(void);

void ESP8266_Clear(void);
void ESP8266_Send_DHT11_Data(double temp,double humi);
void ESP8266_Send_fan_Data(uint8_t fan);
void ESP8266_Send_fire_Data(uint8_t fan);
void ESP8266_Send_one_Data(uint8_t one);
void ESP8266_Send_two_Data(uint8_t two);
void ESP8266_Send_three_Data(uint8_t three);
void ESP8266_Send_all_Data(uint8_t all);
void extractJsonValue(const char *buffer, const char *key, char *value, size_t valueMaxLen);

const char *ESP8266_GetIPD(unsigned short timeOut);


#endif
