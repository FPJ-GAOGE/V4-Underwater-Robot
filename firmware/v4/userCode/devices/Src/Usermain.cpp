//
// Created by LEGION on 2021/10/4.
//
//此文件为主函数和中断服务函数所在地
//每个元件的使用包含三个步骤，初始化、获取数据、执行
//初始化和执行在定时器中断函数进行
//获取串口数据在串口中断函数进行
#include "Usermain.h"
#include "IMU.h"
#include "Propeller.h"
#include "Servo.h"
#include "Sensor.h"
#include "Legacy.h"
#include "Watchdog.h"
#include "LED.h"
#include "Buzzer.h"
#include "gpio.h"
#include "UARTBase.h"

VERSION_E Robot_Version = V40;//根据潜器版本调整，V30，V31，V32，V33，V40

// 静态实例化对象
IMU imu;
PressureSensor pressure_sensor;
Servo servo;
Servo_I2C servo_i2c;
Propeller_I2C propeller_i2c;
Sonar sonar;
Watchdog watchdog;
LED led;
Buzzer buzzer;

// 设备指针数组，用于统一管理和调用设备操作
//------TODO:选用需要的设备
static Device *device[] = {
        &imu,                              //imu
        &pressure_sensor,                  //水压计
        &propeller_i2c,                    //推进器（扩展板pwm控制）
        //&propeller,                      //推进器（C板pwm控制）
        &servo_i2c,                        //舵机（扩展版pwm控制）
        //&servo,                          //舵机（c板pwm控制）
        //&watchdog,                       //看门狗
        &led,                              //LED灯
        //&buzzer                          //蜂鸣器
};

// 定义设备数量宏
#define DEVICE_NUM (sizeof(device) / sizeof(device[0]))

volatile bool system_init_flag = false;
uint8_t RxBuffer6[MAX_SERIAL_LENGTH] = {0};
uint8_t RxBuffer1[MAX_SERIAL_LENGTH] = {0};
volatile uint8_t key_raw_state = 1;
uint32_t key_last_stamp;
volatile uint32_t time_start = 0, time_end = 0, time_interval = 0;
uint8_t cnt_100hz = 0, cnt_10hz = 0;
bool tick_400hz = false, tick_100hz = false, tick_10hz = false;
volatile bool usart6_has_data = false;
bool error = false;

/**
  * @brief  The application entry point.
  * @retval int
  */
