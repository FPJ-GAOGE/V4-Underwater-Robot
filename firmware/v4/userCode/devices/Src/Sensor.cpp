#include "Sensor.h"
#include "IMU.h"
#include "KalmanFilter.h"

extern I2C_HandleTypeDef hi2c2;

KalmanFilter Pressure_Kf[4] = {KalmanFilter(0.01, 0.05, 1.0, 0.0),
                               KalmanFilter(0.01, 0.05, 1.0, 0.0),
                               KalmanFilter(0.01, 0.05, 1.0, 0.0),
                               KalmanFilter(0.01, 0.05, 1.0, 0.0)};

uint8_t adc_rd = MS5837_30BA_ADC_RD;
uint8_t command_temp = MS5837_30BA_D2_OSR4096;
uint8_t command_pres = MS5837_30BA_D1_OSR4096;
LowPassFilter<float> lowpassfilters[SENSOR_NUM] = {
    LowPassFilter<float>(0.7f),
    LowPassFilter<float>(0.7f),
    LowPassFilter<float>(0.7f),
    LowPassFilter<float>(0.7f)
};

void PressureSensor::Init()
{

    //------TODO:4个水压计三维坐标，由测量得到，需要修改
    Sensor_Site_t _site = {
        .x = {-12, 12, 12, -12},
        .y = {-5.4, -5.4, 5.4, 5.4},
        .z = {0, 0, 0, 0}
    };
    site = _site;

    CurrentID = 0;

    // flag_Busy = 0;

    // bsp_flash_read(&flashData);

    for (int i = 0; i < SENSOR_NUM; ++i)
    {
        TCA_SetChannel(i);
        HAL_Delay(5);
        Init_single(i);
    }

    data_plane[0] = 0;
    data_plane[1] = 0;
    data_plane[2] = 1;
    data_plane[3] = 0;
    data_plane[4] = 0;

    flag_calibrate = false;
    flag_read_pres = false;
}

void PressureSensor::Init_single(int id)
{
    data_pressure_offset[id] = 0;
    while (!flag_ok[id])
    {
        MS5837_30BA_PROM(id);
        flag_ok[id] = MS5837_30BA_Crc4(id);
    }

    switch (Robot_Version)
    {
    case V30:
        data_pressure_offset[0] = PRESSURE_0_V30;
        data_pressure_offset[1] = PRESSURE_1_V30;
        data_pressure_offset[2] = PRESSURE_2_V30;
        data_pressure_offset[3] = PRESSURE_3_V30;
        break;
    case V31:
        data_pressure_offset[0] = PRESSURE_0_V31;
        data_pressure_offset[1] = PRESSURE_1_V31;
        data_pressure_offset[2] = PRESSURE_2_V31;
        data_pressure_offset[3] = PRESSURE_3_V31;
        break;
    case V32:
        data_pressure_offset[0] = PRESSURE_0_V32;
        data_pressure_offset[1] = PRESSURE_1_V32;
        data_pressure_offset[2] = PRESSURE_2_V32;
        data_pressure_offset[3] = PRESSURE_3_V32;
		break;
    case V33:
        data_pressure_offset[0] = PRESSURE_0_V33;
        data_pressure_offset[1] = PRESSURE_1_V33;
        data_pressure_offset[2] = PRESSURE_2_V33;
        data_pressure_offset[3] = PRESSURE_3_V33;
		break;
    case V40:
        data_pressure_offset[0] = PRESSURE_0_V40;
        data_pressure_offset[1] = PRESSURE_1_V40;
        data_pressure_offset[2] = PRESSURE_2_V40;
        data_pressure_offset[3] = PRESSURE_3_V40;
		break;
    }

    // 预先请求一次温度
    for (int i = 0; i < SENSOR_NUM; ++i)
    {
        TCA_SetChannel(i);
        HAL_I2C_Master_Transmit(&hi2c2, B02_IIC_ADDRESS, &command_temp, 1, I2C_TO);
    }
    HAL_Delay(5);

    ps_state = PS_HandleState::READ_TEMP_THEN_REQU_PRES;
    ps_i2c_state = PS_I2C_TaskState::TCA_SET_0;
}

void PressureSensor::Handle()
{
    if (flag_calibrate)
    {
        Calibrate(); // 校准
        flag_calibrate = false;
    }
    else
    {
        Handle_all();
        // Handle_all_IT(); // I2C中断还要调试
    }
}

