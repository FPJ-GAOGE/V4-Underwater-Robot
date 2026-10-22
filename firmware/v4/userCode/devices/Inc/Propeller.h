//
// Created by admin on 2023/10/30.
//

#ifndef CONTROL_FRAME_MAIN_PWM_H
#define CONTROL_FRAME_MAIN_PWM_H

#include "Usermain.h"
#include "Extension.h"
#include "PID.h"
#include <array>
#include <functional>
#include "FeedforwardController.h"
#include "ESOObserver.h"

#define FORCE2PWM_FACTOR 20         // 
#define MAX_DELTA_THRUST 0.2f      // N
#define MAX_LONGITUDINAL_FORCE 20.f // 最大纵向推力，N
#define MAX_LATERAL_FORCE 20.f      // 最大横向推力，N
#define MAX_ROTATE_SPEED 0.8f        // 最大旋转速度，单位弧度每秒
#define MAX_DEPTH_SPEED 10.f        // 最大深度方向速度

// Fx(forward),Fy(right),Fz(down),Mx(roll),My(pitch),Mz(yaw)
constexpr float thrust_alloc_factor[8][6] = {
    { 0.3534,  0.3534,      0,       0,       0,  0.9843},
    {      0,       0, 0.2499, -1.7881, -2.2267,       0},
    {      0,       0, 0.2499, -1.7881,  2.2267,       0},
    { 0.3534, -0.3534,      0,       0,       0,  0.9843},
    { 0.3534,  0.3534,      0,       0,       0, -0.9843},
    {      0,       0, 0.2499,  1.7881,  2.2267,       0},
    {      0,       0, 0.2499,  1.7881, -2.2267,       0},
    { 0.3534, -0.3534,      0,       0,       0, -0.9843}
};

// Fx(forward);Fy(right);Fz(down);Mx(roll);My(pitch);Mz(yaw)
constexpr float thrust_calc_factor[6][8] = {
    {0.7071,      0,      0,  0.7071, 0.7071,     0,     0,  0.7071},
    {0.7071,      0,      0, -0.7071, 0.7071,     0,     0, -0.7071},
    {     0,      1,      1,       0,      0,     1,     1,       0},
    {     0, -0.138, -0.138,       0,      0, 0.138, 0.138,       0},
    {     0,  -0.11,   0.11,       0,      0,  0.11, -0.11,       0},
    { 0.253,      0,      0,   0.253, -0.253,     0,     0,  -0.253}
};

typedef struct Target{
    float ang[3] = {0.0f, 0.0f, 0.0f};     // roll, pitch, yaw
    float ang_vel[3] = {0.0f, 0.0f, 0.0f}; // roll, pitch, yaw
    float ang_acc[3] = {0.0f, 0.0f, 0.0f}; // roll, pitch, yaw
    float thrust[3] = {0.0f, 0.0f, 0.0f};  // roll, pitch, yaw
    float torque[3] = {0.0f, 0.0f, 0.0f};  // roll, pitch, yaw
    float thrust_single[PROPELLER_NUM] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float thrust_single_prev[PROPELLER_NUM] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float depth = 0.0f;
}Target_t;

typedef struct TorqueComponent{
    float PID[3] = {0.0f, 0.0f, 0.0f}, FF[3] = {0.0f, 0.0f, 0.0f}, ESO[3] = {0.0f, 0.0f, 0.0f}; // roll, pitch, yaw
    float depth = 0.0f;
}TorqueComponent_t;

enum class PRO_I2C_TaskState{
    TCA_SET,
    SET_PWM,
    Length
};

class Propeller_I2C;

typedef void (Propeller_I2C::*CommandHandler)();

class Propeller_I2C: public Device
{
private:
    float Target_speed[3];
    bool flag_on;
    bool flag_rotate = false;
    bool flag_depth = false;
    bool flag_PWM_output;
    float torque_act[3] = {0.0f, 0.0f, 0.0f};
    uint8_t i2c_it_tx[1+4*PROPELLER_NUM];
    uint8_t i2c_it_data;
    float thr_tau = 0.15f; // 推力一阶滞后时间常数，单位秒
    float thr_K   = 1.0f; // 模型增益 K（以“推力指令单位”到 N 的等效比值）
    float imc_lambda = 0.5f; // IMC 缓和系数(0,1]，越小越稳，越大响应越快
    float T_hat[PROPELLER_NUM] = {0};     // 内模推力估计
    float a_coef = 0.0f; // 一阶滞后系数 a
    float b_coef = 0.0f; // 一阶滞后系数 b

    PID DepthPID_; // 深度->推力
    PID RollInPID_, PitchInPID_, YawInPID_; // 角速度->力矩
    PID RollOutPID_, PitchOutPID_, YawOutPID_; // 角度->角速度

    void ComputeThrustTorque();
    void AllocateThrust();
    void Thrust2PWM();
    void SetPWM();
    void CommandTest();
    void CommandSetYawCloseloop();
    void CommandSetYawValue();
    void CommandSetYawIncrement();
    void CommandSetYawRate();
    void CommandSetDepthValue();
    void CommandSetPlanarVelocity();
    void CommandSetFloatCloseloop();
    void CommandSetHorizontalStop();
    void CommandSetDepthIncrement();
    void CommandSetDepthForce();

public:
    float yaw_diff;
    float yaw_v_diff;
    float pitch_diff;
    float pitch_v_diff;
    float roll_diff;
    float roll_v_diff;
    int16_t current_pwm[PROPELLER_NUM]; // PWM值，1500为不转，1500-2000正转，1000-1500反转，离1500越远转速越快
    Target_t target;
    TorqueComponent_t torque_comp;
    FeedforwardController ff[3]; // roll, pitch, yaw
    ESOObserver eso[3]; // roll, pitch, yaw
    PRO_I2C_TaskState pro_i2c_state;

    void Init();
    void Handle();
    void Receive();
    void Reset();

    void PRO_I2C_IT_Handle();
};


/*extern void Anglectrl_servo();

extern void Speedctrl_propeller_init();

extern void Speedctrl_propeller();

extern void Anglectrl_servo_init();
*/

extern Propeller_I2C propeller_i2c;

#endif //CONTROL_FRAME_MAIN_PWM_H
