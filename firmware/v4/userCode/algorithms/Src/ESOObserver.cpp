#include "ESOObserver.h"

ESOObserver::ESOObserver() = default;

void ESOObserver::init(const float J, const float B, const float D, const float omega_o) {
    J_ = J;
    B_ = B;
    D_ = D;

    b_ = 1.0f / J_;

    omega_o_ = omega_o;

    updateBetasFromOmegaO();

    x1_ = 0.0f;
    x2_ = 0.0f;
}

void ESOObserver::reset() {
    x1_ = 0.0f;
    x2_ = 0.0f;
}

void ESOObserver::update(float w_meas, float tau_act) {
    // 误差
    const float e = w_meas - x1_;

    // 构造 u_lin = τ_act − B*ω − D*|ω|*ω  （单位：N·m）
    const float damping = B_ * w_meas + D_ * fabsf(w_meas) * w_meas;
    const float u_lin   = tau_act - damping;

    // 离散更新（前向欧拉）
    const float dx1 = x2_ + b_ * u_lin + beta1_ * e;
    const float dx2 = beta2_ * e;

    x1_ += dx1 / CONTROL_FREQUENCY;
    x2_ += dx2 / CONTROL_FREQUENCY;
}

float ESOObserver::getDisturbTorque() const {
    // x2 ≈ τ_ext / J  → τ̂_ext = J * x2
    return J_ * x2_;
}

void ESOObserver::updateBetasFromOmegaO() {
    // 二阶 ESO：beta1 = 2*ωo, beta2 = ωo^2
    beta1_ = 2.0f * omega_o_;
    beta2_ = omega_o_ * omega_o_;
}
