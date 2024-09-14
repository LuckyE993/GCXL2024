#include "PID.h"



void PID_init(PID *pid_speed,PID *pid_pos,float init_pid_speed[3],float init_pid_pos[3])
{
	
	pid_speed->kp=init_pid_speed[0];
	pid_speed->ki=init_pid_speed[1];
	pid_speed->kd=init_pid_speed[2];
	pid_speed->output=0.0f;
	pid_speed->err[0]=pid_speed->err[1]=pid_speed->err[2]=0.0f;
	pid_speed->integral=0.0f;
//	pid_speed->kperr[0]=pid_speed->kperr[1]=pid_speed->kperr[2]=0.0f;

	pid_pos->kp=init_pid_pos[0];
	pid_pos->ki=init_pid_pos[1];
	pid_pos->kd=init_pid_pos[2];	
	pid_pos->output=0.0f;
	pid_pos->err[0]=pid_pos->err[1]=pid_pos->err[2]=0.0f;
	pid_pos->integral=0.0f;

//	pid_pos->kperr[0]=pid_pos->kperr[1]=pid_pos->kperr[2]=0.0f;
	
}





float PID_Cal(PID *pid,float fdbck,float set)
{
	
	pid->err[2] = pid->err[1];
	pid->err[1] = pid->err[0];	
	pid->target = set;
	pid->feedback = fdbck;	
	pid->err[0] = set - fdbck;
	pid->integral+=pid->err[0];
	pid->output =pid->kp*pid->err[0]+pid->ki*pid->integral+pid->kd*(pid->err[0]-pid->err[1]);

//µçÁ÷ÏÞ·ù
	
	return pid->output;
}



void Current_Limit(float motor_current_output[4])
{         
	for(uint8_t i=0;i<4;i++)
	{
        if (motor_current_output[i] > CURRENT_MAX)                             
            motor_current_output[i] = CURRENT_MAX;                           
        if (motor_current_output[i] < -CURRENT_MAX)                       
            motor_current_output[i] = -CURRENT_MAX;      
	   				
	}				
	
}

void Speed_Limit(float motor_pos_output[4])
{         
	for(uint8_t i=0;i<4;i++)
	{
        if (motor_pos_output[i] > POS_MAX)                            
            motor_pos_output[i] = POS_MAX;                         
         if (motor_pos_output[i] < -POS_MAX)                       
            motor_pos_output[i] = -POS_MAX;   
        if (motor_pos_output[i] > 0&&motor_pos_output[i] < POS_MIN)                            
            motor_pos_output[i] = POS_MIN;   
        if (motor_pos_output[i] < 0&&motor_pos_output[i] > -POS_MIN)                            
            motor_pos_output[i] = -POS_MIN;  				
     
	}				
	
}



