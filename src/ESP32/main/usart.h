#ifndef __USART_H__
#define __USART_H__
#include "udp_server.h"

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"


#define ECHO_TEST_TXD GPIO_NUM_4
#define ECHO_TEST_RXD GPIO_NUM_5
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define BUF_SIZE   (1024)

#define ECHO_UART_PORT_NUM      2
#define ECHO_UART_BAUD_RATE     115200
#define ECHO_TASK_STACK_SIZE    3072

void udp2usart_init(void);


#endif /*__USART_H__*/