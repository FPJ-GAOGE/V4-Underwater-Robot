//
// Created by admin on 2023/10/29.
//
#include "Extension.h"
#include "Sensor.h"
#include "Propeller.h"
//此文件用来处理i2c扩展板（TCA）及pwm扩展板（PCA）
//i2c扩展板接在C板i2c口（上方第二排4根线，左-SDA-SCL-VCC-GND-右）
//pwm扩展板接在i2c扩展板的0号口

I2C_TaskState i2c_state = I2C_TaskState::PressureSensor;

// 开通第 channel 路的I2C 
void TCA_SetChannel(uint8_t channel)
{
    uint8_t data;
    data = 0x01 << channel;
    HAL_I2C_Master_Transmit(&hi2c2, (TCA9548A_ADDR << 1), &data, 1, 0xffff);
    //HAL_I2C_Master_Transmit(&hi2c2, TCA9548A_ADDR, &data, 1, 0xffff);
}

uint8_t PCA_Read(uint8_t startAddress)
{
    //设置开始读取数据的寄存器地址
    uint8_t tx[1];
    uint8_t buffer[1];
    tx[0] = startAddress;
    HAL_I2C_Master_Transmit(&hi2c2,PCA9685_ADDR,tx,1,10000);
    HAL_I2C_Master_Receive(&hi2c2,PCA9685_ADDR,buffer,1,10000);
    return buffer[0];
}

void PCA_Write(uint8_t startAddress, uint8_t buffer)
{
    //设置开始写数据的寄存器地址
    uint8_t tx[2];
    tx[0] = startAddress;
    tx[1] = buffer;
    HAL_I2C_Master_Transmit(&hi2c2,PCA9685_ADDR, tx,2,I2C_TO); //利用HAL库的I2C通讯函数对寄存器地址写值
}

// 设置PWM频率为 freq
void PCA_Setfreq(float freq)
{
    uint8_t prescale,oldmode,newmode;
    double prescaleval;
    freq *= 1.016; 			//实际使用过程中存在偏差需要×矫正系数=0.83
    // prescaleval = 25000000;
    prescaleval = 25000000.0/(4096.0*freq);
    //prescaleval /= freq;
    prescaleval -= 1;
    prescale = floor(prescaleval + 0.5f);			//floor向下取整函数
    oldmode = PCA_Read(PCA9685_MODE1);
    newmode = (oldmode&0x7F) | 0x10; // sleep睡眠
    PCA_Write(PCA9685_MODE1, newmode); // go to sleep（需要进入随眠状态才能设置频率）
    PCA_Write(PCA9685_PRESCALE, prescale); // 设置预分频系数
    PCA_Write(PCA9685_MODE1, oldmode);
    HAL_Delay(2);
    PCA_Write(PCA9685_MODE1, oldmode | 0xA1);
}

//  @brief 设置对应输出引脚的PWM值
//  @param num PWM输出引脚0 ~ 15, 0 ~ 7 对应推进器， 8 ~ 11 对应舵机
//  @param on PWM上升计数值0 ~ 4096, 一般取 on = 0
//  @param off PWM下降计数值0 ~ 4096, 占空比为 off/4096
void PCA_Setpwm(uint8_t num, uint32_t on, uint32_t off) 
{
    // uint8_t data[5];
    // data[0] = LED0_ON_L + 4 * num;
    // data[1] = on & 0xFF;
    // data[2] = on >> 8;
    // data[3] = off & 0xFF;
    // data[4] = off >> 8;
    // HAL_I2C_Master_Transmit(&hi2c2, PCA9685_ADDR, data, 5, I2C_TO);
    PCA_Write(LED0_ON_L+4*num, on);
    PCA_Write(LED0_ON_H+4*num, on>>8);
    PCA_Write(LED0_OFF_L+4*num, off);
    PCA_Write(LED0_OFF_H+4*num, off>>8);
}