void PressureSensor::Receive()
{
    if (strncmp((char *)RxBuffer6, "CA", 2) == 0)
    {
        flag_calibrate = true;
    }
    else if (strncmp((char *)RxBuffer6, "VA", 2) == 0)
    {
        flag_read_pres = true;
    }
}

void PressureSensor::Calibrate()
{
    for (int i = 0; i < SENSOR_NUM; ++i)
    {
        TCA_SetChannel(i);
        // HAL_Delay(5);
        Calibrate_single(i);
    }
    // bsp_flash_write(&flashData);
}

void PressureSensor::Calibrate_single(int id)
{
    uint8_t TxBuffer[SENSOR_NUM * 9] = {0};
    data_pressure_offset[id] = 0;
    for (int j = 0; j < 100; ++j)
    {
        data_pressure_raw[id] = MS5837_30BA_GetData(id);
        data_pressure_offset[id] += data_pressure_raw[id] / 100;
    }
    if (id == 3)
        SendFloat(data_pressure_offset[id], 3, 1);
    else
        SendFloat(data_pressure_offset[id], 3, 0);
}

// 仿照Handle_single的思路，通过重叠I2C第一次请求后的等待时间实现整体效率提高
void PressureSensor::Handle_all()
{
    uint8_t rx[3];
    switch (ps_state) {
        case PS_HandleState::READ_TEMP_THEN_REQU_PRES:
            for (int i = 0; i < SENSOR_NUM; ++i) {
                // 转换TCA通道
                TCA_SetChannel(i);

                // 读取转换好的温度值
                HAL_I2C_Master_Transmit(&hi2c2, B02_IIC_ADDRESS, &adc_rd, 1, I2C_TO);
                HAL_I2C_Master_Receive(&hi2c2, B02_IIC_ADDRESS, rx, 3, I2C_TO);
                conv_D2[i] = ((uint32_t)rx[0] << 16) | ((uint32_t)rx[1] << 8) | rx[2];
                
                // 触发下一轮 D1（压力）转换
                HAL_I2C_Master_Transmit(&hi2c2, B02_IIC_ADDRESS, &command_pres, 1, I2C_TO);
            }

            ps_state = PS_HandleState::READ_PRES_THEN_REQU_TEMP;
            break;

        case PS_HandleState::READ_PRES_THEN_REQU_TEMP:
            for (int i = 0; i < SENSOR_NUM; ++i) {
                // 转换TCA通道
                TCA_SetChannel(i);
                HAL_I2C_Master_Transmit(&hi2c2, B02_IIC_ADDRESS, &adc_rd, 1, I2C_TO);
                HAL_I2C_Master_Receive(&hi2c2, B02_IIC_ADDRESS, rx, 3, I2C_TO);
                conv_D1[i] = ((uint32_t)rx[0] << 16) | ((uint32_t)rx[1] << 8) | rx[2];

                // 计算实际水深
                CalcRealPressure(i);

                // 触发下一轮 D2（温度）转换
                HAL_I2C_Master_Transmit(&hi2c2, B02_IIC_ADDRESS, &command_temp, 1, I2C_TO);
            }

            data_depth = (data_pressure[3] + data_pressure[2] + data_pressure[1] + data_pressure[0]) / 4;
            data_roll = -data_pressure[0] - data_pressure[1] + data_pressure[2] + data_pressure[3];
            data_pitch = -data_pressure[0] - data_pressure[3] + data_pressure[1] + data_pressure[2];

            // imu.flag_update_pressure = true;
            ps_state = PS_HandleState::READ_TEMP_THEN_REQU_PRES;
            break;
    }
    i2c_state = I2C_TaskState::PROPELLER;
}

void PressureSensor::Handle_single(int id)
{

    data_pressure_raw[id] = MS5837_30BA_GetData(id);
    float temp = data_pressure_raw[id] - data_pressure_offset[id];
    // temp = Pressure_Kf[id].update(temp); // 卡尔曼滤波
    data_pressure[id] = temp;
}

