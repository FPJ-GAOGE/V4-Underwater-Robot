#ifndef FILTER_BASE_H
#define FILTER_BASE_H

template<typename T>
class FilterBase {
public:
    virtual T update(T input) = 0;  // 每个滤波器必须实现的核心函数
    virtual void reset() {}         // 可选复位函数
    virtual ~FilterBase() {}        // 虚析构，防止内存泄漏
};

template<typename T, int N>
class MeanFilter : public FilterBase<T> {
private:
    T buffer[N];
    int index = 0;
    T sum = 0;
    int count = 0;   // 当前有效样本数
public:
    MeanFilter() {
        for (int i = 0; i < N; i++) buffer[i] = 0;
    }

    T update(T input) override {
        sum -= buffer[index];
        buffer[index] = input;
        sum += input;
        index = (index + 1) % N;
        if (count < N) count++;  // 逐步填满
        return sum / count;      // 用有效样本数量除
    }

    void reset() override {
        sum = 0; index = 0;
        for (int i = 0; i < N; i++) buffer[i] = 0;
    }
};

template<typename T>
class LowPassFilter : public FilterBase<T> {
private:
    T alpha;
    T last_output;
public:
    LowPassFilter(T alpha_init = 0.1f) : alpha(alpha_init), last_output(0) {}

    T update(T input) override {
        last_output = alpha * input + (1 - alpha) * last_output;
        return last_output;
    }

    void setAlpha(T a) { alpha = a; }
    void reset() override { last_output = 0; }
};

#endif