#include "stm32f10x.h"  // Device header

/**
  * 函    数：定时中断初始化
  * 参    数：无
  * 返 回 值：无
  */
void Timer3_Init(void)
{
    /* 开启时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  // 开启TIM3的时钟
    
    /* 配置时钟源 */
    TIM_InternalClockConfig(TIM3);  // 选择TIM3为内部时钟
    
    /* 时基单元初始化 */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;  // 定义结构体变量
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;  // 时钟分频，选择不分频
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;  // 计数器模式，选择向上计数
    TIM_TimeBaseInitStructure.TIM_Period = 10000 - 1;  // 计数周期，即ARR的值
    TIM_TimeBaseInitStructure.TIM_Prescaler = 7200 - 1;  // 预分频器，即PSC的值
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;  // 重复计数器，高级定时器才会用到
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);  // 配置TIM3的时基单元
    
    /* 中断输出配置 */
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);  // 清除定时器更新标志位
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);  // 开启TIM3的更新中断
    
    /* NVIC中断分组 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  // 配置NVIC为分组2
    
    /* NVIC配置 */
    NVIC_InitTypeDef NVIC_InitStructure;  // 定义结构体变量
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  // 选择配置NVIC的TIM3线
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  // 指定NVIC线路使能
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  // 指定NVIC线路的抢占优先级为2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  // 指定NVIC线路的响应优先级为1
    NVIC_Init(&NVIC_InitStructure);  // 配置NVIC外设
    
    /* TIM使能 */
    TIM_Cmd(TIM3, ENABLE);  // 使能TIM3，定时器开始运行
}

/* 定时器中断函数

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
    {
        // 在这里添加你的中断处理代码
        
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  // 清除中断标志位
    }
}*/
