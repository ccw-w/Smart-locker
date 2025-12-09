#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "OLED.h"
#include "DHT11.h"
#include "Serial.h"
#include "Fire.h"
#include "InfraredSensor.h"
#include "Servo.h"
#include "usart.h"
#include "esp8266.h"
#include "Timer.h"
#include "Fan.h"
// C库
#include <string.h>
#include <stdio.h>


/********动画**********/

// 定义屏幕的宽度和高度
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

// 定义眼睛的参数
#define EYE_WIDTH     45   // 眼睛的宽度
#define EYE_HEIGHT_NORMAL  55   // 眼睛的正常高度
#define EYE_HEIGHT_MIN     50   // 眼睛的最小高度（闭眼）
#define EYE_RADIUS     10   // 眼睛圆角的半径
#define EYE_SPACING    10   // 两眼之间的距离
#define EYE_FILLED     1    // 眼睛是否填充

// 定义动画步长
#define STEP_SIZE 2
/**
  * 坐标轴定义：
  * 左上角为(0, 0)点
  * 横向向右为X轴，取值范围：0~127
  * 纵向向下为Y轴，取值范围：0~63
  * 
  *       0             X轴           127 
  *      .------------------------------->
  *    0 |
  *      |
  *      |
  *      |
  *  Y轴 |
  *      |
  *      |
  *      |
  *   63 |
  *      vx
  * 
  */

/********动画**********/


/*******眼睛*******/
	int16_t eye_height = EYE_HEIGHT_NORMAL;
    int16_t blinking = 1; // 控制眼睛尺寸的变化方向
/*******眼睛*******/



/********刷新********/

uint16_t one_s = 0;
uint16_t two_s = 0;
uint16_t three_s = 0;
uint16_t fan_s = 0;
uint16_t fire_s = 0;

/********刷新********/




// 声明全局变量
const char *dataPtr = NULL;



DHT11_Data_TypeDef DHT11_Data; // 用于存储DHT11传感器的数据

// DHT11变量
u8 humidityH; // 湿度整数部分
u8 humidityL; // 湿度小数部分
u8 temperatureH; // 温度整数部分
u8 temperatureL; // 温度小数部分
double humi, temp; // 存储湿度和温度的浮点数值
//****在循环里面 避免重复的 
uint16_t readCounter = 0;
//****眨眼睛的 
uint16_t EyCounter = 0;
const uint16_t readInterval = 2; // 每几个循环读取一次

// 更新OLED显示函数
void UpdateOLED(double temp, double humi)
{
	// 清除屏幕
     OLED_Clear();
    // 列，行
    /*在(0, 20)位置画矩形，宽12像素，高16像素，未填充*/
    OLED_DrawRectangle(88, 0, 40, 64, OLED_UNFILLED);
        
if(!one_s)
{
	
    OLED_ShowImage(89, 0, 38, 16, close);
}else OLED_ShowImage(89, 0, 38, 16, open);
if(!two_s)
{
	OLED_ShowImage(89, 16, 38, 16, close);
}else OLED_ShowImage(89, 16, 38, 16, open);
if(!three_s)
{
	OLED_ShowImage(89, 33, 38, 16, close);
}else OLED_ShowImage(89, 33, 38, 16, open);
if(!fan_s)
{
	OLED_ShowImage(39, 34, 32, 14, smile);
}else OLED_ShowImage(39, 34, 32, 14, fan);
if(!fire_s)
{
	OLED_ShowImage(0, 34, 32, 14, smile);
}else OLED_ShowImage(0, 34, 32, 14, fire);
	
	OLED_ShowImage(89, 49, 38, 16, close);
    OLED_ShowString(0, 48, "舒适度:", OLED_8X16);
    
    OLED_ShowString(81, 0, "1", OLED_8X16);
    OLED_ShowString(81, 17, "2", OLED_8X16);
    OLED_ShowString(81, 34, "3", OLED_8X16);
    OLED_ShowString(81, 48, "4", OLED_8X16);
    
    OLED_ShowString(0, 0, "温度:", OLED_8X16);
    OLED_ShowString(0, 17, "湿度:", OLED_8X16);

    // 更新温度显示(温度)
    OLED_ShowNum(41, 0, (int)temp, 2, OLED_8X16);
    OLED_ShowString(59, 0, ".", OLED_8X16);
    OLED_ShowNum(67, 0, (int)((temp - (int)temp) * 10), 1, OLED_8X16);

    // 更新湿度显示(湿度)
    OLED_ShowNum(41, 17, (int)humi, 2, OLED_8X16);
    OLED_ShowString(59, 17, ".", OLED_8X16);
    OLED_ShowNum(67, 17, (int)((humi - (int)humi) * 10), 1, OLED_8X16);
    
    if ((int)temp >= 20 && (int)temp <= 25 && (int)humi >= 40 && (int)humi <= 60)
    {
        OLED_ShowString(57, 48, "好", OLED_8X16);
    } 
    else if ((int)temp >= 18 && (int)temp <= 28 && (int)humi >= 30 && (int)humi <= 70) 
    {
        OLED_ShowString(57, 48, "良", OLED_8X16);
    } 
    else 
    {
        OLED_ShowString(57, 48, "差", OLED_8X16);
    }
    
    OLED_Update();
}

