#pragma once

#include <stdint.h>
#include "stm32f0xx_hal.h"

namespace base {
namespace spi {

class Spi {
  public:
    virtual bool init(void) = 0;

    virtual bool tx(uint8_t* buf, size_t num) = 0;
    virtual bool rx(uint8_t* buf, size_t num) = 0;
    virtual bool xfer(uint8_t* txbuf, uint8_t* rxbuf, size_t num) = 0;

    // NOTE: can be used as a SCLK delay where num * 8 is # of delay cycles
    virtual bool skip(size_t num) = 0;
};

} // namespace spi
} // namespace base

namespace stm32 {
namespace f0 {

class Spi : public base::spi::Spi {
  public:
    virtual bool tx(uint8_t* buf, size_t num) override;
    virtual bool rx(uint8_t* buf, size_t num) override;
    virtual bool xfer(uint8_t* txbuf, uint8_t* rxbuf, size_t num) override;
    virtual bool skip(size_t num) override;

  protected:
    SPI_HandleTypeDef handle_;
};

class Spi1 : public Spi {
  public:
    Spi1(void) {}
    virtual bool init(void) override;
};

} // namespace f0
} // namespace stm32
