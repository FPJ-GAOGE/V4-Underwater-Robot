#ifndef ESO_H
#define ESO_H

#include <algorithm>
#include <cmath>
#include "Usermain.h"

/**
 * 二阶 ESO（扩展状态观测器）
 *
 * 对象（单轴）：
 *   J·ω̇ = τ_act − B·ω − D·|ω|·ω + τ_ext
 * 令 u_lin = τ_act − B·ω − D·|ω|·ω，则
 *   ω̇ = (1/J)·u_lin + d, 其中 d = τ_ext / J
 *
 * 观测器状态：
 *   x1 ≈ ω     （角速度估计）
 *   x2 ≈ d     （外扰/不确定项的等效项，单位为 1/s^2）
 *
 * 输出换算：
 *   τ̂_ext = J * x2
 *
 * 重要：update 调用时，tau_act 必须为“实际作用到该轴的力矩”，即经过分配、死区补偿、饱和、功率限幅、滞后补偿之后。
 */
class ESOObserver {
public:

    ESOObserver();

    /** 初始化（重复调用会重置内部状态） */
    void init(const float J, const float B, const float D, const float omega_o);

    /** 重置状态（x1,x2 置零） */
    void reset();

    /**
     * 单步更新（前向欧拉离散）：
     * @param w_meas  测量角速度（rad/s），建议先 20~40 Hz 低通
     * @param tau_act     实际作用到对象的力矩（N·m）
     */
    void update(float w_meas, float tau_act);

    /**
     * 取外扰力矩估计（N·m）
     * τ̂_ext = J * x2
     */
    float getDisturbTorque() const;

    /** 可选：取估计的角速度（ω̂），便于诊断 */
    float getEstimatedRate() const { return x1_; }

    /** 可选：取 ESO 状态（x2 ≈ τ_ext/J） */
    float getEstimatedDisturbPerJ() const { return x2_; }

private:
    // 参数与离散系数
    float J_      = 1.0f;
    float B_      = 0.0f;
    float D_      = 0.0f;
    float b_      = 1.0f;       // 输入增益（通常=1/J）
    float omega_o_= 100.0f;     // 带宽 [rad/s]
    float beta1_  = 200.0f;     // 2*omega_o
    float beta2_  = 10000.0f;   // omega_o^2

    // 状态
    float x1_ = 0.0f;  // ω̂
    float x2_ = 0.0f;  // d̂ ≈ τ_ext/J

    void updateBetasFromOmegaO();
};

#endif