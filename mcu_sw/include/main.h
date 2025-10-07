#ifndef MAIN_H
#define MAIN_H

#include "stm32f030x6.h"
#include <stm32f0xx.h>
#include <stm32f0xx_hal.h>

#define PIN_TEST_LED    GPIO_PIN_10     // PA10
#define PIN_PSU_SW0     GPIO_PIN_4      // PA4
#define PIN_PSU_SW1     GPIO_PIN_5      // PA5
#define PIN_PSU_SW2     GPIO_PIN_6      // PA6
#define PIN_UART_TX     GPIO_PIN_2      // PA2
#define PIN_UART_RX     GPIO_PIN_3      // PA3

#define RX_BUF_SIZE     32

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART1_UART_Init(void);

extern int atoi (const char * str);

#endif // MAIN_H