// 主函数入口
int main(void)
{
    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_TIM5_Init();
    MX_TIM1_Init();
    MX_TIM4_Init();
    MX_TIM6_Init();
    MX_TIM7_Init();
    MX_TIM10_Init();
    MX_TIM8_Init();
    MX_ADC1_Init();
    MX_ADC3_Init();
    MX_USART1_UART_Init();
    MX_USART6_UART_Init();
    MX_USART3_UART_Init();
    MX_CAN1_Init();
    MX_CAN2_Init();
    MX_I2C2_Init();
    MX_I2C3_Init();
    MX_SPI1_Init();
    MX_SPI2_Init();
    //MX_IWDG_Init();
    MX_USB_DEVICE_Init();
    DWT_Init();

    HAL_TIM_Base_Start_IT(&htim5);
    HAL_TIM_Base_Start_IT(&htim1);
    HAL_TIM_PWM_Start_IT(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start_IT(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start_IT(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start_IT(&htim1, TIM_CHANNEL_4);

    HAL_TIM_Base_Start_IT(&htim8);
    HAL_TIM_PWM_Start_IT(&htim8, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start_IT(&htim8, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start_IT(&htim8, TIM_CHANNEL_3);
    //RemoteControl::init();
    bsp_flash_read(&flashData);
    HAL_TIM_Base_Start_IT(&htim10);
    HAL_TIM_PWM_Start_IT(&htim10, TIM_CHANNEL_1);
    HAL_TIM_Base_Start_IT(&htim6);
    HAL_TIM_Base_Start_IT(&htim7);

    // 初始化所有设备
    for(int i = 0; i < DEVICE_NUM; ++i){
        device[i] -> Init();
    }

    // 开启串口接收中断
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RxBuffer1, MAX_SERIAL_LENGTH);
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, RxBuffer6, MAX_SERIAL_LENGTH);
    __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
    // HAL_UARTEx_ReceiveToIdle_IT(&huart6, RxBuffer6, MAX_SERIAL_LENGTH);

    // 设置初始化完成标志
    system_init_flag = true;

    while (1){
        // IMU 400Hz处理
        if(tick_400hz){
            tick_400hz = false;
        }

        // 控制环100Hz处理
        if(tick_100hz){
            // 测试频率
            // static int a = 0;
            // a++;
            // if(a==100){
            //     time_end = GetDT();
            //     time_interval = time_end - time_start;
            //     SendInt(time_interval, 1);
            //     a = 0;
            //     time_start = GetDT();
            // }
            tick_100hz = false;
            if (usart6_has_data) {
                usart6_has_data = false;
                uint8_t addr = RxBuffer6[0];
                if (addr < DEVICE_NUM)
                    device[addr]->Receive();
                // SendInt(addr, 0);
                // SendInt(RxBuffer6[1],1);
                // std::memset(RxBuffer6, 0, MAX_SERIAL_LENGTH);
            }
            for(int i = 1 ; i < DEVICE_NUM; ++i){ // 除了imu
                device[i]->Handle();
            }
        }

        // 输出环10hz
        if (tick_10hz){
            tick_10hz = false;
            if(!error){
                SendFloat(rad2deg(imu.attitude.ang[0]), 3, 0);
                SendFloat(rad2deg(imu.attitude.ang[1]), 3, 0);
                SendFloat(rad2deg(imu.attitude.ang[2]), 3, 1);
                // SendFloat(propeller_i2c.target.ang_vel[0], 3, 0);
                // SendFloat(propeller_i2c.target.ang_vel[1], 3, 0);
                // SendFloat(propeller_i2c.target.ang_vel[2], 3, 1)
            }
            // SendFloat(imu.h30_imu.euler.roll,2,0);
            // SendFloat(imu.h30_imu.euler.pitch,2,0);
            // SendFloat(imu.h30_imu.euler.yaw,2,1);
            // SendFloat(propeller_i2c.target.torque[0], 3, 1);
            // SendFloat(propeller_i2c.target.torque[1], 3, 1);
            // SendFloat(propeller_i2c.target.torque[2], 3, 1);
            // SendFloat(pressure_sensor.data_depth, 2, 1);
            // SendFloat(propeller_i2c.target.ang[2], 2, 1);
            // SendInt(propeller_i2c.current_pwm[1], 1);
            }
    }
}

// 定时器中断服务函数, 用于周期性处理设备
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(!system_init_flag) return;
    if(htim == &htim1){ // 400Hz
        tick_400hz = true;
        if (imu.dma_state == IMU_DMA_IDLE){
            device[IM]->Handle();
        }
        cnt_100hz = (cnt_100hz + 1) % 4;
        cnt_10hz = (cnt_10hz + 1) % 40;
        if (cnt_100hz == 0) {tick_100hz = true;}
        if (cnt_10hz == 0) {tick_10hz = true;}
    }
}

// 串口接收中断服务函数, 用于处理设备的串口接收任务
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)//HAL_UART_RxCpltCallback
{
    if(!system_init_flag) return;
    if(huart->Instance==USART6) {
        if (Size >= 2){
            if (!usart6_has_data) usart6_has_data = true;
        }
        
        // 重新开启接收
        // HAL_UARTEx_ReceiveToIdle_IT(&huart6, RxBuffer6, MAX_SERIAL_LENGTH);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart6, RxBuffer6, MAX_SERIAL_LENGTH);
        __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
        return;
    }
    else if(huart->Instance==USART1){
        memcpy(imu.h30_imu.frame_buf, RxBuffer1, Size);
        imu.h30_imu.frame_size = Size;
        imu.h30_imu.Decode(imu.h30_imu.frame_buf, imu.h30_imu.frame_size);

        // 重新开启接收
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RxBuffer1, MAX_SERIAL_LENGTH);
        __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
        return;
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){
    uint32_t err = HAL_UART_GetError(huart);
    char info[13] = {"uart_error:"};
    info[11] = err + '0';
    info[12] = '\n';
    error = true;
    SendString(info, 1);
    // if (err & HAL_UART_ERROR_ORE){
    //     volatile uint32_t temp;
    //     temp = huart->Instance->SR;
    //     temp = huart->Instance->DR;
    // }
    // if (huart->Instance==USART1) 
    //     HAL_UARTEx_ReceiveToIdle_DMA(huart, RxBuffer1, MAX_SERIAL_LENGTH);
    // else if(huart->Instance==USART6)
    //     HAL_UARTEx_ReceiveToIdle_DMA(huart, RxBuffer6, MAX_SERIAL_LENGTH);
    // __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
}

// 串口发送中断回调函数
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    // #define HANDLE_UART_TX(ID)                            \
    //     do {                                              \
    //         if (huart == uartHandleList[ID]) {            \
    //             UARTBaseLite<ID>::GetInstance().OnTxDone(); \
    //             return;                                   \
    //         }                                             \
    //     } while (0)

    // HANDLE_UART_TX(1);
    // HANDLE_UART_TX(6);

    // #undef HANDLE_UART_TX
    if(huart == &huart1) {
        UARTBaseLite<1>::GetInstance().OnTxDone();
    }
    else if(huart == &huart6){
        UARTBaseLite<6>::GetInstance().OnTxDone();
    }
}