// 处理串口命令函数（语音识别模块）
void ProcessCommand(char* command)
{
    if (strcmp(command, "FAN_ON") == 0) {
        ESP8266_Send_fan_Data(1);
        Fan_ON();
		fan_s=1;
        OLED_ShowImage(39, 34, 32, 14, fan);
    }
    else if (strcmp(command, "FAN_OFF") == 0) {
        ESP8266_Send_fan_Data(0);
        Fan_OFF();
		fan_s=0;
        OLED_ShowImage(39, 34, 32, 14, smile);
    }
    else if (strcmp(command, "ONE_ON") == 0) {
        ESP8266_Send_one_Data(1);
		one_s=1;
        Servo_SetAngle(56);
        OLED_ShowImage(89, 0, 38, 16, open);
    }
    else if (strcmp(command, "ONE_OFF") == 0) {
        ESP8266_Send_all_Data(0);
        Servo_SetAngle(0);
		one_s=0;
        OLED_ShowImage(89, 0, 38, 16, close);
    }
    else if (strcmp(command, "TWO_ON") == 0) {
        ESP8266_Send_two_Data(1);
        Servo_SetAngle(123);
		two_s=1;
        OLED_ShowImage(89, 16, 38, 16, open);
    }
    else if (strcmp(command, "TWO_OFF") == 0) {
        ESP8266_Send_all_Data(0);
        Servo_SetAngle(0);
		two_s=0;
        OLED_ShowImage(89, 16, 38, 16, close);
    }
    else if (strcmp(command, "THREE_ON") == 0) {
        ESP8266_Send_three_Data(1);
        Servo_SetAngle(180);
		three_s=1;
        OLED_ShowImage(89, 33, 38, 16, open);
    }
    else if (strcmp(command, "THREE_OFF") == 0) {
        ESP8266_Send_all_Data(0);
        Servo_SetAngle(0);
		three_s=0;
        OLED_ShowImage(89, 33, 38, 16, close);
    }
    OLED_Update();
}

// 处理ESP8266数据函数
void ProcessESP8266Data(const char *dataPtr)
{
    char fanValue[50] = "";  // 初始化为空字符串
    char oneValue[50] = "";
    char twoValue[50] = "";
    char threeValue[50] = "";

    // 提取 "fan" 键的值
    extractJsonValue(dataPtr, "fan", fanValue, sizeof(fanValue));
    // 提取 "one" 键的值
    extractJsonValue(dataPtr, "one", oneValue, sizeof(oneValue));
    // 提取 "two" 键的值
    extractJsonValue(dataPtr, "two", twoValue, sizeof(twoValue));
    // 提取 "three" 键的值
    extractJsonValue(dataPtr, "three", threeValue, sizeof(threeValue));

    // 处理 "fan" 键的值
    if (strcmp(fanValue, "\"fan\":true") == 0)
    {
        ESP8266_Send_fan_Data(1);
		fan_s=1;
		Fan_ON();
        OLED_ShowImage(39, 34, 32, 14, fan);
    }
    else if (strcmp(fanValue, "\"fan\":false") == 0)
    {
        ESP8266_Send_fan_Data(0);
		fan_s=0;
		Fan_OFF();
        OLED_ShowImage(39, 34, 32, 14, smile);
    }
    else if (strcmp(oneValue,  "\"one\":true") == 0) // 处理 "one" 键的值
    {
        ESP8266_Send_one_Data(1);
		one_s=1;
        Servo_SetAngle(56);
		OLED_ShowImage(89, 0, 38, 16, open);
    }
    else if (strcmp(oneValue, "\"one\":false") == 0)
    {
        ESP8266_Send_all_Data(0);
		one_s=0;
        Servo_SetAngle(0);
		OLED_ShowImage(89, 0, 38, 16, close);
    }
    else if (strcmp(twoValue, "\"two\":true") == 0) // 处理 "two" 键的值
    {
        ESP8266_Send_two_Data(1);
		two_s=1;
        Servo_SetAngle(123);
		OLED_ShowImage(89, 16, 38, 16, open);
    }
    else if (strcmp(twoValue, "\"two\":false") == 0)
    {
        ESP8266_Send_all_Data(0);
		two_s=0;
        Servo_SetAngle(0);
		OLED_ShowImage(89, 16, 38, 16, close);
    }
    else if (strcmp(threeValue, "\"three\":true") == 0) // 处理 "three" 键的值
    {
        ESP8266_Send_three_Data(1);
		three_s=1;
        Servo_SetAngle(180);
		OLED_ShowImage(89, 33, 38, 16, open);
    }
    else if (strcmp(threeValue, "\"three\":false") == 0)
    {
        ESP8266_Send_all_Data(0);
		three_s=0;
        Servo_SetAngle(0);
		OLED_ShowImage(89, 33, 38, 16, close);
    }
	OLED_Update();
}

