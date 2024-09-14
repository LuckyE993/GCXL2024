#ifndef __USARTX_H_
#define __USARTX_H_
#include "stdio.h"
#include "usart.h"
#include "main.h"
#include "string.h"
#include "bsp_led.h"

#define FRAME_HEADER      0XFF //Frame_header 
#define FRAME_TAIL        0XFE //Frame_tail   
#define SEND_DATA_SIZE    22
#define RECEIVE_DATA_SIZE 22

struct _RECEIVE_
{
	uint8_t data[RECEIVE_DATA_SIZE-4];
	uint8_t head;
	uint8_t length;
	uint8_t mode;
	uint8_t crc8;
	uint8_t tail;
};

struct _SEND_
{
	uint8_t data[SEND_DATA_SIZE-4];
	uint8_t head;
	uint8_t length;
	uint8_t mode;
	uint8_t crc8;
	uint8_t tail;
} ;



void USART6_IRQHandler(void);
void excute_received_cmd (void const *pvParameters);

#endif /*__USARTX_H_*/
