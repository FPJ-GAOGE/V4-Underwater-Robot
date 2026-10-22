/*******************************************************************************
* Copyright (c) 2025.
* IWIN-FINS Lab, Shanghai Jiao Tong University, Shanghai, China.
* All rights reserved.
******************************************************************************/

#ifndef V5_SUB_HPP
#define V5_SUB_HPP

#define PRESSURE_SENSORS_INIT_SAMPLE_TIMES 20
#define MOTOR_COUNT 8
#define PRESSURE_SENSOR_COUNT 4

#include "FillLight/SUBLAB_FillLight.hpp"
#include "ESC_Motors/ESC_Motor.hpp"
#include "PressureSensor/PressureSensorBase.hpp"
#include "IMUBase.hpp"
#include <array>
#include <cstring>
#include <cmath>
#include "Clamp.hpp"
#include "arm_math.h"
#include "Matrix/matrix.h"
#include "Control/PID.hpp"
#include "Verification/CRC.h"
#include "Bus/UART_Base.hpp"

volatile float ls_depth = 0.0f;
volatile float ls_target = 0.0f;

struct SUB_Motor_t {
   ESC_Motor *motor;
   float lx;
   float ly;
   float lz;
};

struct SUB_Pressure_Sensor_t {
   PressureSensorBase *sensor;
   float lx;
   float ly;
   float lz;
};

class V5_SUB;

class V5_SUBBuilder {
public:
   V5_SUBBuilder() = default;

   V5_SUBBuilder& AddMotor(ESC_Motor* motor, float x, float y, float z) {
       if (motorCount < MOTOR_COUNT) {
           motors[motorCount++] = {motor, x, y, z};
       }
       return *this;
   }

   V5_SUBBuilder& AddPressureSensor(PressureSensorBase* sensor, float x, float y, float z) {
       if (sensorCount < PRESSURE_SENSOR_COUNT) {
           pressureSensors[sensorCount++] = {sensor, x, y, z};
       }
       return *this;
   }

   V5_SUBBuilder& AddIMU(IMUBase* imu_) {
       imu = imu_;
       return *this;
   }

   V5_SUBBuilder& AddController(ControllerBase* rollController_,
                                ControllerBase* pitchController_,
                                ControllerBase* yawController_,
                                ControllerBase* deepthController_) {
       rollController = rollController_;
       pitchController = pitchController_;
       yawController = yawController_;
       deepthController = deepthController_;
       return *this;
   }

   V5_SUB* Build();

   std::array<SUB_Motor_t, MOTOR_COUNT> motors{};
   size_t motorCount = 0;

   std::array<SUB_Pressure_Sensor_t, PRESSURE_SENSOR_COUNT> pressureSensors{};
   size_t sensorCount = 0;

   IMUBase* imu = nullptr;

   ControllerBase* rollController = nullptr;
   ControllerBase* pitchController = nullptr;
   ControllerBase* yawController = nullptr;
   ControllerBase* deepthController = nullptr;
};

class V5_SUB : public DeviceBase {
public:
    bool GetIMUInitFinished() const { return imuInitFinished; }
    bool GetPressureInitFinished() const { return pressureInitFinished; }
    uint16_t GetPressureSensorOffsetSampleTimes() const { return pressureSensorOffsetSampleTimes; }
    uint8_t GetStateValue() const { return static_cast<uint8_t>(state); }   explicit V5_SUB(V5_SUBBuilder const &temp) {
       motors = temp.motors;
       pressureSensors = temp.pressureSensors;
       imu = temp.imu;
       rollController = temp.rollController;
       pitchController = temp.pitchController;
       yawController = temp.yawController;
       deepthController = temp.deepthController;

       if (rollController)  rollController->SetFeedback({&currentAngleRate[0][0]});
       if (pitchController) pitchController->SetFeedback({&currentAngleRate[1][0]});
       if (yawController)   yawController->SetFeedback({&currentAngleRate[2][0]});
       if (deepthController) deepthController->SetFeedback({&currentDeepth});

       if (rollController)  rollController->SetTarget(&targetAngleRate[0][0]);
       if (pitchController) pitchController->SetTarget(&targetAngleRate[1][0]);
       if (yawController)   yawController->SetTarget(&targetAngleRate[2][0]);
       if (deepthController) deepthController->SetTarget(&targetDeepth);

       A = Matrixf<4, 3>{
           motors[1].ly, -motors[1].lx, -1.0f,
           motors[2].ly, -motors[2].lx, -1.0f,
           motors[5].ly, -motors[5].lx, -1.0f,
           motors[6].ly, -motors[6].lx, -1.0f
       };

       B = Matrixf<4, 3>{
           1.0f, -1.0f,  1.0f,
           -1.0f,  1.0f,  1.0f,
           1.0f,  1.0f, -1.0f,
           -1.0f, -1.0f, -1.0f
       };
   }