void Eyshow()
{
	for(int i = 0;i<15;i++){
	 // 计算眼睛的位置
     int16_t eye_x_left = (SCREEN_WIDTH - EYE_WIDTH * 2 - EYE_SPACING) / 2;
     int16_t eye_x_right = eye_x_left + EYE_WIDTH + EYE_SPACING;
     int16_t eye_y = (SCREEN_HEIGHT - eye_height) / 2;

     // 清除屏幕
     OLED_Clear();

     // 画左眼
     OLED_DrawRoundedRectangle(eye_x_left, eye_y, EYE_WIDTH, eye_height, EYE_RADIUS, EYE_FILLED);

     // 画右眼
     OLED_DrawRoundedRectangle(eye_x_right, eye_y, EYE_WIDTH, eye_height, EYE_RADIUS, EYE_FILLED);

     // 更新屏幕
     OLED_Update();

     // 延迟一段时间，模拟动画帧率
     Delay_ms(50);

     // 更新眼睛高度
     if (blinking) {
        eye_height -= STEP_SIZE;
          if (eye_height <= EYE_HEIGHT_MIN) {
                blinking = 0;
            }
        } else {
            eye_height += STEP_SIZE;
            if (eye_height >= EYE_HEIGHT_NORMAL) {
                eye_height = EYE_HEIGHT_NORMAL;
               // Delay_ms(1000); // 模拟眨眼后的停顿
                blinking = 1;
            }
        }
	}
	EyCounter=0;
}

int main(void)
{
    Fire_Init();
    Fan_Init();
    Timer3_Init();  // 定时中断初始化
    OLED_Init();
    Serial_Init();
    DHT11_GPIO_Config();
    Servo_Init();
    Usart2_Init(115200);
    ESP8266_Init();
	
    while (1)
    {
        if (readCounter >= readInterval)
        {
            readCounter = 0;
			EyCounter++;
            
            // 读取DHT11数据并更新OLED
            if (Read_DHT11(&DHT11_Data) == SUCCESS)
            {
                humi = DHT11_Data.humi_int + DHT11_Data.humi_deci / 10.0;
                temp = DHT11_Data.temp_int + DHT11_Data.temp_deci / 10.0;
		if(!Fire_Get())
		{
			ESP8266_Send_fire_Data(1);
			fire_s = 1;
			OLED_ShowImage(0, 34, 32, 14, fire);
			OLED_Update();
		} else
		{
			ESP8266_Send_fire_Data(0);
			fire_s = 0;
		}
                UpdateOLED(temp, humi); // 更新显示
                ESP8266_Send_DHT11_Data(temp, humi); // 发送数据
                ESP8266_Clear();
            }
        }
		
		
        // 处理串口数据
        if (Serial_RxFlag == 1)
        {
            ProcessCommand(Serial_RxPacket); // 处理接收到的命令
            Serial_RxFlag = 0;
        }

        // 处理ESP8266数据
        dataPtr = ESP8266_GetIPD(5);
        if (dataPtr != NULL)
        {
            ProcessESP8266Data(dataPtr); // 处理ESP8266返回的数据
        }
		
		if(EyCounter==10)
		{
			Eyshow();
		}
		OLED_Update();
    }
}


// 定时器中断函数
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
    {
        // 中断处理代码
        readCounter++;
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  // 清除中断标志位
    }
}
