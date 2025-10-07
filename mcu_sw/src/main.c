#include "main.h"
#include <string.h>

static char rx_buf[RX_BUF_SIZE];
static uint8_t rx_index = 0;

UART_HandleTypeDef huart;
uint8_t rx_irq_byte;

const char *msg_ok = "OK\r\n";
const char *msg_err = "ERR\r\n";
const char *msg_fw = "Build " __DATE__ " " __TIME__  "\r\n";

#define UART_SEND(msg) HAL_UART_Transmit(&huart, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

uint16_t resolve_pin(int value) {
    
    // here the pinout can be remapped

    switch (value) {
        case 1:
            return PIN_PSU_SW0;
        case 2:
            return PIN_PSU_SW1;
        case 3:
            return PIN_PSU_SW2;

        default:
            return -1;
    }
}

void handle_psu_switch(int cmd_len, int state) {
    if (rx_buf[cmd_len] != '\0')
    {
        int val = atoi(&rx_buf[cmd_len]);
        uint16_t pin_mask = resolve_pin(val);

        if (pin_mask != -1) {
            HAL_GPIO_WritePin(GPIOA, pin_mask, state);
            UART_SEND(msg_ok);
        } else {
            UART_SEND(msg_err);
        }

    } else {
        UART_SEND(msg_err);
    }
    return;
}

void uart_process(uint8_t byte)
{
    if (byte == '\n' || byte == '\r')
    {
        rx_buf[rx_index] = '\0'; // terminate it string

        // parse commands
        if (strcmp(rx_buf, "VER") == 0)
        {
            UART_SEND(msg_fw);
        }
        else if (strncmp(rx_buf, "ON", 2) == 0)
        {
            handle_psu_switch(2, 1);
        }
        else if (strncmp(rx_buf, "OFF", 3) == 0)
        {
            handle_psu_switch(3, 0);
        }
        else
        {
            UART_SEND(msg_err);
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

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    // start uart interrupts
    HAL_UART_Receive_IT(&huart, &rx_irq_byte, 1);

    uint32_t last_tick = HAL_GetTick();

    while (1)
    {
     
        // Blink mode (toggle every 1000ms)
        if (HAL_GetTick() - last_tick >= 1000)
        {
            HAL_GPIO_TogglePin(GPIOA, PIN_TEST_LED);
            last_tick = HAL_GetTick();
        }
      
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
     
        uart_process(rx_irq_byte);

        // Restart RX interrupt
        HAL_UART_Receive_IT(huart, &rx_irq_byte, 1);
    }
}

void SystemClock_Config(void)
{
    // modified from https://github.com/BLavery/STM32F030F4P6-Arduino/blob/master/(stm32)/1.4.0/variants/DEMO_F030F4/variant.cpp

    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /* Initializes the CPU, AHB and APB busses clocks */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI14; // | RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_OFF; // RCC_HSE_ON;
    RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
    RCC_OscInitStruct.HSI14CalibrationValue = RCC_HSI14CALIBRATION_DEFAULT; //16;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
    RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
       __BKPT(0);
    }

    /* Initializes the CPU, AHB and APB busses clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    {
       __BKPT(0);
    }

    /* Configure the Systick interrupt time */
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

    /* Configure the Systick */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // test led
    GPIO_InitStruct.Pin = PIN_TEST_LED;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // sw0
    GPIO_InitStruct.Pin = PIN_PSU_SW0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // sw1
    GPIO_InitStruct.Pin = PIN_PSU_SW1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // sw2
    GPIO_InitStruct.Pin = PIN_PSU_SW2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    return;
}

void MX_USART1_UART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE(); // already performed in MX_GPIO_Init()

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = PIN_UART_TX;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = PIN_UART_RX;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart.Instance = USART1;
    huart.Init.BaudRate = 115200;
    huart.Init.WordLength = UART_WORDLENGTH_8B;
    huart.Init.StopBits = UART_STOPBITS_1;
    huart.Init.Parity = UART_PARITY_NONE;
    huart.Init.Mode = UART_MODE_TX_RX;
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&huart);

    return;
}