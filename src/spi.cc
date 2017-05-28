#include "spi.h"

#include "stm32f0xx_hal.h"

extern "C" {

// called by HAL_SPI_Init
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
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
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi) {
  if(hspi->Instance == SPI1) {
    __HAL_RCC_SPI1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
  }
}

} // extern "C"

namespace stm32 {
namespace f0 {

// TODO: optimize
bool Spi::tx(uint8_t* buf, size_t num) {
    return HAL_SPI_Transmit(&handle_, buf, num, HAL_MAX_DELAY) == HAL_OK;
}

// TODO: optimize
bool Spi::rx(uint8_t* buf, size_t num) {
    return HAL_SPI_Receive(&handle_, buf, num, HAL_MAX_DELAY) == HAL_OK;
}

// TODO: optimize
bool Spi::xfer(uint8_t* txbuf, uint8_t* rxbuf, size_t num) {
    return HAL_SPI_TransmitReceive(&handle_, txbuf, rxbuf, num, HAL_MAX_DELAY) == HAL_OK;
}

// TODO: optimize to not use local buffer
bool Spi::skip(size_t num) {
    static constexpr size_t kUpperBound = 16;
    if (num >= kUpperBound)
        return false;
    uint8_t buf[kUpperBound] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    return HAL_SPI_Transmit(&handle_, buf, num, HAL_MAX_DELAY) == HAL_OK;
}

bool Spi1::init(void) {
    handle_.Instance = SPI1;

    handle_.Init.Mode = SPI_MODE_MASTER;
    handle_.Init.TIMode = SPI_TIMODE_DISABLE;

    handle_.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;

    handle_.Init.Direction = SPI_DIRECTION_2LINES;
    handle_.Init.DataSize = SPI_DATASIZE_8BIT;
    handle_.Init.FirstBit = SPI_FIRSTBIT_MSB;

    handle_.Init.CLKPolarity = SPI_POLARITY_LOW;
    handle_.Init.CLKPhase = SPI_PHASE_1EDGE;

    handle_.Init.NSS = SPI_NSS_SOFT;
    handle_.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

    handle_.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    //handle_.Init.CRCPolynomial = 7;
    //handle_.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;

    bool result = HAL_SPI_Init(&handle_) == HAL_OK;
    return result;
}

} // namespace f0
} // namespace stm32
