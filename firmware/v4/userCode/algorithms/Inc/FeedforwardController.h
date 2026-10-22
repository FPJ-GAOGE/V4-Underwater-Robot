#ifndef FEEDFORWARD_H
#define FEEDFORWARD_H

#include <cmath>
#include <algorithm>
#include "Usermain.h" 

/**
 * @brief 前馈控制器类（惯性 + 阻尼）
 * 
 * τ_ff = J * α_ref + B * ω_ref + D * |ω_ref| * ω_ref
 * α_ref 由 w_ref 的差分得到并经过一阶低通滤波与限幅。
 */
class FeedforwardController {
public:

    FeedforwardController();

    /** 初始化参数 */
    void init(const float J, const float B, const float D);

    /** 重置状态 */
    void reset();

    /**
     * @brief 计算前馈输出
     * @param w_ref 期望角速度 [rad/s]
     * @return τ_ff 前馈力矩 [N·m]
     */
    float computeTorque(float w_ref);

private:
    float J_; // 惯量
    float B_; // 线性阻尼系数
    float D_; // 二次阻尼系数 (|w|w)
    float alpha_ref_;
    float wref_prev_;
    bool has_prev_;
};

#endif