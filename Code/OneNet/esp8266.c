#include "stm32f10x.h"
#include "Delay.h"
#include "usart.h"
#include "Fire.h"
#include <string.h>
#include <stdio.h>

#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志

#define ESP8266_WIFI_INFO		"AT+CWJAP=\"rcan\",\"96527866\"\r\n"

#define ESP8266_MQTTUS_INFO		"AT+MQTTUSERCFG=0,1,\"cabinet\",\"oay0gSchgn\",\"version=2018-10-31&res=products%2Foay0gSchgn%2Fdevices%2Fcabinet&et=1916285142&method=md5&sign=iE9rAZ8KU8zpHjSLQDvPUg%3D%3D\",0,0,\"\"\r\n"

#define ESP8266_ONENET_INFO		"AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,1\r\n"  	//新版OneNET地址

unsigned char esp8266_buf[256];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;

const char* pubtopic="$sys/oay0gSchgn/cabinet/thing/property/post";
const char* pubtopic2="$sys/oay0gSchgn/cabinet/thing/property/set"; //订阅

//==========================================================
//	函数名称：	ESP8266_Clear
//
//	函数功能：	清空缓存
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_Clear(void)
{

	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;

}

//==========================================================
//	函数名称：	ESP8266_WaitRecive
//
//	函数功能：	等待接收完成
//
//	入口参数：	无
//
//	返回参数：	REV_OK-接收完成		REV_WAIT-接收超时未完成
//
//	说明：		循环调用检测是否接收完成
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

	if(esp8266_cnt == 0) 							//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return REV_WAIT;
		
	if(esp8266_cnt == esp8266_cntPre)				//如果上一次的值和这次相同，则说明接收完毕
	{
		esp8266_cnt = 0;							//清0接收计数
			
		return REV_OK;								//返回接收完成标志
	}
		
	esp8266_cntPre = esp8266_cnt;					//置为相同
	
	return REV_WAIT;								//返回接收未完成标志

}

//==========================================================
//	函数名称：	ESP8266_SendCmd
//
//	函数功能：	发送命令
//
//	入口参数：	cmd：命令
//				res：需要检查的返回指令
//
//	返回参数：	0-成功	1-失败
//
//	说明：		
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res)
{
	
	unsigned char timeOut = 200;

	Usart_SendString(USART2, (unsigned char *)cmd, strlen((const char *)cmd));

	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)							//如果收到数据
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)		//如果检索到关键词
			{
				ESP8266_Clear();									//清空缓存
				
				return 0;
			}
		}
		
		Delay_ms(10);
	}
	
	return 1;

}

//==========================================================
//	函数名称：	ESP8266_SendData
//
//	函数功能：	发送数据
//
//	入口参数：	temp：温度值
//				      humi：湿度值
//				      adcx：光照度//后面自己加
//
//	返回参数：	无
//
//	说明：		
//==========================================================
// 发送数据到OneNET平台

//==========================================================
//	数据（串口调试用）
/*
AT

AT+CWMODE=1

AT+CWJAP="CMCC-CC","67824366"

AT+MQTTUSERCFG=0,1,"mqtt1","V0uTuio971","version=2018-10-31&res=products%2FV0uTuio971%2Fdevices%2Fmqtt1&et=2810295937&method=md5&sign=28AeB5C1zIV22nZ8HCeAUQ%3D%3D",0,0,""

AT+MQTTCONN=0,"mqtts.heclouds.com",1883,1


//温度湿度
AT+MQTTPUB=0,"$sys/oay0gSchgn/cabinet/thing/property/post","{\"id\":\"123\"\,\"params\":{\"temp\":{\"value\":33.000000}\,\"humi\":{\"value\":33.000000}}}",0,0

//排风扇
AT+MQTTPUB=0,"$sys/oay0gSchgn/cabinet/thing/property/post","{\"id\":\"123\"\,\"params\":{\"fan\":{\"value\":false}}}",0,0

//一号柜
AT+MQTTPUB=0,"$sys/oay0gSchgn/cabinet/thing/property/post","{\"id\":\"123\"\,\"params\":{\"one\":{\"value\":false}}}",0,0

//二号柜
AT+MQTTPUB=0,"$sys/oay0gSchgn/cabinet/thing/property/post","{\"id\":\"123\"\,\"params\":{\"two\":{\"value\":false}}}",0,0

//三号柜
AT+MQTTPUB=0,"$sys/oay0gSchgn/cabinet/thing/property/post","{\"id\":\"123\"\,\"params\":{\"three\":{\"value\":false}}}",0,0

*/

