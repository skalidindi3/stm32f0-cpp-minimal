#include "board.h"

int main(void)
{
    board::STM32F0Discovery dev;
    dev.configureClock();
    dev.initGpio();
    dev.initSpi();
    while (1) {
        if (dev.readUserButton()) {
            dev.blueLedSet();
            dev.greenLedReset();
        } else {
            dev.blueLedReset();
            dev.greenLedSet();
        }
    }
}

