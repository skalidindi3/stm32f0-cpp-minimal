#include "board.h"

#include "stm32f0xx_hal.h"

namespace board {

// override default handler
extern "C" void SysTick_Handler(void) {
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
}

// called by HAL_SPI_Init
extern "C" void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
  GPIO_InitTypeDef GPIO_InitStruct;
  if(hspi->Instance == SPI1) {
    /* Peripheral clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}

// called by HAL_SPI_DeInit
extern "C" void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi) {
  if(hspi->Instance == SPI1) {
    __HAL_RCC_SPI1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
  }
}

// uses high-speed internal clock
void STM32F0Discovery::configureClock(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /* Initializes the CPU, AHB and APB busses clocks */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 16;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
    RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
    HAL_RCC_OscConfig(&RCC_OscInitStruct); // ?== HAL_OK

    /* Initializes the CPU, AHB and APB busses clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK
                                | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1); // ?== HAL_OK

    /* Configure the Systick interrupt time */
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /* Configure the Systick */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void STM32F0Discovery::initGpio(void) {
    GPIO_InitTypeDef GPIO_InitStruct;

    // GPIO Ports Clock Enable
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // Set LED pins LOW
    HAL_GPIO_WritePin(led_port_, blue_led_pin_ | green_led_pin_, GPIO_PIN_RESET);

    // Configure User Button (A0)
    GPIO_InitStruct.Pin = user_button_pin_;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(user_button_port_, &GPIO_InitStruct);

    // Configure LEDs
    GPIO_InitStruct.Pin = blue_led_pin_|green_led_pin_;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(led_port_, &GPIO_InitStruct);
}

void STM32F0Discovery::initSpi(void) {
    hspi1_.Instance = SPI1;
    hspi1_.Init.Mode = SPI_MODE_MASTER;
    hspi1_.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1_.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1_.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1_.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1_.Init.NSS = SPI_NSS_SOFT;
    hspi1_.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
    hspi1_.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1_.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1_.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1_.Init.CRCPolynomial = 7;
    hspi1_.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    hspi1_.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    HAL_SPI_Init(&hspi1_); // ?== HAL_OK
}

} // namespace board
