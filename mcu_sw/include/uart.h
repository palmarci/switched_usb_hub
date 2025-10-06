#ifndef UART_TASK_H
#define UART_TASK_H

//#include "stm32f0xx_hal.h"
#include <stm32f0xx.h>
#include <stm32f0xx_hal.h>


// Initialize uart task with uart handle and led gpio info
void uart_task_init(UART_HandleTypeDef *huart, GPIO_TypeDef *led_port, uint16_t led_pin);

// Process UART data; should be called from UART RX complete callback or polling
void uart_task_process_byte(uint8_t byte);

// This function allows main to override blinking (1=LED ON, 0=LED OFF, -1=blink mode)
int uart_task_get_led_override(void);

#endif // UART_TASK_H
