#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "DHT11.h"


void DHT11_GPIO_Config(void)
{		
	GPIO_InitTypeDef GPIO_InitStructure; // 定义GPIO初始化结构体

	RCC_APB2PeriphClockCmd(DHT11_Out_RCC, ENABLE);  // 使能DHT11的GPIO时钟
  	GPIO_InitStructure.GPIO_Pin = DHT11_Out_Pin;	 // 设置GPIO的引脚
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 设置GPIO的模式为推挽输出
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 设置GPIO的速度为50MHz
  	GPIO_Init(DHT11, &GPIO_InitStructure);		  // 初始化GPIO

	GPIO_SetBits(DHT11, DHT11_Out_Pin);	 // 设置GPIO的输出为高电平
}

static void DHT11_Mode_IPU(void)
{
 	  GPIO_InitTypeDef GPIO_InitStructure;
	  GPIO_InitStructure.GPIO_Pin = DHT11_Out_Pin;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ; 
	  GPIO_Init(DHT11, &GPIO_InitStructure);	 
}

static void DHT11_Mode_Out_PP(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;

  	GPIO_InitStructure.GPIO_Pin = DHT11_Out_Pin;	

  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  	GPIO_Init(DHT11, &GPIO_InitStructure);	 	 
}

static uint8_t Read_Byte(void)
{
	uint8_t i, temp=0;

	for (i=0; i<8; i++)    
	{	 
		while (DHT11_DATA_IN() == Bit_RESET);

		Delay_us(40);    	  

		if (DHT11_DATA_IN() == Bit_SET)
		{
			while(DHT11_DATA_IN() == Bit_SET);

			temp |= (uint8_t)(0x01 << (7 - i));
		}
		else
		{			   
			temp &= (uint8_t) ~ (0x01<<(7-i)); 
		}
	}
	return temp;
}

uint8_t Read_DHT11(DHT11_Data_TypeDef *DHT11_Data)
{  
	DHT11_Mode_Out_PP();
	DHT11_DATA_OUT(LOW);
	Delay_ms(18);

	DHT11_DATA_OUT(HIGH); 

	Delay_us(30);  

	DHT11_Mode_IPU();

	if(DHT11_DATA_IN() == Bit_RESET)     
	{
		while(DHT11_DATA_IN() == Bit_RESET);

		while(DHT11_DATA_IN() == Bit_SET);

		DHT11_Data -> humi_int = Read_Byte();

		DHT11_Data -> humi_deci = Read_Byte();

		DHT11_Data -> temp_int = Read_Byte();

		DHT11_Data -> temp_deci = Read_Byte();

		DHT11_Data -> check_sum= Read_Byte();


		DHT11_Mode_Out_PP();
		DHT11_DATA_OUT(HIGH);

		if (DHT11_Data -> check_sum == DHT11_Data -> humi_int + DHT11_Data -> humi_deci + DHT11_Data -> temp_int + DHT11_Data -> temp_deci)
			return SUCCESS;
		else 
			return ERROR;
	}
	else
	{		
		return ERROR;
	}   
}	
