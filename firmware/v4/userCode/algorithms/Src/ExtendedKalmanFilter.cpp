#include <ExtendedKalmanFilter.h>
#include <MahonyAHRS.h>
#include <Sensor.h>
#include <vector>

//EKF融合IMU和水压计估算姿态
void ExtendedKalmanFilter::update(float q[4], float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, bool have_pres, float hA, float hB, float hC, float hD)
{
	//中间计算变量初始化
	float recipNorm;
	float q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;
	float mx_e, my_e, mz_e;
	float q0mxe, q0mye, q0mze, q1mxe, q1mye, q1mze, q2mxe, q2mye, q2mze, q3mxe, q3mye, q3mze;
	
	float F[X_SIZE][X_SIZE];
	float FP[X_SIZE][X_SIZE];
	float FPFt[X_SIZE][X_SIZE];
	float P_Pred[X_SIZE][X_SIZE];
	float q_Pred[X_SIZE];
	
	float H[Z_SIZE][X_SIZE];
	float HPp[X_SIZE];
	float HPpHt;
	float S;
	float PpHt[X_SIZE];
	float K[X_SIZE];
	
	float z_Pred[Z_SIZE];
	float z_Measure[Z_SIZE];
	float z_bias[Z_SIZE];
	
	float KH[X_SIZE][X_SIZE];
	float ImKH[X_SIZE][X_SIZE];
	float ImKHPp[X_SIZE][X_SIZE];
	float ImKHPpImKHt[X_SIZE][X_SIZE];
	float KR[X_SIZE];
	float KRKt[X_SIZE][X_SIZE];
	
	uint8_t z_Num = Z_SIZE;

	uint32_t now_dwt_cnt = GetDT();
	uint32_t interval_cnt = now_dwt_cnt - last_dwt_cnt;
	last_dwt_cnt = now_dwt_cnt;
	dt = ConvertDT(interval_cnt);

	// Normalise accelerometer measurement
	recipNorm = invSqrt(ax * ax + ay * ay + az * az);
	ax *= recipNorm;
	ay *= recipNorm;
	az *= recipNorm;     

	// Normalise magnetometer measurement
	recipNorm = invSqrt(mx * mx + my * my + mz * mz);
	mx *= recipNorm;
	my *= recipNorm;
	mz *= recipNorm; 
	
    // 预测步骤	
	F[0][0] = 1.0f;
	F[0][1] = -0.5f * dt * gx;
	F[0][2] = -0.5f * dt * gy;
	F[0][3] = -0.5f * dt * gz;
	
	F[1][0] = 0.5f * dt * gx;
	F[1][1] = 1.0f;
	F[1][2] = 0.5f * dt * gz;
	F[1][3] = -0.5f * dt * gy;
	
	F[2][0] = 0.5f * dt * gy;
	F[2][1] = -0.5f * dt * gz;
	F[2][2] = 1.0f;
	F[2][3] = 0.5f * dt * gx;
	
	F[3][0] = 0.5f * dt * gz;
	F[3][1] = 0.5f * dt * gy;
	F[3][2] = -0.5f * dt * gx;
	F[3][3] = 1.0f;
	
	//计算预测的四元数
	q_Pred[0] = F[0][0]*q[0] + F[0][1]*q[1] + F[0][2]*q[2] + F[0][3]*q[3];
	q_Pred[1] = F[1][0]*q[0] + F[1][1]*q[1] + F[1][2]*q[2] + F[1][3]*q[3];
	q_Pred[2] = F[2][0]*q[0] + F[2][1]*q[1] + F[2][2]*q[2] + F[2][3]*q[3];
	q_Pred[3] = F[3][0]*q[0] + F[3][1]*q[1] + F[3][2]*q[2] + F[3][3]*q[3];
	
	
	//更新协方差矩阵
	for (uint8_t i = 0; i < X_SIZE; i++) 
	{
		for (uint8_t j = 0; j < X_SIZE; j++) 
		{
			FP[i][j] = F[i][0] * P[0][j] + F[i][1] * P[1][j] + F[i][2] * P[2][j] + F[i][3] * P[3][j];
		}
	}
	
	for (uint8_t i = 0; i < X_SIZE; i++) 
	{
		for (uint8_t j = 0; j < X_SIZE; j++) 
		{
			FPFt[i][j] = FP[i][0] * F[j][0] + FP[i][1] * F[j][1] + FP[i][2] * F[j][2] + FP[i][3] * F[j][3];
		}
	}

	for (uint8_t i = 0; i < X_SIZE; i++) 
	{
		for (uint8_t j = 0; j < X_SIZE; j++) 
		{
			P_Pred[i][j] = FPFt[i][j] + Q[i][j];
		}
	}
	
	//四元数归一化
	q_normalization_update(q_Pred, P_Pred);
	
	// 更新步骤
	// Auxiliary variables to avoid repeated arithmetic
	q0q0 = q_Pred[0] * q_Pred[0];
	q0q1 = q_Pred[0] * q_Pred[1];
	q0q2 = q_Pred[0] * q_Pred[2];
	q0q3 = q_Pred[0] * q_Pred[3];
	q1q1 = q_Pred[1] * q_Pred[1];
	q1q2 = q_Pred[1] * q_Pred[2];
	q1q3 = q_Pred[1] * q_Pred[3];
	q2q2 = q_Pred[2] * q_Pred[2];
	q2q3 = q_Pred[2] * q_Pred[3];
	q3q3 = q_Pred[3] * q_Pred[3];   

	// Reference direction of Earth's magnetic field
	mx_e = 2.0f * (mx * (0.5f - q2q2 - q3q3) + my * (q1q2 - q0q3) + mz * (q1q3 + q0q2));
	my_e = 2.0f * (mx * (q1q2 + q0q3) + my * (0.5f - q1q1 - q3q3) + mz * (q2q3 - q0q1));
	mz_e = 2.0f * (mx * (q1q3 - q0q2) + my * (q2q3 + q0q1) + mz * (0.5f - q1q1 - q2q2));
	
	q0mxe = q_Pred[0] * mx_e;
	q0mye = q_Pred[0] * my_e;
	q0mze = q_Pred[0] * mz_e;
	q1mxe = q_Pred[1] * mx_e;
	q1mye = q_Pred[1] * my_e;
	q1mze = q_Pred[1] * mz_e;
	q2mxe = q_Pred[2] * mx_e;
	q2mye = q_Pred[2] * my_e;
	q2mze = q_Pred[2] * mz_e;
	q3mxe = q_Pred[3] * mx_e;
	q3mye = q_Pred[3] * my_e;
	q3mze = q_Pred[3] * mz_e;
	
	//计算H
	H[0][0] = -2.0f * q_Pred[2];
	H[0][1] = 2.0f * q_Pred[3];
	H[0][2] = -2.0f * q_Pred[0];
	H[0][3] = 2.0f * q_Pred[1];
	
	H[1][0] = 2.0f * q_Pred[1];
	H[1][1] = 2.0f * q_Pred[0];
	H[1][2] = 2.0f * q_Pred[3];
	H[1][3] = 2.0f * q_Pred[2];
	
	H[2][0] = 2.0f * q_Pred[0];
	H[2][1] = -2.0f * q_Pred[1];
	H[2][2] = -2.0f * q_Pred[2];
	H[2][3] = 2.0f * q_Pred[3];
	
	H[3][0] = 2.0f * (q0mxe + q3mye - q2mze);
	H[3][1] = 2.0f * (q1mxe + q2mye + q3mze);
	H[3][2] = 2.0f * (-q2mxe + q1mye - q0mze);
	H[3][3] = 2.0f * (-q3mxe + q0mye + q1mze);

	H[4][0] = 2.0f * (-q3mxe + q0mye + q1mze);
	H[4][1] = 2.0f * (q2mxe - q1mye + q0mze);
	H[4][2] = 2.0f * (q1mxe + q2mye + q3mze);
	H[4][3] = 2.0f * (-q0mxe - q3mye + q2mze);
	
	H[5][0] = 2.0f * (q2mxe - q1mye + q0mze);
	H[5][1] = 2.0f * (q3mxe - q0mye - q1mze);
	H[5][2] = 2.0f * (q0mxe + q3mye - q2mze);
	H[5][3] = 2.0f * (q1mxe + q2mye + q3mze);
	
	H[6][0] = H[0][0];
	H[6][1] = H[0][1];
	H[6][2] = H[0][2];
	H[6][3] = H[0][3];
	
	H[7][0] = H[1][0];
	H[7][1] = H[1][1];
	H[7][2] = H[1][2];
	H[7][3] = H[1][3];
	
	//计算z_bias
	z_Pred[0] = 2.0f * (q1q3 - q0q2);
	z_Pred[1] = 2.0f * (q2q3 + q0q1);
	z_Pred[2] = 2.0f * (q0q0 + q3q3 - 0.5f);
	z_Pred[3] = 2.0f * ((q0q0 + q1q1 - 0.5f)*mx_e + (q1q2 + q0q3)*my_e + (q1q3 - q0q2)*mz_e);
	z_Pred[4] = 2.0f * ((q1q2 - q0q3)*mx_e + (q0q0 + q2q2 - 0.5f)*my_e + (q2q3 + q0q1)*mz_e);
	z_Pred[5] = 2.0f * ((q1q3 + q0q2)*mx_e + (q2q3 - q0q1)*my_e + (q0q0 + q3q3 - 0.5f)*mz_e);
	z_Pred[6] = 2.0f * (q1q3 - q0q2);
	z_Pred[7] = 2.0f * (q2q3 + q0q1);
	
	z_Measure[0] = ax;
	z_Measure[1] = ay;
	z_Measure[2] = az;
	z_Measure[3] = mx;
	z_Measure[4] = my;
	z_Measure[5] = mz;
	z_Measure[6] = (hA + hD - hC - hB) / (2.0f * PITCH_SEN_DIST);
	z_Measure[7] = (hA + hB - hC - hD) / (2.0f * ROLL_SEN_DIST);
	
	for(uint8_t i = 0; i < Z_SIZE; i++)
	{
		z_bias[i] = z_Measure[i] - z_Pred[i];
	}
	
	if (have_pres)
	{//水压计数据更新
		z_Num = Z_SIZE;
	}
	else
	{//水压计数据未更新
		z_Num = Z_SIZE - 2;
	}
	
	for(uint8_t k = 0; k < z_Num; k++)
	{//序贯滤波，用每一个观测变量依次更新四元数
		
		//计算卡尔曼增益Kk
		//H*P_Pred
		for(uint8_t i = 0; i < X_SIZE; i++)
		{
			HPp[i] = H[k][0]*P_Pred[0][i] + H[k][1]*P_Pred[1][i] + H[k][2]*P_Pred[2][i] + H[k][3]*P_Pred[3][i];
		}
		//H*P_Pred*Ht
		HPpHt = HPp[0]*H[k][0] + HPp[1]*H[k][1] + HPp[2]*H[k][2] + HPp[3]*H[k][3];
		//S=H*P_Pred*Ht+R
		S = HPpHt + R[k][k];
		//P_Pred*Ht
		for(uint8_t i = 0; i < X_SIZE; i++)
		{
			PpHt[i] = P_Pred[i][0]*H[k][0] + P_Pred[i][1]*H[k][1] + P_Pred[i][2]*H[k][2] + P_Pred[i][3]*H[k][3];
		}
		//K=P_Pred*Hkt*S_inv
		for(uint8_t i = 0; i < X_SIZE; i++)
		{
			K[i] = PpHt[i] / S;
		}
		
		//x = xp + K*z_bias
		for(uint8_t i = 0; i< X_SIZE; i++)
		{
			q_Pred[i] = q_Pred[i] + K[i] * z_bias[k];
		}
		// P = (I- KH)*Pp*(I-KH)t+KRKt;
		// K*H	
		for(uint8_t i = 0; i < X_SIZE; i++)
		{
			for(uint8_t j = 0; j < X_SIZE; j++)
			{
				KH[i][j] = K[i] * H[k][j];
			}
		}
		//ImKH
		for (uint8_t i = 0; i < X_SIZE; i++) 
		{
			for (uint8_t j = 0; j < X_SIZE; j++) 
			{
				ImKH[i][j] = -KH[i][j];
				if(i==j)
				{
					ImKH[i][j]++; 
				}
			}
		}
		//ImKH*Pp
		for (uint8_t i = 0; i < X_SIZE; i++) 
		{
			for (uint8_t j = 0; j < X_SIZE; j++) 
			{
				ImKHPp[i][j] = ImKH[i][0]*P_Pred[0][j] + ImKH[i][1]*P_Pred[1][j] + ImKH[i][2]*P_Pred[2][j] + ImKH[i][3]*P_Pred[3][j];
			}
		}
		//ImKH*Pp*(ImKH)t
		for (uint8_t i = 0; i < X_SIZE; i++) 
		{
			for (uint8_t j = 0; j < X_SIZE; j++) 
			{
				ImKHPpImKHt[i][j] = ImKHPp[i][0]*ImKH[j][0] + ImKHPp[i][1]*ImKH[j][1] + ImKHPp[i][2]*ImKH[j][2] + ImKHPp[i][3]*ImKH[j][3];
			}
		}
		//K*R
		for (uint8_t i = 0; i < X_SIZE; i++) 
		{
			KR[i] = K[i] * R[k][k];
		}
		//K*R*Kt
		for (uint8_t i = 0; i < X_SIZE; i++) 
		{
			for (uint8_t j = 0; j < X_SIZE; j++) 
			{
				KRKt[i][j] = KR[i]*K[j];
			}
		}
		//P
		for (uint8_t i = 0; i < X_SIZE; i++) 
		{
			for (uint8_t j = 0; j < X_SIZE; j++) 
			{
				P_Pred[i][j] = ImKHPpImKHt[i][j] + KRKt[i][j];
			}
		}
		q_normalization_update(q_Pred, P_Pred);
	}
	for(uint8_t i = 0; i < X_SIZE; i++)
	{
		q[i] = q_Pred[i];
		for(uint8_t j = 0; j < X_SIZE; j++)
		{
			P[i][j] = P_Pred[i][j];
		}
	}
}