void PressureSensor::CalcRealPressure(uint8_t id)
{
    D1_Pres = conv_D1[id];
    D2_Temp = conv_D2[id];

    dT   = D2_Temp - (((uint32_t)Cal_C[id][5]) * 256l);
    SENS = (int64_t)Cal_C[id][1] * 65536l + ((int64_t)Cal_C[id][3] * dT) / 128l;
    OFF_ = (int64_t)Cal_C[id][2] * 131072l + ((int64_t)Cal_C[id][4] * dT) / 64l;

    TEMP = 2000l + (int64_t)(dT)*Cal_C[id][6] / 8388608LL;

    if (TEMP < 2000) { // 低温补偿
        Ti   = (11 * (int64_t)dT * (int64_t)dT) / (34359738368LL);
        OFFi = (31 * (TEMP - 2000) * (TEMP - 2000)) / 8;
        SENSi= (63 * (TEMP - 2000) * (TEMP - 2000)) / 32;
    } else {           // 高温补偿
        Ti   = 2 * (dT * dT) / (137438953472LL);
        OFFi = (1 * (TEMP - 2000) * (TEMP - 2000)) / 16;
        SENSi= 0;
    }
    OFF2 = OFF_ - OFFi;
    SENS2= SENS - SENSi;

    float pres = ((D1_Pres * SENS2) / 2097152.0 - OFF2) / 32768.0 / 100.0;

    // 去零偏
    data_pressure_raw[id] = pres;
    float val = data_pressure_raw[id] - data_pressure_offset[id];
    
    //滤波
    val = lowpassfilters[id].update(val);
    data_pressure[id] = val;
}

void PressureSensor::Handle_all_IT()
{   
    while(i2c_state != I2C_TaskState::PressureSensor){
        __NOP();
    }
    if (ps_i2c_state == PS_I2C_TaskState::TCA_SET_0){
        PS_I2C_IT_Handle();
    }
}

void PressureSensor::PS_I2C_IT_Handle()
{
    HAL_StatusTypeDef state;
    switch(ps_i2c_state)
    {
        case PS_I2C_TaskState::TCA_SET_0:
            i2c_it_data = 0x01 << 0;
            HAL_I2C_Master_Transmit_IT(&hi2c2, (TCA9548A_ADDR << 1), &i2c_it_data, 1);
            break;

        case PS_I2C_TaskState::TCA_SET_1:
            i2c_it_data = 0x01 << 1;
            HAL_I2C_Master_Transmit_IT(&hi2c2, (TCA9548A_ADDR << 1), &i2c_it_data, 1);
            break;

        case PS_I2C_TaskState::TCA_SET_2:
            i2c_it_data = 0x01 << 2;
            HAL_I2C_Master_Transmit_IT(&hi2c2, (TCA9548A_ADDR << 1), &i2c_it_data, 1);
            break;

        case PS_I2C_TaskState::TCA_SET_3:
            i2c_it_data = 0x01 << 3;
            HAL_I2C_Master_Transmit_IT(&hi2c2, (TCA9548A_ADDR << 1), &i2c_it_data, 1);
            break;

        case PS_I2C_TaskState::TRANSMIT_READ_0:
        case PS_I2C_TaskState::TRANSMIT_READ_1:
        case PS_I2C_TaskState::TRANSMIT_READ_2:
        case PS_I2C_TaskState::TRANSMIT_READ_3:
            HAL_I2C_Master_Transmit_IT(&hi2c2, B02_IIC_ADDRESS, &adc_rd, 1);
            break;

        case PS_I2C_TaskState::RECEIVE_0:
        case PS_I2C_TaskState::RECEIVE_1:
        case PS_I2C_TaskState::RECEIVE_2:
        case PS_I2C_TaskState::RECEIVE_3:
            HAL_I2C_Master_Receive_IT(&hi2c2, B02_IIC_ADDRESS, i2c_it_rx, 3);
            break;

        case PS_I2C_TaskState::TRANSMIT_CONV_0:
            if (ps_state == PS_HandleState::READ_TEMP_THEN_REQU_PRES){
                conv_D2[0] = ((uint32_t)i2c_it_rx[0] << 16) | ((uint32_t)i2c_it_rx[1] << 8) | i2c_it_rx[2];
                HAL_I2C_Master_Transmit_IT(&hi2c2, B02_IIC_ADDRESS, &command_pres, 1);
            }
            else{
                conv_D1[0] = ((uint32_t)i2c_it_rx[0] << 16) | ((uint32_t)i2c_it_rx[1] << 8) | i2c_it_rx[2];
                HAL_I2C_Master_Transmit_IT(&hi2c2, B02_IIC_ADDRESS, &command_temp, 1);
                CalcRealPressure(0);
            }
            break;

        case PS_I2C_TaskState::TRANSMIT_CONV_1:
            if (ps_state == PS_HandleState::READ_TEMP_THEN_REQU_PRES){
                conv_D2[1] = ((uint32_t)i2c_it_rx[0] << 16) | ((uint32_t)i2c_it_rx[1] << 8) | i2c_it_rx[2];
                HAL_I2C_Master_Transmit_IT(&hi2c2, B02_IIC_ADDRESS, &command_pres, 1);
            }
            else{
                conv_D1[1] = ((uint32_t)i2c_it_rx[0] << 16) | ((uint32_t)i2c_it_rx[1] << 8) | i2c_it_rx[2];
                HAL_I2C_Master_Transmit_IT(&hi2c2, B02_IIC_ADDRESS, &command_temp, 1);
                CalcRealPressure(1);
            }
            break;

        case PS_I2C_TaskState::TRANSMIT_CONV_2:
            if (ps_state == PS_HandleState::READ_TEMP_THEN_REQU_PRES){
                conv_D2[2] = ((uint32_t)i2c_it_rx[0] << 16) | ((uint32_t)i2c_it_rx[1] << 8) | i2c_it_rx[2];
                HAL_I2C_Master_Transmit_IT(&hi2c2, B02_IIC_ADDRESS, &command_pres, 1);
            }
            else{
                conv_D1[2] = ((uint32_t)i2c_it_rx[0] << 16) | ((uint32_t)i2c_it_rx[1] << 8) | i2c_it_rx[2];
                HAL_I2C_Master_Transmit_IT(&hi2c2, B02_IIC_ADDRESS, &command_temp, 1);
                CalcRealPressure(2);
            }
            break;

        case PS_I2C_TaskState::TRANSMIT_CONV_3:
            if (ps_state == PS_HandleState::READ_TEMP_THEN_REQU_PRES){
                conv_D2[3] = ((uint32_t)i2c_it_rx[0] << 16) | ((uint32_t)i2c_it_rx[1] << 8) | i2c_it_rx[2];
                HAL_I2C_Master_Transmit_IT(&hi2c2, B02_IIC_ADDRESS, &command_pres, 1);
                ps_state = PS_HandleState::READ_PRES_THEN_REQU_TEMP;
            }
            else{
                conv_D1[3] = ((uint32_t)i2c_it_rx[0] << 16) | ((uint32_t)i2c_it_rx[1] << 8) | i2c_it_rx[2];
                HAL_I2C_Master_Transmit_IT(&hi2c2, B02_IIC_ADDRESS, &command_temp, 1);
                CalcRealPressure(3);
                data_depth = (data_pressure[3] + data_pressure[2] + data_pressure[1] + data_pressure[0]) / 4;
                data_roll = -data_pressure[0] - data_pressure[1] + data_pressure[2] + data_pressure[3];
                data_pitch = -data_pressure[0] - data_pressure[3] + data_pressure[1] + data_pressure[2];
                ps_state = PS_HandleState::READ_TEMP_THEN_REQU_PRES;
            }
            break;
            
        default:
            break;
    }
}

