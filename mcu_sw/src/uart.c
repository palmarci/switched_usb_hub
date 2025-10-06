#include "uart.h"
#include <string.h>
#include <stdio.h>

#define RX_BUF_SIZE 32

static UART_HandleTypeDef *huart;
static GPIO_TypeDef *led_port;
static uint16_t led_pin;

static char rx_buf[RX_BUF_SIZE];
static uint8_t rx_index = 0;

static int led_override = -1; // -1=blink, 0=off, 1=on

void uart_task_init(UART_HandleTypeDef *uart, GPIO_TypeDef *port, uint16_t pin)
{
    huart = uart;
    led_port = port;
    led_pin = pin;
}

void uart_task_process_byte(uint8_t byte)
{
    if (byte == '\n' || byte == '\r')
    {
        rx_buf[rx_index] = '\0'; // terminate string

        // parse commands: VER, ON, OFF
        if (strcmp(rx_buf, "VER") == 0)
        {
            const char *msg = "Firmware v1.0\r\n";
            HAL_UART_Transmit(huart, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
        }
        else if (strcmp(rx_buf, "ON") == 0)
        {
            led_override = 1;
            const char *msg = "LED ON\r\n";
            HAL_UART_Transmit(huart, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
        }
        else if (strcmp(rx_buf, "OFF") == 0)
        {
            led_override = 0;
            const char *msg = "LED OFF\r\n";
            HAL_UART_Transmit(huart, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
        }
        else
        {
            const char *msg = "Unknown command\r\n";
            HAL_UART_Transmit(huart, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
        }

        rx_index = 0;
        memset(rx_buf, 0, RX_BUF_SIZE);
    }
    else
    {
        if (rx_index < RX_BUF_SIZE - 1)
        {
            rx_buf[rx_index++] = byte;
        }
        else
        {
            // Buffer overflow, reset
            rx_index = 0;
            memset(rx_buf, 0, RX_BUF_SIZE);
        }
    }
}