#include "stm32f10x.h"                  // Device header

/**
  * 函    数：火焰报警初始化
  * 参    数：无
  * 返 回 值：无
  */
void Fire_Init(void)
{
    /* 开启时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  // 开启GPIOA的时钟
    
    /* GPIO初始化 */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;     // 设置为浮空输入模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 设置GPIO初始化后的默认电平 */
    //GPIO_SetBits(GPIOA, GPIO_Pin_5);  // 设置PA5引脚为高电平
}

// 获取FIRE报警器输出的高低电平
uint8_t Fire_Get(void)
{
    return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5);          // 读取PA5输入寄存器的状态
}