void PCA_Setpwm_IT(uint8_t num, uint32_t on, uint32_t off)
{
    uint8_t data[5];
    data[0] = LED0_ON_L + 4 * num;
    data[1] = on & 0xFF;
    data[2] = on >> 8;
    data[3] = off & 0xFF;
    data[4] = off >> 8;
    HAL_I2C_Master_Transmit_IT(&hi2c2, PCA9685_ADDR, data, 5);
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C2){
        NOP_Delay(2000); // 等待TCA稳定
        switch (i2c_state)
        {
        case I2C_TaskState::PressureSensor:
            switch (pressure_sensor.ps_i2c_state)
            {
                case PS_I2C_TaskState::TCA_SET_0:
                    pressure_sensor.ps_i2c_state = PS_I2C_TaskState::TRANSMIT_READ_0;
                    // NOP_Delay(2000); // 等待TCA稳定
                    break;

                case PS_I2C_TaskState::TCA_SET_1:
                    pressure_sensor.ps_i2c_state = PS_I2C_TaskState::TRANSMIT_READ_1;
                    // NOP_Delay(2000); // 等待TCA稳定
                    break;

                case PS_I2C_TaskState::TCA_SET_2:
                    pressure_sensor.ps_i2c_state = PS_I2C_TaskState::TRANSMIT_READ_2;
                    // NOP_Delay(2000); // 等待TCA稳定
                    break;

                case PS_I2C_TaskState::TCA_SET_3:
                    pressure_sensor.ps_i2c_state = PS_I2C_TaskState::TRANSMIT_READ_3;
                    // NOP_Delay(2000); // 等待TCA稳定
                    break;

                case PS_I2C_TaskState::TRANSMIT_READ_0:
                    pressure_sensor.ps_i2c_state = PS_I2C_TaskState::RECEIVE_0;
                    break;

                case PS_I2C_TaskState::TRANSMIT_READ_1:
                    pressure_sensor.ps_i2c_state = PS_I2C_TaskState::RECEIVE_1;
                    break;

                case PS_I2C_TaskState::TRANSMIT_READ_2:
                    pressure_sensor.ps_i2c_state = PS_I2C_TaskState::RECEIVE_2;
                    break;

                case PS_I2C_TaskState::TRANSMIT_READ_3:
                    pressure_sensor.ps_i2c_state = PS_I2C_TaskState::RECEIVE_3;
                    break;

                case PS_I2C_TaskState::TRANSMIT_CONV_0:
                    pressure_sensor.ps_i2c_state = PS_I2C_TaskState::TCA_SET_1;
                    break;

                case PS_I2C_TaskState::TRANSMIT_CONV_1:
                    pressure_sensor.ps_i2c_state = PS_I2C_TaskState::TCA_SET_2;
                    break;

                case PS_I2C_TaskState::TRANSMIT_CONV_2:
                    pressure_sensor.ps_i2c_state = PS_I2C_TaskState::TCA_SET_3;
                    break;

                case PS_I2C_TaskState::TRANSMIT_CONV_3:
                    pressure_sensor.ps_i2c_state = PS_I2C_TaskState::TCA_SET_0;
                    break;

                default:
                    break;
            }
            if (pressure_sensor.ps_i2c_state == PS_I2C_TaskState::TCA_SET_0){
                i2c_state = I2C_TaskState::PROPELLER;
            }
            else{
                pressure_sensor.PS_I2C_IT_Handle();
            }
            break;
        
        case I2C_TaskState::PROPELLER:
            if (propeller_i2c.pro_i2c_state == PRO_I2C_TaskState::TCA_SET){
                propeller_i2c.pro_i2c_state = PRO_I2C_TaskState::SET_PWM;
                NOP_Delay(2000); // 等待TCA稳定
            }
            else {
                propeller_i2c.pro_i2c_state = PRO_I2C_TaskState::TCA_SET;
                i2c_state = I2C_TaskState::SERVO;
            }
            break;

        case I2C_TaskState::SERVO:
            i2c_state = I2C_TaskState::PressureSensor;
            break;

        }
    }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C2){
        switch (i2c_state)
        {
        case I2C_TaskState::PressureSensor:
            switch (pressure_sensor.ps_i2c_state) 
            {
                case PS_I2C_TaskState::RECEIVE_0:
                    pressure_sensor.ps_i2c_state = PS_I2C_TaskState::TRANSMIT_CONV_0;
                    break;

                case PS_I2C_TaskState::RECEIVE_1:
                    pressure_sensor.ps_i2c_state = PS_I2C_TaskState::TRANSMIT_CONV_1;
                    break;

                case PS_I2C_TaskState::RECEIVE_2:
                    pressure_sensor.ps_i2c_state = PS_I2C_TaskState::TRANSMIT_CONV_2;
                    break;

                case PS_I2C_TaskState::RECEIVE_3:
                    pressure_sensor.ps_i2c_state = PS_I2C_TaskState::TRANSMIT_CONV_3;
                    break;

                default:
                    break;
            }
            pressure_sensor.PS_I2C_IT_Handle();
            break;
        
        default:
            break;
        }
    }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C2)
    {
        // uint32_t err  = HAL_I2C_GetError(hi2c);
        SendInt(hi2c->ErrorCode, 1);
        SendInt((int)pressure_sensor.ps_i2c_state, 1);
        // 重置I2C外设状态
        HAL_I2C_DeInit(hi2c);
        HAL_I2C_Init(hi2c);
        // 重置状态机到初始状态，避免卡死
        // pressure_sensor.ps_i2c_state = PS_I2C_TaskState::TCA_SET_0;
        // i2c_state = I2C_TaskState::PressureSensor;
    }
}
