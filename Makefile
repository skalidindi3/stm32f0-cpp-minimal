BINPATH = ../../gcc-arm-none-eabi-6-2017-q1/bin/
STM32CUBEPATH = ../../STM32Cube_FW_F0_V1.8.0
STM32F0HALPATH = $(STM32CUBEPATH)/Drivers/STM32F0xx_HAL_Driver
STLINK = ../../stlink/build/Release/st-flash
OPENOCDPATH = ../../openocd-0.10.0/out-osx

BINPREFIX = arm-none-eabi-
# NOTE: expecting "/" if necessary to be in BINPATH, so that global bins can be used
CC = $(BINPATH)$(BINPREFIX)gcc
AS = $(BINPATH)$(BINPREFIX)gcc -x assembler-with-cpp
OBJCOPY = $(BINPATH)$(BINPREFIX)objcopy
AR = $(BINPATH)$(BINPREFIX)ar
SIZE = $(BINPATH)$(BINPREFIX)size
GDB = $(BINPATH)$(BINPREFIX)gdb

TARGET = minimal

OPT = -O0

BUILD_DIR = build

C_SOURCES =  \
$(STM32F0HALPATH)/Src/stm32f0xx_hal.c \
$(STM32F0HALPATH)/Src/stm32f0xx_hal_cortex.c \
$(STM32F0HALPATH)/Src/stm32f0xx_hal_rcc.c \
$(STM32F0HALPATH)/Src/stm32f0xx_hal_gpio.c \
$(STM32F0HALPATH)/Src/stm32f0xx_hal_spi.c \
$(STM32CUBEPATH)/Drivers/CMSIS/Device/ST/STM32F0xx/Source/Templates/system_stm32f0xx.c

CXX_SOURCES = \
src/board.cc \
src/spi.cc \
src/main.cc

ASM_SOURCES =  \
$(STM32CUBEPATH)/Drivers/CMSIS/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f051x8.s

MCU = -mcpu=cortex-m0 -mthumb-interwork -mlong-calls

# TODO: remove HAL def if we replace all calls
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32F051x8

C_INCLUDES =  \
-Iinc \
-I$(STM32CUBEPATH)/Drivers/CMSIS/Include/ \
-I$(STM32CUBEPATH)/Drivers/CMSIS/Device/ST/STM32F0xx/Include/ \
-I$(STM32F0HALPATH)/Inc

ASFLAGS = $(MCU) $(OPT) -Wall -fdata-sections -ffunction-sections -g
ASFLAGS += -fno-common -ffreestanding -fno-exceptions -fno-non-call-exceptions

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -g
CFLAGS += -fno-common -ffreestanding -fno-exceptions -fno-non-call-exceptions #-fno-inline-functions
#CFLAGS += -flto
CXXFLAGS = -std=c++11 -fno-rtti -fno-use-cxa-atexit
# NOTE: -fno-exceptions cuts down ~4kB

# Generate dependency information
#CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"

LDSCRIPT = src/STM32F051R8Tx_FLASH.ld

LDFLAGS = $(MCU) -specs=nosys.specs -T$(LDSCRIPT) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -flto

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

re: clean all

# TODO: make flash address a var
flash: $(BUILD_DIR)/$(TARGET).bin
	$(STLINK) --reset write $(BUILD_DIR)/$(TARGET).bin 0x08000000

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(CXX_SOURCES:.cc=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.cc $(sort $(dir $(CXX_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	echo $(sort $(dir $(C_SOURCES)))
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.cc Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $(CXXFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cc=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(OBJCOPY) -O binary -S $< $@

$(BUILD_DIR):
	mkdir -p $@

gdbserver:
	$(OPENOCDPATH)/bin/openocd -f $(OPENOCDPATH)/share/openocd/scripts/board/stm32f0discovery.cfg

gdb: $(BUILD_DIR)/$(TARGET).elf
	$(GDB) $(BUILD_DIR)/$(TARGET).elf

clean:
	-rm -rf $(BUILD_DIR)

#-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)