void PressureSensor::Solve_plane_3(float *data, float *h, float *x, float *y, float *z)
{
    float pos_data[9];
    for (int i = 0; i < 3; ++i)
    {
        pos_data[i * 3] = x[i];
        pos_data[i * 3 + 1] = y[i];
        pos_data[i * 3 + 2] = z[i];
    }

    double a = 0, b = 0, c = 0, d = 0;
    Matrix pos(3, 3, pos_data);

    Matrix pos_inv;
    if (pos_inv.inv(pos))
    {

        float a1 = 0, a2 = 0, b1 = 0, b2 = 0, c1 = 0, c2 = 0, ax = 0, bx = 0, cx = 0;

        for (int i = 1; i < 4; ++i)
        {
            a1 -= pos_inv.mat[1][i];
            b1 -= pos_inv.mat[2][i];
            c1 -= pos_inv.mat[3][i];
            a2 -= pos_inv.mat[1][i] * h[i - 1];
            b2 -= pos_inv.mat[2][i] * h[i - 1];
            c2 -= pos_inv.mat[3][i] * h[i - 1];
        }

        ax = a1 * a1 + b1 * b1 + c1 * c1;
        bx = 2 * (a1 * a2 + b1 * b2 + c1 * c2);
        cx = a2 * a2 + b2 * b2 + c2 * c2 - 1;
        data[0] = (-bx - sqrt(bx * bx - 4 * ax * cx)) / (2 * ax);
        data[1] = a1 * d + a2;
        data[2] = b1 * d + b2;
        data[3] = c1 * d + c2;
    }
}