//==========================================================
void ESP8266_Send_DHT11_Data(double temp, double humi)
{
    char cmdBuf[512]; // 使用缓冲区

    // 构造 MQTT 发送命令
    sprintf(cmdBuf,
            "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"temp\\\":{\\\"value\\\":%lf}\\,\\\"humi\\\":{\\\"value\\\":%lf}}}\",0,0\r\n",
            pubtopic, temp, humi);

    // 发送命令并检查返回 "OK"
    if (ESP8266_SendCmd(cmdBuf, "OK"))
    {
        Delay_ms(20); // 增加适当的延时
    }
}

void ESP8266_Send_fan_Data(uint8_t fan)
{
	char cmdBuf[512]; // 使用缓冲区

    // 构造 MQTT 发送命令
    sprintf(cmdBuf,
            "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"fan\\\":{\\\"value\\\":%s}}}\",0,0\r\n",
            pubtopic, fan==1?"true ":"false");

    // 发送命令并检查返回 "OK"
    if (ESP8266_SendCmd(cmdBuf, "OK"))
    {
        Delay_ms(20); // 增加适当的延时
    }
}

void ESP8266_Send_fire_Data(uint8_t fire)
{
	char cmdBuf[512]; // 使用缓冲区

    // 构造 MQTT 发送命令
    sprintf(cmdBuf,
            "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"fire\\\":{\\\"value\\\":%s}}}\",0,0\r\n",
            pubtopic, fire==1?"true ":"false");

    // 发送命令并检查返回 "OK"
    if (ESP8266_SendCmd(cmdBuf, "OK"))
    {
        Delay_ms(20); // 增加适当的延时
    }
}

void ESP8266_Send_one_Data(uint8_t one)
{
	char cmdBuf[512]; // 使用缓冲区

    // 构造 MQTT 发送命令
    sprintf(cmdBuf,
            "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"one\\\":{\\\"value\\\":%s}}}\",0,0\r\n",
            pubtopic, one==1?"true ":"false");

    // 发送命令并检查返回 "OK"
    if (ESP8266_SendCmd(cmdBuf, "OK"))
    {
        Delay_ms(20); // 增加适当的延时
    }
}

void ESP8266_Send_two_Data(uint8_t two)
{
	char cmdBuf[512]; // 使用缓冲区

    // 构造 MQTT 发送命令
    sprintf(cmdBuf,
            "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"two\\\":{\\\"value\\\":%s}}}\",0,0\r\n",
            pubtopic, two==1?"true ":"false");

    // 发送命令并检查返回 "OK"
    if (ESP8266_SendCmd(cmdBuf, "OK"))
    {
        Delay_ms(20); // 增加适当的延时
    }
}

void ESP8266_Send_three_Data(uint8_t three)
{
	char cmdBuf[512]; // 使用缓冲区

    // 构造 MQTT 发送命令
    sprintf(cmdBuf,
            "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"three\\\":{\\\"value\\\":%s}}}\",0,0\r\n",
            pubtopic, three==1?"true ":"false");

    // 发送命令并检查返回 "OK"
    if (ESP8266_SendCmd(cmdBuf, "OK"))
    {
        Delay_ms(20); // 增加适当的延时
    }
}

void ESP8266_Send_all_Data(uint8_t all)
{
	char cmdBuf[512]; // 使用缓冲区

    // 构造 MQTT 发送命令
    sprintf(cmdBuf,
            "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"one\\\":{\\\"value\\\":%s}}}\",0,0\r\n",
            pubtopic, all==1?"true ":"false");
		if (ESP8266_SendCmd(cmdBuf, "OK"))
    {
        Delay_ms(20); // 增加适当的延时
    }
	sprintf(cmdBuf,
            "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"two\\\":{\\\"value\\\":%s}}}\",0,0\r\n",
            pubtopic, all==1?"true ":"false");
		if (ESP8266_SendCmd(cmdBuf, "OK"))
    {
        Delay_ms(20); // 增加适当的延时
    }
	sprintf(cmdBuf,
            "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"three\\\":{\\\"value\\\":%s}}}\",0,0\r\n",
            pubtopic, all==1?"true ":"false");
    // 发送命令并检查返回 "OK"
    if (ESP8266_SendCmd(cmdBuf, "OK"))
    {
        Delay_ms(20); // 增加适当的延时
    }
}

//==========================================================
//	函数名称：	ESP8266_Init
//
//	函数功能：	初始化ESP8266
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_Init(void)
{
	
	GPIO_InitTypeDef GPIO_Initure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	//ESP8266复位引脚
	GPIO_Initure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Initure.GPIO_Pin = GPIO_Pin_11;					//GPIOA11-复位
	GPIO_Initure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_Initure);
	
	GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_RESET);
	Delay_ms(250);
	GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_SET);
	Delay_ms(500);
	
	ESP8266_Clear();

	while(ESP8266_SendCmd("AT\r\n", "OK"))
		Delay_ms(500);
	
	while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
		Delay_ms(500);
	
	while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "OK"))
		Delay_ms(500);
	
	while(ESP8266_SendCmd(ESP8266_MQTTUS_INFO, "OK"))
		Delay_ms(500);
	
	while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "OK"))
		Delay_ms(500);
	
	while(ESP8266_SendCmd("AT+MQTTSUB=0,\"$sys/oay0gSchgn/cabinet/thing/property/set\",0\r\n", "OK"))
		Delay_ms(500);

}

