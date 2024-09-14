#ifndef PID_H
#define PID_H
#include "struct_typedef.h"

#define CURRENT_MAX  3000.0f
#define CURRENT_MIN  50.0f



#define POS_MAX  7000.0f
#define POS_MIN  250.0f

typedef struct{

	float target;
	float feedback;
	
	float kp;
	float ki;
	float kd;
	
	float integral;
	float err[3];

	float output;

}PID;




void PID_init(PID *pid_speed,PID *pid_pos,float init_pid_speed[3],float init_pid_pos[3]);
float PID_Cal(PID *pid,float fdbck,float set);
void Current_Limit(float motor_current_output[4]);
void Speed_Limit(float motor_pos_output[4]);




#endif
