//
// Created by admin on 2023/12/5.
//

#ifndef CONTROL_FRAME_MAIN_SERVO_H
#define CONTROL_FRAME_MAIN_SERVO_H

#include "Usermain.h"
#include "Extension.h"
#include "PID.h"
#include <cstdint>

class Servo : public Device
{

public:
    int32_t current_pwm[SERVO_NUM]; // PWM值，500-2500对应0-180度，线性关系
    void data_extract(uint8_t *rx, int32_t *current_pwm, int32_t num);

    void Init();
    void Handle();
    void Receive();
};

extern Servo servo;

class Servo_I2C : public Device
{
private:
    uint8_t i2c_it_tx[1+4*SERVO_NUM];

public:
    int32_t ID[SERVO_NUM];   // 舵机编号
    int32_t current_pwm[SERVO_NUM]; // PWM值，500-2500对应0-180度，线性关系
    void data_extract(uint8_t *rx, int32_t *current_pwm, int32_t num);

    void Init();
    void Handle();
    void Receive();
    void SER_I2C_IT_Handle();
    void CommandStartMotor();
    void CommandStopMotor();
};

extern Servo_I2C servo_i2c;

typedef void (Servo_I2C::*ServoCommandHandler)();

#endif // CONTROL_FRAME_MAIN_SERVO_H
