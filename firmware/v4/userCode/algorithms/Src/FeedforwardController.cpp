#include "FeedforwardController.h"

FeedforwardController::FeedforwardController()
    : wref_prev_(0.0f), alpha_ref_(0.0f), has_prev_(false) {}

void FeedforwardController::init(const float J, const float B, const float D) {
    J_ = J;
    B_ = B;
    D_ = D;
    wref_prev_ = 0.0f;
    has_prev_ = false;
}

void FeedforwardController::reset() {
    wref_prev_ = 0.0f;
    has_prev_ = false;
}

float FeedforwardController::computeTorque(float w_ref) {
    // 差分得到 α_ref 原始值
    float alpha_raw = 0.0f;
    if (has_prev_) {
        alpha_raw = (w_ref - wref_prev_) * CONTROL_FREQUENCY;
    }
    wref_prev_ = w_ref;
    has_prev_ = true;

    // 一阶低通滤波
    alpha_ref_ += 0.8 * (alpha_raw - alpha_ref_);

    // 计算惯性与阻尼前馈
    float tau_inertia = J_ * alpha_ref_;
    float tau_linear  = B_ * w_ref;
    float tau_quad    = D_ * fabsf(w_ref) * w_ref;

    return tau_inertia + tau_linear + tau_quad;
}
