#ifndef PARAM_CONFIG_H
#define PARAM_CONFIG_H

// 系统
#define CONTROL_FREQUENCY 100       // 更新频率
#define MAX_SERIAL_LENGTH 128       // 串口指令最大长度
#define SENSOR_NUM 4                // 水压计数量
#define SERVO_NUM 4                 // 舵机数量
#define PROPELLER_NUM 8             // 推进器数量
#define LED_PERIOD 500              // 亮灯周期
#define ROLL_SEN_DIST 14.0          // 计算roll方向误差角的水压计间距，用于从深度误差计算角度误差，单位cm
#define PITCH_SEN_DIST 33.65        // 计算pitch方向误差角的水压计间距，用于从深度误差计算角度误差，单位cm
#define I2C_TO 10                   // I2C超时

/*
 * 设备类型枚举
 */
typedef enum {
    IM, // imu
    PS, // pressure sensor
    PR, // propeller
    SE, // servo
    LE  // led
} DEVICE_TYPE_E;

typedef enum{
    V30,
    V31,
    V32,
    V33,
    V40
} VERSION_E;

// IMU

// 推进器


#endif