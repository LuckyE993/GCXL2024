/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       can_receive.c/h
  * @brief      there is CAN interrupt function  to receive motor data,
  *             and CAN send function to send motor current to control motor.
  *             ������CAN�жϽ��պ��������յ������,CAN���ͺ������͵���������Ƶ��.
  * @note       
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Dec-26-2018     RM              1. done
  *  V1.1.0     Nov-11-2019     RM              1. support hal lib
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2019 DJI****************************
  */

#include "CAN_receive.h"

#include "cmsis_os.h"

#include "main.h"

#include "mytask.h"
//#include "detect_task.h"

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

float KalmanFilter(KFPTypeS *kfp, float input);





//motor data read
#define get_motor_measure(ptr, data)                                    \
    {                                                                   \
        (ptr)->last_ecd = (ptr)->ecd;                                   \
        (ptr)->ecd = (uint16_t)((data)[0] << 8 | (data)[1]);            \
        (ptr)->speed_rpm = (uint16_t)((data)[2] << 8 | (data)[3]);      \
        (ptr)->given_current = (uint16_t)((data)[4] << 8 | (data)[5]);  \
    }
/*
motor data,  0:chassis motor1 2006;1:chassis motor3 2006;2:chassis motor3 2006;3:chassis motor4 2006;
4:yaw gimbal motor 6020;5:pitch gimbal motor 6020;6:trigger motor 2006;
�������, 0:���̵��1 2006���,  1:���̵��2 2006���,2:���̵��3 2006���,3:���̵��4 2006���;
4:yaw��̨��� 6020���; 5:pitch��̨��� 6020���; 6:������� 2006���*/
 motor_measure_t motor_chassis[7];
	KFPTypeS kfpVar = 
	{
    0.2, //����Э����. ��ʼ��ֵΪ 0.02
    0, //����������. ��ʼ��ֵΪ 0
    0.01, //��������Э����,Q���󣬶�̬��Ӧ��죬�����ȶ��Ա仵. ��ʼ��ֵΪ 0.001
    1, //��������Э����,R���󣬶�̬��Ӧ�����������ȶ��Ա��. ��ʼ��ֵΪ 1
    0 //�������˲������. ��ʼ��ֵΪ 0
	};


static CAN_TxHeaderTypeDef  gimbal_tx_message;
static uint8_t              gimbal_can_send_data[8];
static CAN_TxHeaderTypeDef  chassis_tx_message;
static uint8_t              chassis_can_send_data[8];

		
		
		
		
		
		
void sum_encoder(uint8_t i)
{

	if(abs(motor_chassis[i].ecd - motor_chassis[i].last_ecd) > 4096)
	{
		if(motor_chassis[i].ecd >= 6500 && motor_chassis[i].last_ecd <= 1000)
		{
			motor_chassis[i].num -= 1;
		}
		else if(motor_chassis[i].ecd <= 1000 && motor_chassis[i].last_ecd >= 6500)
		{
			motor_chassis[i].num += 1;
		}
	}		
	motor_chassis[i].angle=(8191*motor_chassis[i].num + motor_chassis[i].ecd -motor_chassis[i].err_ecd)*JIXIE2RAD;
	
}
/**
  * @brief          hal CAN fifo call back, receive motor data
  * @param[in]      hcan, the point to CAN handle
  * @retval         none
  */
