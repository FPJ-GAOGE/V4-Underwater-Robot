#include "Utils.h"

float NormalizeAngle(float angle)
{
    angle = std::fmod(angle + PI, 2 * PI);
    if (angle < 0) angle += 2 * PI;
    return angle - PI;
}

float deg2rad(float degree){
    return degree * PI / 180;
}

float rad2deg(float rad){
    return rad * 180 / PI;
}