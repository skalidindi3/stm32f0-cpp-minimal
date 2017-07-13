#include "sd.h"

#include "stm32f0xx_hal.h"
#include "spi.h"

static inline uint32_t switchEndian(uint32_t val) {
    return ((var & 0x000000FF) << 24)
         | ((var & 0x0000FF00) << 8)
         | ((var & 0x00FF0000) >> 8)
         | ((var & 0xFF000000) >> 24);
}

bool sd::ready(void) {
    uint8_t response;
    spi()->rx(&response, 1);
    return response == 0xFF;
}

void sd::waitUntilReady(void) {
    for(int i = 0; i < 512; i++) {
        if (ready()) return;
    }
    PANIC();
    // TODO: return ETIMEOUT?
}

void sd::prepAppCommand(void) {
    static uint8_t buf[] = {0x40 | 55, 0, 0, 0, 0};
    uint8_t response;
    // TODO: make |buf| const when spi tx is made to take const buffers

    spi()->tx(buf, sizeof(buf));
    for (int i = 0; i < 8; i++) {
        spi()->rx(&response, 1);
        if ((response & 0x80) == 0) {
            return;
        }
    }
    PANIC();
}

typedef struct __attribute__(packed) {
    uint8_t  meta;
    union {
        uint32_t output;
        uint8_t  output_raw[4];
    }
} Response;

// TODO: make response a union of single response and extra var
Response sd::sendCommand(Command cmd, uint32_t arg) {
    Response response;
    // TODO: if app cmd?

    uint8_t opcode = 0x40 | (0x3F & cmd);
    if (arg) {
        arg = switchEndian(arg);
    }
    uint8_t crc = cmd == 8 ? 0x87 : 0x95;
    spi()->tx(&opcode, sizeof(opcode));
    spi()->tx(reinterpret_cast<uint8_t*>(&arg), sizeof(arg));
    spi()->tx(&crc, sizeof(crc));

    for (int i = 0; i < 8; i++) {
        spi()->rx(&response.meta, 1);
        if ((response.meta & 0x80) == 0) {
            return true;
            // TODO: check for extra args
            spi()->rx(response.output_raw, sizeof(response.output));
            response.output = switchEndian(response.output);
        }
    }
    return false;
}

bool sd::init(void) {
}
