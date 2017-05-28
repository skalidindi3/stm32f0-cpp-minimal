#pragma once

#include <stdint.h>
#include "spi.h"
#include "stm32f0xx_hal.h"

namespace board {

class Board {
  public:
    virtual void configureClock(void) = 0;
    virtual void initGpio(void) = 0;

    virtual base::spi::Spi* spi() = 0;
};

class STM32F0Discovery : public Board {
  public:
    STM32F0Discovery(void) {}

    void configureClock(void) override;
    void initGpio(void) override;

    inline base::spi::Spi* spi() override { return &spi1_; }

    // TODO: error/assert function & call on failed HAL fns

    // NOTE: user button is active LOW
    inline bool userButtonPushed(void) {
        return HAL_GPIO_ReadPin(user_button_port_, user_button_pin_) == GPIO_PIN_RESET;
    }

    // LED helpers
    inline void greenLedSet(void) {
        HAL_GPIO_WritePin(led_port_, green_led_pin_, GPIO_PIN_SET);
    }
    inline void greenLedReset(void) {
        HAL_GPIO_WritePin(led_port_, green_led_pin_, GPIO_PIN_RESET);
    }
    inline void blueLedSet(void) {
        HAL_GPIO_WritePin(led_port_, blue_led_pin_, GPIO_PIN_SET);
    }
    inline void blueLedReset(void) {
        HAL_GPIO_WritePin(led_port_, blue_led_pin_, GPIO_PIN_RESET);
    }

  private:
    // PA5     ------> SPI1_SCK
    // PA6     ------> SPI1_MISO
    // PA7     ------> SPI1_MOSI
    stm32::f0::Spi1 spi1_;

    // GPIO A0 --> User Button
    uint16_t user_button_pin_ = GPIO_PIN_0;
    GPIO_TypeDef* user_button_port_ = GPIOA;

    // GPIO C8 --> Blue LED
    // GPIO C9 --> Green LED
    uint16_t blue_led_pin_ = GPIO_PIN_8;
    uint16_t green_led_pin_ = GPIO_PIN_9;
    GPIO_TypeDef* led_port_ = GPIOC;

    // TODO: future chip selects: PA1/PA2
};

} // namespace board
