//
// Created by admin on 2023/12/5.
//
#include "Servo.h"

int32_t ID_V30[4] = {8, 9, 10, 11}; // 4个舵机接在PWM扩展板的编号，左上-左下-右上-右下
int32_t ID_V31[4] = {8, 9, 10, 11};
int32_t ID_V32[4] = {8, 9, 11, 10};
int32_t ID_V33[4] = {8, 9, 10, 11};
int32_t ID_V40[4] = {8, 9, 10, 11};

static ServoCommandHandler servo_command[256] = {nullptr};

// int32_t InitPWM = 1500;

void Servo::Init()
{   
    for (int i = 0; i < SERVO_NUM; ++i)
    {
        current_pwm[i] = 1500;
    }
    __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_3, current_pwm[3]);
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, current_pwm[0]);
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_3, current_pwm[2]);
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_4, current_pwm[1]);
}

void Servo_I2C::Init()
{
    servo_command[0x00] = &Servo_I2C::CommandStartMotor;
    servo_command[0x01] = &Servo_I2C::CommandStopMotor;

    std::memcpy(ID, ID_V40, sizeof(ID));
    // TCA_SetChannel(4);
    current_pwm[0] = 1550;
    current_pwm[1] = 1500;
    current_pwm[2] = 1500;
    current_pwm[3] = 1550;
    for (int i = 0; i < SERVO_NUM; ++i)
    {
        // current_pwm[i] = 1500;
        PCA_Setpwm(i + 8, 0, floor(current_pwm[i] * 4096 / 20000 + 0.5f));
    }
}

void Servo::Receive()
{
    data_extract(RxBuffer6, current_pwm, SERVO_NUM);
}

void Servo_I2C::Receive()
{
    uint8_t code = RxBuffer6[1];
    if (servo_command[code] != nullptr) 
    {
        (this->*servo_command[code])(); // 调用对应的处理函数
    }
}

void Servo_I2C::CommandStartMotor()
{
    current_pwm[0] = 1650;
    current_pwm[3] = 1650;
}

void Servo_I2C::CommandStopMotor()
{
    current_pwm[0] = 1550;
    current_pwm[3] = 1550;
}
void Servo::Handle()
{
    __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_3, current_pwm[3]);
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, current_pwm[0]);
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_3, current_pwm[2]);
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_4, current_pwm[1]);
}

void Servo_I2C::Handle()
{
    // TCA_SetChannel(4);
    for (int i = 0; i < SERVO_NUM; ++i)
    {
        current_pwm[i] = clamp(current_pwm[i], 500, 2500);
    }
    // for (int i = 0; i < SERVO_NUM; ++i)
    // {
    //     PCA_Setpwm(ID[i], 0, floor(current_pwm[i] * 4096 / 20000 + 0.5f));
    // }
    // i2c_state = I2C_TaskState::PROPELLER;

    while(i2c_state != I2C_TaskState::SERVO){
        __NOP();
    }
    SER_I2C_IT_Handle();
}

void Servo::data_extract(uint8_t *rx, int32_t *current_pwm, int32_t num)
{
    if (strncmp((char *)rx, "MOT:", 4) == 0)
    {

        char *data_str = (char *)rx + 4;
        char *token = strtok(data_str, ",");
        int i = 0;
        while (token != NULL && i < num)
        {
            current_pwm[i] = atoi(token);
            token = strtok(NULL, ",");
            i++;
        }
    }
}

void Servo_I2C::data_extract(uint8_t *rx, int32_t *current_pwm, int32_t num)
{
    // 示例：假设 PWM 命令格式是 "MOT:1000,2000,1500,1800,1600,1400\n"
    if (strncmp((char *)rx, "MOT:", 4) == 0)
    {
        char *data_str = (char *)rx + 4;
        char *token = strtok(data_str, ",");
        int i = 0;
        while (token != NULL && i < num)
        {
            current_pwm[i] = atoi(token);
					  if (i == 2) {current_pwm[i] = 3000 - current_pwm[i];} // 这行新版没有
            token = strtok(NULL, ",");
            i++;
        }
    }
}

void Servo_I2C::SER_I2C_IT_Handle(){
    i2c_it_tx[0] = LED0_ON_L + 4*8;
    for (int i = 0; i < SERVO_NUM; i++){
        int16_t pwm = current_pwm[i];
        pwm = clamp(pwm, (int16_t)1000, (int16_t)2000);
        int16_t duty_count = (pwm * 4096 + 10000) / 20000;
        i2c_it_tx[4*i+1] = 0x00;
        i2c_it_tx[4*i+2] = 0x00;
        i2c_it_tx[4*i+3] = duty_count & 0xFF;
        i2c_it_tx[4*i+4] = duty_count >> 8;
    }
    HAL_I2C_Master_Transmit_IT(&hi2c2, PCA9685_ADDR, i2c_it_tx, 1+4*SERVO_NUM);
}