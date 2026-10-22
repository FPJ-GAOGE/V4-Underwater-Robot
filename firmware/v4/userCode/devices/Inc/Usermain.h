//
// Created by LEGION on 2021/10/4.
//

#ifndef RM_FRAME_C_DEVICE_H
#define RM_FRAME_C_DEVICE_H

#include "main.h"
#include "can.h"
#include "dma.h"
#include "i2c.h"
#include "iwdg.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include "ParamConfig.h"
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "core_cm4.h"
#include "Utils.h"


#define INRANGE(NUM, MIN, MAX) \
{\
    if(NUM<MIN){\
        NUM=MIN;\
    }else if(NUM>MAX){\
        NUM=MAX;\
    }\
}

template <typename T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi)
{
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

/*枚举类型定义------------------------------------------------------------*/

extern uint8_t RxBuffer6[MAX_SERIAL_LENGTH];

//extern void ChassisStart();
//extern void ChassisHandle();
//extern void CtrlHandle();
//extern void ARMHandle();

extern VERSION_E Robot_Version;

//extern const int length;

/*extern void I2C_Extension_init();
extern void I2C_Extension();
*/
//extern void Update_level();


extern int Count_ms_dog;
extern int Flag_stop;
extern int Flag_serial;
extern uint32_t volatile time_start, time_end, time_interval;
extern bool tick_100hz;
extern bool tick_10hz;

/*结构体定义--------------------------------------------------------------*/



/*类型定义----------------------------------------------------------------*/



class Device {
public:
    DEVICE_TYPE_E deviceType;
    //uint32_t deviceID;

    virtual void Init() = 0;

    virtual void Handle() = 0;

    virtual void Receive() = 0;

    //virtual void ErrorHandle() = 0;

};

void SendFloat(float value, uint8_t decimalPlaces, bool endSign);
void SendInt(int value, bool endSign);
void SendString(char* buffer_string, bool endSign);

void DWT_Init(void);

uint32_t GetDT(void);

float ConvertDT(uint32_t cnt);

void NOP_Delay(uint32_t delay);

/*结构体成员取值定义组------------------------------------------------------*/
/*外部变量声明-------------------------------------------------------------*/
/*外部函数声明-------------------------------------------------------------*/

extern void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);
extern void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);

#endif //RM_FRAME_C_DEVICE_H