void SendFloat(float value, uint8_t decimalPlaces, bool endSign){
    bool isNegative = 0;
    uint8_t len = 0;
    static char buffer_float[20] = {0};

    // 处理负数
    if(value < 0){
        isNegative = 1;
        value = -value;
    }
    if(isNegative) {
        buffer_float[0] = '-';
        len = 1;
    }

    // 转换整数部分
    int32_t intPart = (int)value;
    len += sprintf(buffer_float+len, "%d", intPart);

    // 处理小数部分
    if (decimalPlaces > 0) {
        buffer_float[len++] = '.';

        float fraction = value - (float)intPart;
        for (uint8_t i = 0; i < decimalPlaces; i++) {
            fraction *= 10;
            int digit = (int)fraction;
            buffer_float[len++] = digit + '0';
            fraction -= digit;
        }
    }

    // 若结束则换行，否则是逗号
    if(endSign)
        buffer_float[len++] = '\n'; // 字符串结束符
    else
        buffer_float[len++] = ',';

    buffer_float[len] = '\0';

    // HAL_UART_Transmit(&huart6, (uint8_t *)buffer, len, 0x00ff);
    UARTBaseLite<6>::GetInstance().Transmit(reinterpret_cast<uint8_t *>(buffer_float),len);
}

void SendInt(int32_t value, bool endSign){
    static char buffer_int[20] = {0};
    uint8_t len = 0;
    bool isNegative = 0;

    // 处理负数
    if(value < 0){
        isNegative = 1;
        value = -value;
    }
    if(isNegative) {
        buffer_int[0] = '-';
        len = 1;
    }

    len += sprintf(buffer_int+len, "%d", value);

    if(endSign)
        buffer_int[len++] = '\n';
    else
        buffer_int[len++] = ',';

    buffer_int[len] = '\0';
    // HAL_UART_Transmit(&huart6, (uint8_t *)buffer, len, 0x00ff);
    UARTBaseLite<6>::GetInstance().Transmit(reinterpret_cast<uint8_t *>(buffer_int),len);
}

void SendString(char* buffer_string, bool endSign){
    UARTBaseLite<6>::GetInstance().Transmit(reinterpret_cast<uint8_t *>(buffer_string), strlen(buffer_string));
}

void DWT_Init(void)
{
    // 使能 DWT 外设
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    // 清零计数器
    DWT->CYCCNT = 0;

    // 启用 CPU cycle counter
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t GetDT(void)
{
    return DWT->CYCCNT;
}

float ConvertDT(uint32_t cnt)
{
    return (float)cnt / SystemCoreClock;
}

void NOP_Delay(uint32_t delay)
{
    for (volatile int i = 0; i < delay; ++i) { __NOP(); }
}