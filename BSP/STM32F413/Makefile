######################################
# target
######################################
TARGET = controls-leader


######################################
# building variables
######################################
# optimization
ifeq ($(DEBUG), 0)
OPT = -O3
else
OPT = -Og
endif

#######################################
# paths
#######################################
# Build path
BUILD_DIR = ../../Objects

######################################
# source
######################################
# C sources
# since current path is in the BSP folder, go to the top level with ../../
C_SOURCES =  \
$(wildcard ../../Drivers/Src/*.c)	\
$(wildcard ../../BSP/STM32F413/Src/*.c)	\
$(wildcard ../../BSP/STM32F413/STM32F4xx_StdPeriph_Driver/Src/*.c) \
$(wildcard ../../CMSIS/DSP_Lib/Source/*.c)	\
$(wildcard ../../RTOS/uCOS-III-STM32F4/uCOS-III/Source/*.c)	\
$(wildcard ../../RTOS/uCOS-III-STM32F4/uCOS-III/Ports/ARM-Cortex-M4/Generic/GNU/*.c) \
$(wildcard ../../RTOS/uCOS-III-STM32F4/uC-CPU/*.c) \
$(wildcard ../../RTOS/uCOS-III-STM32F4/uC-CPU/ARM-Cortex-M4/GNU/*.c) \
$(wildcard ../../RTOS/uCOS-III-STM32F4/uC-LIB/*.c)

# This line adds everything in Apps/Src/*.c except for main.c, then adds the test file
C_SOURCES += \
$(filter-out ../../Apps/Src/main.c, $(wildcard ../../Apps/Src/*.c))	\
../../$(TEST)

# ASM sources
ASM_SOURCES =  \
../../BSP/STM32F413/Src/startup_stm32f413xx.s \
../../RTOS/uCOS-III-STM32F4/uCOS-III/Ports/ARM-Cortex-M4/Generic/GNU/os_cpu_a.s \
../../RTOS/uCOS-III-STM32F4/uC-CPU/ARM-Cortex-M4/GNU/cpu_a.s


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

SF = st-flash
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# float-abi
FLOAT-ABI = -mfloat-abi=soft

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DSTM32F413_423xx	\
-DUSE_STDPERIPH_DRIVER	\
-D__FPU_PRESENT


# AS includes
AS_INCLUDES = 

# C includes
# since current path is in the BSP folder, go to the top level with ../../
C_INCLUDES =  \
-I../../Apps/Inc	\
-I../../Drivers/Inc	\
-I../../Config/Inc	\
-I../../BSP/Inc	\
-I../../CMSIS/Device/ST/STM32F4xx/Include	\
-I../../CMSIS/Include	\
-I../../BSP/STM32F413/STM32F4xx_StdPeriph_Driver/Inc \
-I../../RTOS/uCOS-III-STM32F4/uCOS-III/Source/ \
-I../../RTOS/uCOS-III-STM32F4/uCOS-III/Ports/ARM-Cortex-M4/Generic/GNU/ \
-I../../RTOS/uCOS-III-STM32F4/uC-CPU/ \
-I../../RTOS/uCOS-III-STM32F4/uC-CPU/ARM-Cortex-M4/GNU/ \
-I../../RTOS/uCOS-III-STM32F4/uC-LIB/ \
-I../../Tests/Inc/ \

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -Werror -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g3 -gdwarf-2 -DDEBUG
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = ./GCC/STM32F413RHTx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	@echo "CC $(<:../../%=%)"
	@$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo "AS $(<:../../%=%)"
	@$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@echo "LD $(<:../../%=%)"
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@echo "SZ $(<:../../%=%)"
	@$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "HEX $(<:../../%=%)"
	@$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "BIN $(<:../../%=%)"
	@$(BIN) $< $@
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

#######################################
# flash
#######################################
flash:
	$(SF) write $(BUILD_DIR)/$(TARGET).bin 0x8000000

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