   void EnableMotors() {
       for (auto &motor : motors) {
           if (motor.motor != nullptr) {
               motor.motor->EnableMotor();
           }
       }
   }

   void DisableMotors() {
       for (auto &motor : motors) {
           if (motor.motor != nullptr) {
               motor.motor->DisableMotor();
           }
       }
   }

   void Disarm() {
       state = SUB_STATE::STABILIZING;
   }

   void Command(float yawRate, float FBSpeed, float LRSpeed, float downSpeed, float upSpeed, uint8_t startButton) {
       targetDeepth += (downSpeed - upSpeed) * 0.001f * 0.1f;
       targetDeepth = Clamp(targetDeepth, 0.0f, 15.0f);
       ls_target=targetDeepth;

       targetAttitude[2][0] += yawRate * 0.001f * 0.4f;
       targetVel[0][0] = FBSpeed * 0.35f;
       targetVel[1][0] = LRSpeed * 0.35f;

       SwitchState(startButton);
   }

   void Handle() final {
       Update();

       switch (state) {
       case SUB_STATE::INIT:
           PressureSensorInit();
           IMUInit();
           if (pressureInitFinished && imuInitFinished) {
               state = SUB_STATE::IDLE;
           }
           break;

       case SUB_STATE::IDLE:
           break;

       case SUB_STATE::STABILIZING:
           StabilizeMode();
           if (rollController)  rollController->Calc();
           if (pitchController) pitchController->Calc();
           if (yawController)   yawController->Calc();
           if (deepthController) deepthController->Calc();
           MotorPowerDistribution();
           break;

       case SUB_STATE::POSITION_HOLDING:
           PositionHoldMode();
           break;

       default:
           break;
       }

       static uint8_t cnt = 0;
       cnt++;
       if (cnt >= 10) {
           cnt = 0;
           uploadData.roll = currentAttitude[0][0];
           uploadData.pitch = currentAttitude[1][0];
           uploadData.yaw = currentAttitude[2][0];
           uploadData.deepth = currentDeepth;
           uploadData.targetDeepth = targetDeepth;
           uploadData.currentRollRate = currentAngleRate[0][0];
           uploadData.currentPitchRate = currentAngleRate[1][0];
           uploadData.currentYawRate = currentAngleRate[2][0];
           uploadData.targetRollRate = targetAngleRate[0][0];
           uploadData.targetPitchRate = targetAngleRate[1][0];
           uploadData.targetYawRate = targetAngleRate[2][0];
           uploadData.targetPitch = targetAttitude[2][0];
           uploadData.pitchOutput = pitchController ? pitchController->GetOutput() : 0.0f;
           uploadData.targetFBvel = targetVel[0][0];
           uploadData.targetLRvel = targetVel[1][0];

           memcpy(txBuffer, &uploadData, 64);
           if (HAL_GetTick() > 30000) {
               UART_Base<3>::GetInstance().Transmit(txBuffer, 64);
           }
       }
   }

private:
   enum class SUB_STATE {
       INIT = 0X00,
       IDLE = 0X01,
       STABILIZING = 0X02,
       POSITION_HOLDING = 0X03
   } state = SUB_STATE::INIT;

   struct UploadData {
       float roll{0.0f};
       float pitch{0.0f};
       float yaw{0.0f};
       float deepth{0.0f};
       float targetDeepth{0.0f};
       float currentRollRate{0.0f};
       float currentPitchRate{0.0f};
       float currentYawRate{0.0f};
       float targetRollRate{0.0f};
       float targetPitchRate{0.0f};
       float targetYawRate{0.0f};
       float targetPitch{0.0f};
       float pitchOutput{0.0f};
       float targetFBvel{0.0f};
       float targetLRvel{0.0f};
       uint8_t tail[4]{0x00, 0x00, 0x80, 0x7f};
   } uploadData __packed;

   uint8_t txBuffer[100]{};

   std::array<SUB_Motor_t, MOTOR_COUNT> motors{};
   std::array<SUB_Pressure_Sensor_t, PRESSURE_SENSOR_COUNT> pressureSensors{};
   IMUBase *imu = nullptr;