//==========================================================
//	函数名称：	USART2_IRQHandler
//
//	函数功能：	串口3收发中断
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void USART2_IRQHandler(void)
{

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //接收中断
	{
		if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //查接收缓冲区esp8266_buf是否已满。如果已满，则将计数器esp8266_cnt重置为0，以防止缓冲区溢出。
		esp8266_buf[esp8266_cnt++] = USART2->DR;
		
		USART_ClearFlag(USART2, USART_FLAG_RXNE);
	}

}

//==========================================================
//	函数名称：	ESP8266_GetIPD
//
//	函数功能：	获取平台返回的数据
//
//	入口参数：	等待的时间(乘以10ms)
//
//	返回参数：	平台返回的原始数据
//
//	说明：		不同网络设备返回的格式不同，需要去调试
//				如ESP8266的返回格式为	"+MQTTSUBRECV,x:yyy"	x代表数据长度，yyy是数据内容
//==========================================================
const char *ESP8266_GetIPD(unsigned short timeOut)
{
    char *ptrIPD = NULL;

    do
    {
        if(ESP8266_WaitRecive() == REV_OK) // 如果接收完成
        {
            ptrIPD = strstr((char *)esp8266_buf, "+MQTTSUBRECV"); // 搜索“+MQTTSUBRECV”头
            if(ptrIPD != NULL) // 如果找到了
            {
                ptrIPD = strchr(ptrIPD, ':'); // 找到第一个':'，这是数据开始的标志
                if(ptrIPD != NULL)
                {
                    ptrIPD++; // 跳过':'
                    // 再次查找下一个':'，以跳过消息ID
                    ptrIPD = strchr(ptrIPD, ':');
                    if(ptrIPD != NULL)
                    {
                        ptrIPD++; // 跳过第二个':'
                        return (const char *)(ptrIPD); // 返回指向实际数据的指针
                    }
                    else
                    {
                        return NULL; // 如果没有找到第二个':'，返回NULL
                    }
                }
                else
                {
                    return NULL; // 如果没有找到第一个':'，返回NULL
                }
            }
        }

        Delay_ms(50); // 延时等待
    } while(timeOut--);

    return NULL; // 超时还未找到，返回空指针
}

/**
 * @brief  从JSON字符串中提取出指定键的值
 * @param  *buffer 包含JSON的字符串
 * @param  *key 指定查找的键值
 * @param  *value 储存查找的键值
 * @param  valueMaxLen 变量缓冲区的最大长度
 * @retval 无
 */
void extractJsonValue(const char *buffer, const char *key, char *value, size_t valueMaxLen)
{
    const char *jsonStart, *jsonEnd, *keyStart, *keyValueEnd;
    size_t keyValueLength;
    size_t keyLength = strlen(key);

    char keyPattern[40];                   /* 预留足够的空间存储 key 和查找格式串 */
    sprintf(keyPattern, "\"%s\"", key);   /* 构建查找模式串，即 "key": */
    
    /* 查找JSON字符串的开头 */
    jsonStart = strchr(buffer, '{');
    if (jsonStart == NULL)
        return;

    /* 查找JSON字符串的结尾 */
    jsonEnd = strrchr(jsonStart, '}');
    if (jsonEnd == NULL)
        return;

    /* 查找键 */
    keyStart = strstr(jsonStart, keyPattern);
    if (keyStart == NULL)
        return;

    /* 查找键值的结束 括号 ，确定键值的结束位置 */
    keyValueEnd = strchr(keyStart, '}');
    if (keyValueEnd == NULL || keyValueEnd > jsonEnd)
        return;

    /* 根据键值的起始和结束位置，计算键值的长度 */
    keyValueLength = keyValueEnd - keyStart;

    /* 拷贝键值到value缓冲区中 */
    if (keyValueLength < valueMaxLen)
    {
        strncpy(value, keyStart, keyValueLength);
        value[keyValueLength] = '\0';   /* 添加字符串终止字符\0 */
    }
    else
    {
        /* 如果提供的缓冲区不够大，那么可以根据实际情况处理，这里简单地截断字符串 */
        strncpy(value, keyStart, valueMaxLen - 1);
        value[valueMaxLen - 1] = '\0';   /* 添加字符串终止字符\0 */
    }
}