void q_normalization_update(float q[4], float P[4][4])
{//四元数归一化后，更新状态协方差矩阵
	float recipNorm;
	float J[X_SIZE][X_SIZE];
	float JP[X_SIZE][X_SIZE];
	
	//归一化四元数
	recipNorm = invSqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
	q[0] *= recipNorm;
	q[1] *= recipNorm;
	q[2] *= recipNorm;
	q[3] *= recipNorm;
	//归一化过程的误差传播矩阵（雅可比矩阵）
	for (uint8_t i = 0; i < X_SIZE; i++) 
	{
        for (uint8_t j = 0; j < X_SIZE; j++) 
		{
            if (i == j) 
			{
                J[i][j] = (1.0f - q[i] * q[i]) * recipNorm;
            } 
			else 
			{
                J[i][j] = (-q[i] * q[j]) * recipNorm;
            }
        }
    }
	//更新归一化后的状态协方差矩阵P
	for (uint8_t i = 0; i < X_SIZE; i++) 
	{
		for (uint8_t j = 0; j < X_SIZE; j++) 
		{
			JP[i][j] = J[i][0] * P[0][j] + J[i][1] * P[1][j] + J[i][2] * P[2][j] + J[i][3] * P[3][j];
		}
	}
	
	for (uint8_t i = 0; i < X_SIZE; i++) 
	{
		for (uint8_t j = 0; j < X_SIZE; j++) 
		{
			P[i][j] = JP[i][0] * J[j][0] + JP[i][1] * J[j][1] + JP[i][2] * J[j][2] + JP[i][3] * J[j][3];
		}
	}
}


