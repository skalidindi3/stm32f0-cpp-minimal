#include "board.h"

#include "stm32f0xx_hal.h"

namespace board {

// override default handler
extern "C" void SysTick_Handler(void) {
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
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

    // Configure SD Card ~CS (Active LOW)
    GPIO_InitStruct.Pin = sd_csn_pin_;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(user_button_port_, &GPIO_InitStruct);

    // Configure LEDs
    GPIO_InitStruct.Pin = blue_led_pin_|green_led_pin_;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(led_port_, &GPIO_InitStruct);
}

// embedded C++ callbacks?
void STM32F0Discovery::sdCardPhysicalInit(void) {
    uint32_t responseVal;
    uint8_t response;

    selectSD();
    spi()->skip(10);
    deselectSD();
    spi()->skip(2);

#define CHECK(a, b) if (a != b) { blueLedSet(); __asm__ volatile ("bkpt"); return; }

    selectSD();

    // CMD0 --> Go Idle State
    response = sdCardSendCommand(0, 0);
    // response == 1?
    CHECK(response, 1);

    // TODO: AA is random pattern to check --> https://luckyresistor.me/cat-protector/software/sdcard-2/
    // CMD8 --> Send If Cond
    response = sdCardSendCommandWithResponse(8, 0x01aa, &responseVal);
    if (response & 0x40) {
        // SD v1
        // CMD41 --> Send Op Cond
        response = sdCardSendCommand(41, 0, true);
        // response == 1?
        CHECK(response, 1);
    } else {
        // (responseVal & 0x000000ff) != 0x000000aa?
        CHECK((responseVal & 0x000000ff), 0x000000aa);
        // SD v2
        // CMD41 --> Send Op Cond
        response = sdCardSendCommand(41, 0x40000000, true);
        // response == 1?
        CHECK(response, 1);
        // CMD58 --> Read OCR
        response = sdCardSendCommandWithResponse(58, 0, &responseVal);
        // response == 1?
        CHECK(response, 1);
        // responseValue & 0xc0000000 --> high capacity
    }

    //// CMD16 --> Set Block Length
    //response = sdCardSendCommand(16, 512);
    //// response == 1?

    deselectSD();
}

uint8_t STM32F0Discovery::sdCardSendCommand(uint8_t cmd, uint32_t arg, bool appcmd) {
    uint8_t response;

    sdCardWaitUntilReady();

    if (appcmd) {
        uint8_t appcmd_buf[] = {0x40 | 55, 0, 0, 0, 0};
        spi()->tx(appcmd_buf, 5);
        for (int i = 0; i < 8; i++) {
            spi()->rx(&response, 1);
            if ((response & 0x80) == 0) {
                break;
            }
        }
    }

    uint8_t opcode = 0x40 | (0x3F & cmd);
    spi()->tx(&opcode, 1);
    uint32_t big_endian_arg = ((arg & 0x000000FF) << 24)
                            | ((arg & 0x0000FF00) << 8)
                            | ((arg & 0x00FF0000) >> 8)
                            | ((arg & 0xFF000000) >> 24);
    spi()->tx(reinterpret_cast<uint8_t*>(&big_endian_arg), 4);
    uint8_t crc = cmd == 8 ? 0x87 : 0x95;
    spi()->tx(&crc, 1);

    for (int i = 0; i < 8; i++) {
        spi()->rx(&response, 1);
        if ((response & 0x80) == 0) {
            break;
        }
    }
    return response;
}

uint8_t STM32F0Discovery::sdCardSendCommandWithResponse(uint8_t cmd, uint32_t arg, uint32_t* response) {
    uint8_t orig_response = sdCardSendCommand(cmd, arg);
    for(int i = 0; i < 4; i++) {
        spi()->rx(reinterpret_cast<uint8_t*>(response) + i, 1);
    }
    uint32_t big_endian_response = *response;
    uint32_t little_endian_response = ((big_endian_response & 0x000000FF) << 24)
                                    | ((big_endian_response & 0x0000FF00) << 8)
                                    | ((big_endian_response & 0x00FF0000) >> 8)
                                    | ((big_endian_response & 0xFF000000) >> 24);
    *response = little_endian_response;
    return orig_response;
}

} // namespace board