void PressureSensor::Solve_plane(float *data, float *h, float *x, float *y, float *z, int num)
{
    float xi = 0, yi = 0, zi = 0, hi = 0, xi2 = 0, yi2 = 0, zi2 = 0, xiyi = 0, xizi = 0, yizi = 0, xihi = 0, yihi = 0, zihi = 0;
    for (int i = 0; i < num; ++i)
    {
        xi += x[i];
        yi += y[i];
        zi += z[i];
        hi += h[i];
        xi2 += x[i] * x[i];
        yi2 += y[i] * y[i];
        zi2 += z[i] * z[i];
        xiyi += x[i] * y[i];
        xizi += x[i] * z[i];
        yizi += y[i] * z[i];
        xihi += x[i] * h[i];
        yihi += y[i] * h[i];
        zihi += z[i] * h[i];
    }
    float gradient[5] = {0};
    gradient[0] = xi2 * data[0] + xiyi * data[1] + xizi * data[2] + xi * data[3] + xihi + data[0] * data[4];
    gradient[1] = xiyi * data[0] + yi2 * data[1] + yizi * data[2] + yi * data[3] + yihi + data[1] * data[4];
    gradient[2] = xizi * data[0] + yizi * data[1] + zi2 * data[2] + zi * data[3] + zihi + data[2] * data[4];
    gradient[3] = xi * data[0] + yi * data[1] + zi * data[2] + num * data[3] + hi;
    gradient[4] = data[0] * data[0] + data[1] * data[1] + data[2] * data[2] - 1;

    float jacobi_data[25] = {xi2 + data[4], xiyi, xizi, xi, data[0],
                             xiyi, yi2 + data[4], yizi, yi, data[1],
                             xizi, yizi, zi2 + data[4], zi, data[2],
                             xi, yi, zi, (float)num, 0,
                             data[0], data[1], data[2], 0, 0};
    Matrix jacobi(5, 5, jacobi_data);
    Matrix jacobi_inv;
    if (jacobi_inv.inv(jacobi))
    {
        float data_dx[5] = {0};
        for (int i = 0; i < 5; ++i)
        {
            for (int j = 0; j < 5; ++j)
            {
                data_dx[i] += jacobi_inv.mat[i + 1][j + 1] * gradient[j];
            }
            data[i] -= 0.5 * data_dx[i];
        }
    }
}

void PressureSensor::Update_plane()
{
    Solve_plane(data_plane, data_pressure, site.x, site.y, site.z, SENSOR_NUM);

    for (int i = 0; i < 3; ++i)
    {
        data_level[i] = data_plane[i];
    }
}

unsigned char PressureSensor::MS5837_30BA_Crc4(int id)
{
    int cnt;
    int t;
    unsigned int n_rem = 0;
    unsigned char n_bit;
    unsigned char a = 0;
    unsigned char b = 0;
    unsigned short int n_prom[8];

    for (t = 0; t < 7; t++)
    {
        n_prom[t] = Cal_C[id][t];
    }
    n_prom[0] = ((n_prom[0]) & 0x0FFF);
    n_prom[7] = 0;
    for (cnt = 0; cnt < 16; cnt++)
    {
        if (cnt % 2 == 1)
            n_rem ^= (unsigned short)((n_prom[cnt >> 1]) & 0x00FF);
        else
            n_rem ^= (unsigned short)(n_prom[cnt >> 1] >> 8);
        for (n_bit = 8; n_bit > 0; n_bit--)
        {
            if (n_rem & (0x8000))
                n_rem = (n_rem << 1) ^ 0x3000;
            else
                n_rem = (n_rem << 1);
        }
    }
    n_rem = ((n_rem >> 12) & 0x000F);
    a = (n_rem ^ 0x00);
    b = Cal_C[id][0] >> 12;
    if (a == b)
    {
        return 1;
    }
    else
        return 0;
}

void PressureSensor::MS5837_30BA_ReSet(void)
{
    uint8_t data[1];
    data[0] = MS5837_30BA_ResetCommand;
    HAL_I2C_Master_Transmit(&hi2c2, B02_IIC_ADDRESS, data, 1, 0xffff);
}

uint8_t PressureSensor::MS5837_30BA_PROM(int id)
{
    uint8_t memaddr[1];
    uint8_t data[2];
    int i;

    MS5837_30BA_ReSet(); // ��λMS5837
    HAL_Delay(20);
    for (i = 0; i < 7; i++)
    {
        memaddr[0] = MS5837_30BA_PROM_RD + (i * 2);
        HAL_I2C_Master_Transmit(&hi2c2, B02_IIC_ADDRESS, memaddr, 1, 0xffff);
        HAL_I2C_Master_Receive(&hi2c2, B02_IIC_ADDRESS, data, 2, 0xffff);
        Cal_C[id][i] = (((uint16_t)data[0] << 8) | data[1]);
    }
    return !Cal_C[id][0];
}