uint8_t inverse_matrix(float a[Z_SIZE][Z_SIZE], float a_inv[Z_SIZE][Z_SIZE]) 
{//方阵求逆
	// Initialize matrix
	for (uint8_t i = 0; i < Z_SIZE; i++) 
	{
		for (uint8_t j = 0; j < Z_SIZE; j++) 
		{
			if (i != j)
				a_inv[i][j] = 0;
			else
				a_inv[i][j] = 1;
		}
	}

	// Applying Gauss Jordan Elimination
	float ratio = 1.0;
	for (uint8_t i = 0; i < Z_SIZE; i++)
	{
		if (a[i][i] == 0.0)
		{
			return 0;
		}
		for (uint8_t j = 0; j < Z_SIZE; j++)
		{
			if (i != j)
			{
				ratio = a[j][i] / a[i][i];
				for (uint8_t k = 0; k < 6; k++)
				{
					a[j][k] = a[j][k] - ratio * a[i][k];
					a_inv[j][k] = a_inv[j][k] - ratio * a_inv[i][k];
				}
			}
		}
	}
	// Row Operation to Make Principal Diagonal to 1
	for (uint8_t i = 0; i < Z_SIZE; i++)
	{
		for (uint8_t j = 0; j < Z_SIZE; j++)
		{
			a_inv[i][j] = a_inv[i][j] / a[i][i];
		}
	}
	return 1;
}

