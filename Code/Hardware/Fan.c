#include "stm32f10x.h"                  // Device header

/**
  * 函数：Fan初始化
  * 参数：无
  * 返回值：无
  */
void Fan_Init(void)
{
    /* 开启时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  // 开启GPIOB的时钟
    
    /* GPIO初始化 */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       // 推挽输出模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;              // PB3
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      // 输出速度50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);                 // 将PB3引脚初始化为推挽输出
    
    /* 确保引脚不是复用功能 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE );	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);		//Disable Jlink,enable SW
    
    /* 设置GPIO初始化后的默认电平 */
    GPIO_ResetBits(GPIOB, GPIO_Pin_3);                     // 设置PB3引脚为低电平（关闭风扇）
}

/**
  * 函数：Fan开启
  * 参数：无
  * 返回值：无
  */
void Fan_ON(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_3);                       // 设置PB3引脚为高电平（开启风扇）
}

/**
  * 函数：Fan关闭
  * 参数：无
  * 返回值：无
  */
void Fan_OFF(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_3);                     // 设置PB3引脚为低电平（关闭风扇）
}

/**
  * 函数：Fan状态翻转
  * 参数：无
  * 返回值：无
  */
void Fan_Toggle(void)
{
    if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_3) == 0)    // 获取输出寄存器的状态，如果当前引脚输出低电平
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_3);                   // 则设置PB3引脚为高电平
    }
    else                                                   // 否则，即当前引脚输出高电平
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_3);                 // 则设置PB3引脚为低电平
    }
}
