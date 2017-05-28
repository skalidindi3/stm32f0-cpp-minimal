#include "board.h"

int main(void)
{
    board::STM32F0Discovery dev;
    dev.configureClock();
    dev.initGpio();
    dev.spi()->init();
    while (1) {
        if (dev.userButtonPushed()) {
            dev.blueLedSet();
            dev.greenLedReset();
        } else {
            dev.blueLedReset();
            dev.greenLedSet();
        }
    }
}

