//
// Created by admin on 2023/9/19.
//

#ifndef EXTENDED_KALMAN_FILTER_H
#define EXTENDED_KALMAN_FILTER_H

#include <string>
#include <vector>
#include <cmath>
#include "Usermain.h"

#define X_SIZE 4
#define Z_SIZE 8

class ExtendedKalmanFilter {
public:
     ExtendedKalmanFilter(float process_noise[X_SIZE][X_SIZE], float measurement_noise[Z_SIZE][Z_SIZE], float estimation_error[X_SIZE][X_SIZE]) {
        for (uint8_t i = 0; i < X_SIZE; i++) {
            for (uint8_t j = 0; j < X_SIZE; j++) {
                Q[i][j] = process_noise[i][j];
                P[i][j] = estimation_error[i][j];
            }
        }
        for (uint8_t i = 0; i < Z_SIZE; i++) {
            for (uint8_t j = 0; j < Z_SIZE; j++) {
                R[i][j] = measurement_noise[i][j];
            }
        }
    }
    void update(float q[4], float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, bool have_pres, float hA, float hB, float hC, float hD);

private:
    float Q[X_SIZE][X_SIZE];       // 过程噪声方差
    float R[Z_SIZE][Z_SIZE];       // 测量噪声方差
    float P[X_SIZE][X_SIZE];       // 估计误差协方差
    float dt = 0.0f;
    uint32_t last_dwt_cnt = 0;
};

void q_normalization_update(float q[4], float P[4][4]);
uint8_t inverse_matrix(float a[Z_SIZE][Z_SIZE], float a_inv[Z_SIZE][Z_SIZE]);

#endif