   std::array<float, PRESSURE_SENSOR_COUNT> pressureSensorOffset = {0.0f, 0.0f, 0.0f, 0.0f};
   std::array<float, PRESSURE_SENSOR_COUNT> pressureSensorOffsetSum = {0.0f, 0.0f, 0.0f, 0.0f};
   uint16_t pressureSensorOffsetSampleTimes = 0;

   bool pressureInitFinished = false;
   bool imuInitFinished = false;

   Matrixf<3, 1> targetAttitude{};
   Matrixf<3, 1> targetAngleRate{};
   Matrixf<3, 1> currentAngleRate{};
   Matrixf<3, 1> currentAttitude{};
   Matrixf<3, 1> rollPitchDeepthControllerOutput{};
   Matrixf<3, 1> yawFbLrControllerOutput{};
   Matrixf<4, 3> A{};
   Matrixf<4, 3> B{};
   Matrixf<2, 1> targetVel{};

   float targetDeepth{0.0f};
   float currentDeepth{0.0f};

   ControllerBase* rollController = nullptr;
   ControllerBase* pitchController = nullptr;
   ControllerBase* yawController = nullptr;
   ControllerBase* deepthController = nullptr;

private:
   void PressureSensorInit() {
       if (pressureInitFinished) {
           return;
       }

       std::array<float, PRESSURE_SENSOR_COUNT> currentPressures = {0.0f, 0.0f, 0.0f, 0.0f};

       for (size_t i = 1; i < PRESSURE_SENSOR_COUNT; i=i+2) {
           if (pressureSensors[i].sensor == nullptr) {
               return;
           }

           float p = pressureSensors[i].sensor->GetPressure();

           // MS5837_TCA_Channel::GetPressure() 返回绝对压力 Pa
           if (p < 50000.0f) {
               return;
           }

           currentPressures[i] = p;
       }

       for (size_t i = 0; i < PRESSURE_SENSOR_COUNT; i++) {
           pressureSensorOffsetSum[i] += currentPressures[i];
       }
       pressureSensorOffsetSampleTimes++;

       if (pressureSensorOffsetSampleTimes >= PRESSURE_SENSORS_INIT_SAMPLE_TIMES) {
           for (size_t i = 0; i < PRESSURE_SENSOR_COUNT; i++) {
               pressureSensorOffset[i] =
                   pressureSensorOffsetSum[i] / static_cast<float>(PRESSURE_SENSORS_INIT_SAMPLE_TIMES);
           }

           currentDeepth = 0.0f;
           targetDeepth = 0.0f;
           pressureInitFinished = true;
       }
   }

   void IMUInit() {
       if (imuInitFinished) {
           return;
       }
       if (imu != nullptr && imu->IsDataGet()) {
           imuInitFinished = true;
           targetAttitude[2][0] = imu->getEulerArray()[2];
       }
   }

   float SqrtControler(float error, float errorLimit, float accLimit, float p) {
       if (error > errorLimit) {
           error = errorLimit;
       } else if (error < -errorLimit) {
           error = -errorLimit;
       }

       if (errorLimit < 1e-6f) {
           return error * p;
       } else if (p < 1e-6f) {
           if (error > 1e-6f) {
               return sqrtf(2.0f * accLimit * error);
           } else if (error < -1e-6f) {
               return -sqrtf(-2.0f * accLimit * error);
           } else {
               return 0.0f;
           }
       } else {
           float linear_dist = accLimit / (p * p);

           if (error > linear_dist) {
               return sqrtf(2.0f * accLimit * (error - (linear_dist / 2.0f)));
           } else if (error < -linear_dist) {
               return -sqrtf(2.0f * accLimit * (-error - (linear_dist / 2.0f)));
           } else {
               return error * p;
           }
       }
   }

   void Update() {
       if (imu != nullptr) {
           Matrixf<3,1> attitudeError = targetAttitude - imu->getEulerArray().data();
           targetAngleRate[0][0] = SqrtControler(attitudeError[0][0], 2.0f, 1.0f, 5.0f);
           targetAngleRate[1][0] = SqrtControler(attitudeError[1][0], 2.0f, 1.0f, 5.0f);
           targetAngleRate[2][0] = SqrtControler(attitudeError[2][0], 2.0f, 1.0f, 5.0f);
           currentAngleRate = imu->getAngleRateArray().data();
           currentAttitude = imu->getEulerArray().data();
       }

       float p1 = 0.0f;
       float p3 = 0.0f;

       if (pressureSensors[1].sensor != nullptr) {
           p1 = pressureSensors[1].sensor->GetPressure() - pressureSensorOffset[1];
       }
       if (pressureSensors[3].sensor != nullptr) {
           p3 = pressureSensors[3].sensor->GetPressure() - pressureSensorOffset[3];
       }

       float waterPressure = (p1 + p3) * 0.5f;
       if (waterPressure < 0.0f) {
           waterPressure = 0.0f;
       }

       currentDeepth = waterPressure / 9.8f / 1000.0f;
       ls_depth = currentDeepth;
   }

