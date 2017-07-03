#include "board.h"

extern "C" void __cxa_pure_virtual() { while (1); }

int main(void)
{
    board::STM32F0Discovery dev;
    dev.configureClock();
    dev.initGpio();
    dev.spi()->init();

    dev.deselectSD();
    dev.blueLedReset();
    dev.greenLedReset();

    // http://elm-chan.org/docs/mmc/mmc_e.html
    dev.sdCardPhysicalInit();
    dev.greenLedSet();

    while (1) {
        __asm__ volatile ("bkpt");
        //if (dev.userButtonPushed()) {
        //    dev.blueLedSet();
        //    dev.greenLedReset();
        //} else {
        //    dev.blueLedReset();
        //    dev.greenLedSet();
        //}
        //dev.greenLedSet();
    }
}