/**
  * @brief          hal��CAN�ص�����,���յ������
  * @param[in]      hcan:CAN���ָ��
  * @retval         none
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];

    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data);

    switch (rx_header.StdId)
    {
        case CAN_2006_M1_ID:
				{
					get_motor_measure(&motor_chassis[0], rx_data);
//					motor_chassis[0].speed_rpm=KalmanFilter(&kfpVar,motor_chassis[0].speed_rpm);
			    sum_encoder(0);		
					break;
				}
        case CAN_2006_M2_ID:
				{
					get_motor_measure(&motor_chassis[1], rx_data);
//					motor_chassis[0].speed_rpm=KalmanFilter(&kfpVar,motor_chassis[0].speed_rpm);
			    sum_encoder(1);		
					break;
				}
        case CAN_2006_M3_ID:
				{
					get_motor_measure(&motor_chassis[2], rx_data);
//					motor_chassis[0].speed_rpm=KalmanFilter(&kfpVar,motor_chassis[0].speed_rpm);
			    sum_encoder(2);		
					break;
				}
        case CAN_2006_M4_ID:
				{
					get_motor_measure(&motor_chassis[3], rx_data);
//					motor_chassis[0].speed_rpm=KalmanFilter(&kfpVar,motor_chassis[0].speed_rpm);
			    sum_encoder(3);		
					break;
				}
        case CAN_YAW_MOTOR_ID: 
        case CAN_TRIGGER_MOTOR_ID:
        {
//            static uint8_t i = 0;
            //get motor id
//            i = rx_header.StdId - CAN_2006_M1_ID;
//            get_motor_measure(&motor_chassis[i], rx_data);
          //  detect_hook(CHASSIS_MOTOR1_TOE + i);
			
            break;
        }

        default:
        {
            break;
        }
    }
}



/**
  * @brief          send control current of motor (0x205, 0x206, 0x207, 0x208)
  * @param[in]      yaw: (0x205) 6020 motor control current, range [-30000,30000] 
  * @param[in]      pitch: (0x206) 6020 motor control current, range [-30000,30000]
  * @param[in]      shoot: (0x207) 2006 motor control current, range [-10000,10000]
  * @param[in]      rev: (0x208) reserve motor control current
  * @retval         none
  */
/**
  * @brief          ���͵�����Ƶ���(0x205,0x206,0x207,0x208)
  * @param[in]      yaw: (0x205) 6020������Ƶ���, ��Χ [-30000,30000]
  * @param[in]      pitch: (0x206) 6020������Ƶ���, ��Χ [-30000,30000]
  * @param[in]      shoot: (0x207) 2006������Ƶ���, ��Χ [-10000,10000]
  * @param[in]      rev: (0x208) ������������Ƶ���
  * @retval         none
  */
void CAN_cmd_gimbal(int16_t yaw, int16_t pitch, int16_t shoot, int16_t rev)
{
    uint32_t send_mail_box;
    gimbal_tx_message.StdId = CAN_GIMBAL_ALL_ID;
    gimbal_tx_message.IDE = CAN_ID_STD;
    gimbal_tx_message.RTR = CAN_RTR_DATA;
    gimbal_tx_message.DLC = 0x08;
    gimbal_can_send_data[0] = (yaw >> 8);
    gimbal_can_send_data[1] = yaw;
    gimbal_can_send_data[2] = (pitch >> 8);
    gimbal_can_send_data[3] = pitch;
    gimbal_can_send_data[4] = (shoot >> 8);
    gimbal_can_send_data[5] = shoot;
    gimbal_can_send_data[6] = (rev >> 8);
    gimbal_can_send_data[7] = rev;
    HAL_CAN_AddTxMessage(&GIMBAL_CAN, &gimbal_tx_message, gimbal_can_send_data, &send_mail_box);
}

/**
  * @brief          send CAN packet of ID 0x700, it will set chassis motor 2006 to quick ID setting
  * @param[in]      none
  * @retval         none
  */
/**
  * @brief          ����IDΪ0x700��CAN��,��������2006��������������ID
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_chassis_reset_ID(void)
{
    uint32_t send_mail_box;
    chassis_tx_message.StdId = 0x700;
    chassis_tx_message.IDE = CAN_ID_STD;
    chassis_tx_message.RTR = CAN_RTR_DATA;
    chassis_tx_message.DLC = 0x08;
    chassis_can_send_data[0] = 0;
    chassis_can_send_data[1] = 0;
    chassis_can_send_data[2] = 0;
    chassis_can_send_data[3] = 0;
    chassis_can_send_data[4] = 0;
    chassis_can_send_data[5] = 0;
    chassis_can_send_data[6] = 0;
    chassis_can_send_data[7] = 0;

    HAL_CAN_AddTxMessage(&CHASSIS_CAN, &chassis_tx_message, chassis_can_send_data, &send_mail_box);
}


/**
  * @brief          send control current of motor (0x201, 0x202, 0x203, 0x204)
  * @param[in]      motor1: (0x201) 2006 motor control current, range [-16384,16384] 
  * @param[in]      motor2: (0x202) 2006 motor control current, range [-16384,16384] 
  * @param[in]      motor3: (0x203) 2006 motor control current, range [-16384,16384] 
  * @param[in]      motor4: (0x204) 2006 motor control current, range [-16384,16384] 
  * @retval         none
  */
