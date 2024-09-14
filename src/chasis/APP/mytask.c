#include "mytask.h"
#include "cmsis_os.h"
#include "main.h"
#include "CAN_receive.h"
#include "usart.h"


PID 		pid_speed;
PID 		pid_pos;
float  	tar_pos=720;
float motor_current_output[4]={0};	
float motor_Speed_output[4]={0};
float speed[4]={0};

float x=0;
float y=0;
float yaw=0;

void motor_task (void const *pvParameters)
{
	
	vTaskDelay(3000);
	float init_pid_speed[3]={1.5,0.0008,0.0001};//1.5,0.0008,0.0001
	float init_pid_pos[3]  ={6,0,1}; // 3,0.0001,1
	
	uint8_t id=0;
	
	err_ecd_get();
	
	PID_init(&pid_speed,&pid_pos,init_pid_speed,init_pid_pos);
	
	vTaskDelay(50);

	uint32_t currentTime;

	while(1)
	{
		
		currentTime = xTaskGetTickCount();	
	
		speed[0]=-x-y-yaw;
		speed[1]= x-y-yaw;		
		speed[2]= x+y-yaw;		
		speed[3]=-x+y-yaw;	
		
		for(id=0;id<4;id++)
		{
			

			motor_current_output[id]=PID_Cal(&pid_speed,motor_chassis[id].speed_rpm, speed[id]); //ËÙ¶È»·	
			Current_Limit(motor_current_output);
		}
		CAN_cmd_chassis(motor_current_output[0],motor_current_output[1],motor_current_output[2],motor_current_output[3]);
			
		vTaskDelayUntil(&currentTime, 2);	

	
	}
}