   void MotorPowerDistribution() {
       float rollOutput   = Clamp(1.0f * (rollController ? rollController->GetOutput() : 0.0f),   -0.2f, 0.2f);
       float pitchOutput  = Clamp(1.0f * (pitchController ? pitchController->GetOutput() : 0.0f), -0.2f, 0.2f);
       float yawOutput    = Clamp(1.0f * (yawController ? yawController->GetOutput() : 0.0f),     -0.2f, 0.2f);
       float deepthOutput = Clamp(1.0f * (deepthController ? deepthController->GetOutput() : 0.0f), -0.5f, 0.5f);

       rollPitchDeepthControllerOutput[0][0] = rollOutput;
       rollPitchDeepthControllerOutput[1][0] = pitchOutput;
       rollPitchDeepthControllerOutput[2][0] = deepthOutput;

       Matrixf<4,1> upperMotorOutPut = A * rollPitchDeepthControllerOutput;
       upperMotorOutPut[0][0] = fabsf(upperMotorOutPut[0][0]) < 0.01f ? 0.0f : upperMotorOutPut[0][0];
       upperMotorOutPut[1][0] = fabsf(upperMotorOutPut[1][0]) < 0.01f ? 0.0f : upperMotorOutPut[1][0];
       upperMotorOutPut[2][0] = fabsf(upperMotorOutPut[2][0]) < 0.01f ? 0.0f : upperMotorOutPut[2][0];
       upperMotorOutPut[3][0] = fabsf(upperMotorOutPut[3][0]) < 0.01f ? 0.0f : upperMotorOutPut[3][0];

       yawFbLrControllerOutput[0][0] = yawOutput;
       yawFbLrControllerOutput[1][0] = targetVel[0][0];
       yawFbLrControllerOutput[2][0] = targetVel[1][0];

       Matrixf<4,1> lowerMotorOutPut = B * yawFbLrControllerOutput;
       lowerMotorOutPut[0][0] = fabsf(lowerMotorOutPut[0][0]) < 0.01f ? 0.0f : lowerMotorOutPut[0][0];
       lowerMotorOutPut[1][0] = fabsf(lowerMotorOutPut[1][0]) < 0.01f ? 0.0f : lowerMotorOutPut[1][0];
       lowerMotorOutPut[2][0] = fabsf(lowerMotorOutPut[2][0]) < 0.01f ? 0.0f : lowerMotorOutPut[2][0];
       lowerMotorOutPut[3][0] = fabsf(lowerMotorOutPut[3][0]) < 0.01f ? 0.0f : lowerMotorOutPut[3][0];

       if (motors[1].motor) motors[1].motor->SetThrottle(-upperMotorOutPut[0][0]);
       if (motors[2].motor) motors[2].motor->SetThrottle(upperMotorOutPut[1][0]);
       if (motors[5].motor) motors[5].motor->SetThrottle(-upperMotorOutPut[2][0]);
       if (motors[6].motor) motors[6].motor->SetThrottle(-upperMotorOutPut[3][0]);

       if (motors[0].motor) motors[0].motor->SetThrottle(lowerMotorOutPut[0][0]);
       if (motors[3].motor) motors[3].motor->SetThrottle(-lowerMotorOutPut[1][0]);
       if (motors[4].motor) motors[4].motor->SetThrottle(-lowerMotorOutPut[2][0]);
       if (motors[7].motor) motors[7].motor->SetThrottle(lowerMotorOutPut[3][0]);
   }

   void StabilizeMode() {
   }

   void PositionHoldMode() {
   }

   void SwitchState(uint8_t buttonState) {
       static uint8_t lastButtonState = 0;
       if (buttonState != lastButtonState && buttonState == 1) {
           if (state == SUB_STATE::IDLE) {
               EnableMotors();
               targetAttitude[2][0] = currentAttitude[2][0];
               state = SUB_STATE::STABILIZING;
           } else if (state == SUB_STATE::STABILIZING) {
               DisableMotors();
               state = SUB_STATE::IDLE;
           }
       }
       lastButtonState = buttonState;
   }
};

inline V5_SUB* V5_SUBBuilder::Build() {
   return new V5_SUB(*this);
}

#endif // V5_SUB_HPP