/**
  * @brief          ���͵�����Ƶ���(0x201,0x202,0x203,0x204)
  * @param[in]      motor1: (0x201) 2006������Ƶ���, ��Χ [-16384,16384]
  * @param[in]      motor2: (0x202) 2006������Ƶ���, ��Χ [-16384,16384]
  * @param[in]      motor3: (0x203) 2006������Ƶ���, ��Χ [-16384,16384]
  * @param[in]      motor4: (0x204) 2006������Ƶ���, ��Χ [-16384,16384]
  * @retval         none
  */
void CAN_cmd_chassis(int16_t motor1, int16_t motor2, int16_t motor3, int16_t motor4)
{
    uint32_t send_mail_box;
    chassis_tx_message.StdId = CAN_CHASSIS_ALL_ID;
    chassis_tx_message.IDE = CAN_ID_STD;
    chassis_tx_message.RTR = CAN_RTR_DATA;
    chassis_tx_message.DLC = 0x08;
    chassis_can_send_data[0] = motor1 >> 8;
    chassis_can_send_data[1] = motor1;
    chassis_can_send_data[2] = motor2 >> 8;
    chassis_can_send_data[3] = motor2;
    chassis_can_send_data[4] = motor3 >> 8;
    chassis_can_send_data[5] = motor3;
    chassis_can_send_data[6] = motor4 >> 8;
    chassis_can_send_data[7] = motor4;

    HAL_CAN_AddTxMessage(&CHASSIS_CAN, &chassis_tx_message, chassis_can_send_data, &send_mail_box);
}

/**
  * @brief          return the yaw 6020 motor data point
  * @param[in]      none
  * @retval         motor data point
  */
/**
  * @brief          ����yaw 6020�������ָ��
  * @param[in]      none
  * @retval         �������ָ��
  */
const motor_measure_t *get_yaw_gimbal_motor_measure_point(void)
{
    return &motor_chassis[4];
}

/**
  * @brief          return the pitch 6020 motor data point
  * @param[in]      none
  * @retval         motor data point
  */
/**
  * @brief          ����pitch 6020�������ָ��
  * @param[in]      none
  * @retval         �������ָ��
  */
const motor_measure_t *get_pitch_gimbal_motor_measure_point(void)
{
    return &motor_chassis[5];
}


/**
  * @brief          return the trigger 2006 motor data point
  * @param[in]      none
  * @retval         motor data point
  */
/**
  * @brief          ���ز������ 2006�������ָ��
  * @param[in]      none
  * @retval         �������ָ��
  */
const motor_measure_t *get_trigger_motor_measure_point(void)
{
    return &motor_chassis[6];
}


/**
  * @brief          return the chassis 2006 motor data point
  * @param[in]      i: motor number,range [0,3]
  * @retval         motor data point
  */
/**
  * @brief          ���ص��̵�� 2006�������ָ��
  * @param[in]      i: ������,��Χ[0,3]
  * @retval         �������ָ��
  */
const motor_measure_t *get_chassis_motor_measure_point(uint8_t i)
{
    return &motor_chassis[(i & 0x03)];
}

float KalmanFilter(KFPTypeS *kfp, float input)
{
    //����Э����̣���ǰ ����Э���� = �ϴθ��� Э���� + ��������Э����
    kfp->P = kfp->P + kfp->Q;
 
    //���������淽�̣���ǰ ���������� = ��ǰ ����Э���� / ����ǰ ����Э���� + ��������Э���
    kfp->G = kfp->P / (kfp->P + kfp->R);
 
    //��������ֵ���̣���ǰ ����ֵ = ��ǰ ����ֵ + ���������� * ����ǰ ����ֵ - ��ǰ ����ֵ��
    kfp->Output = kfp->Output + kfp->G * (input - kfp->Output); //��ǰ ����ֵ = �ϴ� ����ֵ
 
    //���� Э���� = ��1 - ���������棩 * ��ǰ ����Э���
    kfp->P = (1 - kfp->G) * kfp->P;
 
     return kfp->Output;
}

void err_ecd_get(void)
{
	uint8_t i;
	for(i=0;i<4;i++)
	{
		motor_chassis[i].err_ecd=motor_chassis[i].ecd ;
	}


}
