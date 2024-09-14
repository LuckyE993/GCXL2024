#include "usartx.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "mytask.h"
#include "PID.h"

struct _RECEIVE_  receive;
struct _SEND_  send;
SemaphoreHandle_t xSemaphore_usart_receive;

extern float x;
extern float y;
extern float yaw;

void USART6_IRQHandler(void)  
{

	volatile uint8_t Usart_Receive;
	//Usart_Receive interrupt 接收中断
	if(huart6.Instance->SR & UART_FLAG_RXNE)
	{
			
		static uint8_t Count,i;
		static uint8_t rxbuf[RECEIVE_DATA_SIZE];
		
		Usart_Receive = huart6.Instance->DR;	
		rxbuf[Count]=Usart_Receive;
	
		 if(Usart_Receive == FRAME_HEADER || Count>0) 
			Count++; 
		else 
			Count=0;
		
		if (Count == RECEIVE_DATA_SIZE) //Verify the length of the packet //验证数据包的长度
		{   
				Count=0; //Prepare for the serial port data to be refill into the array //为串口数据重新填入数组做准备
				if(rxbuf[RECEIVE_DATA_SIZE-1] == FRAME_TAIL) //Verify the frame tail of the packet //验证数据包的帧尾
				{	
					
						memcpy(receive.data, (const uint8_t *)rxbuf + 2, 18);
						receive.head = rxbuf[0];
						receive.mode = rxbuf[1];
						receive.crc8 = rxbuf[RECEIVE_DATA_SIZE-2];
						receive.tail = rxbuf[RECEIVE_DATA_SIZE-1];
						receive.length = strlen((const void *)rxbuf)+1;

									
				}
		 }
		
		HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);

	}
	//idle interrupt 空闲中断
	else if(huart6.Instance->SR & UART_FLAG_IDLE)
	{
			Usart_Receive = huart6.Instance->DR;
			HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
	}


}

void excute_received_cmd (void const *pvParameters)
{
		
		xSemaphore_usart_receive = xSemaphoreCreateMutex();
		if(xSemaphore_usart_receive!=NULL)
			
		{
			HAL_GPIO_TogglePin(LED_B_GPIO_Port, LED_B_Pin);
			uint8_t mode;
			uint8_t cmd[RECEIVE_DATA_SIZE-4];
			while(1)
			{
				HAL_GPIO_TogglePin(LED_B_GPIO_Port, LED_B_Pin);
				if(xSemaphoreTake( xSemaphore_usart_receive, 10 / portTICK_PERIOD_MS ) == pdTRUE )
				{
					mode = receive.mode;
					memcpy(cmd,receive.data,RECEIVE_DATA_SIZE-4);
					xSemaphoreGive( xSemaphore_usart_receive );
				}
				else
				{
					vTaskDelay(30/portTICK_PERIOD_MS);
					HAL_GPIO_TogglePin(LED_B_GPIO_Port, LED_B_Pin);
					continue;
				}
				
				switch(mode)
				{
					case 0x07:
					{
						float speed;
						int16_t combined_value = (int16_t)((cmd[0] << 8) | cmd[1]);
						speed = ((float)combined_value/32767.0)*CURRENT_MAX;
						x = speed ;
						
						
						combined_value = (int16_t)(cmd[2]<<8|cmd[3]);
						speed = (((float)combined_value/32767.0)*CURRENT_MAX);
						y = speed;
						
						combined_value = (int16_t)(cmd[4]<<8|cmd[5]);
						speed = (((float)combined_value/32767.0)*CURRENT_MAX);
						yaw = speed;
						
						// printf("%f , %f , %f ",x,y,yaw);
						break;
					}
					
						
					default: 
						// printf("%c",mode);
						break;
				}
					
				
				vTaskDelay(30/portTICK_PERIOD_MS);
			
			}
		}
}