unsigned long PressureSensor::MS5837_30BA_GetConversion(uint8_t command)
{
    unsigned long conversion = 0;
    uint8_t data = MS5837_30BA_ADC_RD;
    uint8_t temp[3];
    HAL_I2C_Master_Transmit(&hi2c2, B02_IIC_ADDRESS, &command, 1, 0xffff);
    HAL_Delay(10);
    HAL_I2C_Master_Transmit(&hi2c2, B02_IIC_ADDRESS, &data, 1, 0xffff);

    HAL_I2C_Master_Receive(&hi2c2, B02_IIC_ADDRESS, temp, 3, 0xffff);
    conversion = (unsigned long)temp[0] * 65536 + (unsigned long)temp[1] * 256 + (unsigned long)temp[2];
    return conversion;
}

/*signed int PressureSensor::MS5837_30BA_GetTemp(int id)
{
    if(flag_ok[id]) {
        D2_Temp = MS5837_30BA_GetConversion(MS5837_30BA_D2_OSR_8192);
        HAL_Delay(20);
        dT = D2_Temp - (((uint32_t) Cal_C[id][5]) * 256l);
        return dT;
    }
    else return -1;
}*/

float PressureSensor::MS5837_30BA_GetData(int id)
{

    if (flag_ok[id])
    {
        D2_Temp = MS5837_30BA_GetConversion(MS5837_30BA_D2_OSR1024);
        // HAL_Delay(1);
        D1_Pres = MS5837_30BA_GetConversion(MS5837_30BA_D1_OSR1024);
        // HAL_Delay(3);
        dT = D2_Temp - (((uint32_t)Cal_C[id][5]) * 256l);
        SENS = (int64_t)Cal_C[id][1] * 65536l + ((int64_t)Cal_C[id][3] * dT) / 128l;
        OFF_ = (int64_t)Cal_C[id][2] * 131072l + ((int64_t)Cal_C[id][4] * dT) / 64l;

        TEMP = 2000l + (int64_t)(dT)*Cal_C[id][6] / 8388608LL;

        // �����¶Ȳ���
        if (TEMP < 2000) // low temp
        {

            Ti = (11 * (int64_t)(dT) * (int64_t)(dT) / (34359738368LL));
            OFFi = (31 * (TEMP - 2000) * (TEMP - 2000)) / 8;
            SENSi = (63 * (TEMP - 2000) * (TEMP - 2000)) / 32;
        }
        else
        { // high temp
            Ti = 2 * (dT * dT) / (137438953472LL);
            OFFi = (1 * (TEMP - 2000) * (TEMP - 2000)) / 16;
            SENSi = 0;
        }
        OFF2 = OFF_ - OFFi;
        SENS2 = SENS - SENSi;
        /*if(D1_Pres==0||D2_Temp==0) return last_measure[id];
        else{
            float pressure = ((D1_Pres * SENS2) / 2097152.0 - OFF2) / 32768.0 / 100.0;
            last_measure[id]=pressure;
            return pressure;

        }*/
        float pressure = ((D1_Pres * SENS2) / 2097152.0 - OFF2) / 32768.0 / 100.0;
        return pressure;

        // data_pressure = ((D1_Pres * SENS2) / 2097152.0 - OFF2) / 32768.0 / 100.0;          //У׼��ѹ������

        // Temperature = (TEMP - Ti) / 100.0;                                //У׼���¶�����
    }
    else
        return -1;
}

void Sonar::Init()
{
    data_offset = 0;
    for (int i = 0; i < 100; ++i)
    {
        Handle();
        HAL_Delay(20);
        if (RxBuffer6[0] == 0xFF)
            data_offset += (RxBuffer6[1] * 256 + RxBuffer6[2]) / 100;
    }
}

void Sonar::Handle()
{
    // if(tim_id == 1){
    uint8_t rqst[1] = {' '};
    HAL_UART_Transmit_IT(&huart6, rqst, 1);
    //}
}

void Sonar::Receive()
{
    if (RxBuffer6[0] == 0xFF)
    {
        data = RxBuffer6[1] * 256 + RxBuffer6[2] - data_offset;
    }
    else
        data = -1;
}