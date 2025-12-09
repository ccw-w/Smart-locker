#include "stm32f10x.h"                  // Device header

#include <stdarg.h>
#include <string.h>
#include <stdio.h>


/*
************************************************************
*	函数名称：	Usart2_Init（ESP8266接收中断）
*
*	函数功能：	串口初始化
*
*	入口参数：	baud：设定的波特率
*
*	返回参数：	无
*
*	说明：		TX-PA2		RX-PA3
************************************************************
*/
void Usart2_Init(unsigned int baud)
{
    GPIO_InitTypeDef gpio_initstruct;
    USART_InitTypeDef usart_initstruct;
    NVIC_InitTypeDef nvic_initstruct;
    
    // 使能GPIOA和USART2的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
    // PA2 配置为USART2的TX（发送）
    gpio_initstruct.GPIO_Pin = GPIO_Pin_2;
    gpio_initstruct.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
    gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_initstruct);
    
    // PA3 配置为USART2的RX（接收）
    gpio_initstruct.GPIO_Pin = GPIO_Pin_3;
    gpio_initstruct.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
    GPIO_Init(GPIOA, &gpio_initstruct);
    
    // USART2 初始化设置
    usart_initstruct.USART_BaudRate = baud; // 波特率
    usart_initstruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控
    usart_initstruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // 接收和发送模式
    usart_initstruct.USART_Parity = USART_Parity_No; // 无校验位
    usart_initstruct.USART_StopBits = USART_StopBits_1; // 1个停止位
    usart_initstruct.USART_WordLength = USART_WordLength_8b; // 8位数据位
    USART_Init(USART2, &usart_initstruct);
    
    USART_Cmd(USART2, ENABLE); // 使能USART2
    
    // 使能USART2的接收中断
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    
    // 配置NVIC，使能USART2中断
    nvic_initstruct.NVIC_IRQChannel = USART2_IRQn;
    nvic_initstruct.NVIC_IRQChannelCmd = ENABLE;
    nvic_initstruct.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级
    nvic_initstruct.NVIC_IRQChannelSubPriority = 1; // 子优先级
    NVIC_Init(&nvic_initstruct);
}

/*
************************************************************
*	函数名称：	Usart_SendString
*
*	函数功能：	串口数据发送
*
*	入口参数：	USARTx：串口组
*				str：要发送的数据
*				len：数据长度
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Usart_SendString(USART_TypeDef *USARTx, unsigned char *str, unsigned short len)
{

	unsigned short count = 0;
	
	for(; count < len; count++)
	{
		USART_SendData(USARTx, *str++);									//发送数据
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);		//等待发送完成
	}

}

/*
************************************************************
*	函数名称：	UsartPrintf
*
*	函数功能：	格式化打印
*
*	入口参数：	USARTx：串口组
*				fmt：不定长参
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void UsartPrintf(USART_TypeDef *USARTx, char *fmt,...)
{

	unsigned char UsartPrintfBuf[296];
	va_list ap;
	unsigned char *pStr = UsartPrintfBuf;
	
	va_start(ap, fmt);
	vsnprintf((char *)UsartPrintfBuf, sizeof(UsartPrintfBuf), fmt, ap);							//格式化
	va_end(ap);
	
	while(*pStr != 0)
	{
		USART_SendData(USARTx, *pStr++);
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
	}

}

