# COLORS
RED=\033[0;31m
GREEN=\033[0;32m
ORANGE=\033[0;33m
BLUE=\033[0;34m
PURPLE=\033[0;35m
CYAN=\033[0;36m
LIGHTGRAY=\033[0;37m
DARKGRAY=\033[1;30m
YELLOW=\033[0;33m
NC=\033[0m # No Color

# Build path
BUILD_DIR = Objects#../../Objects

######################################
# source

# C sources																			changed to
# Need to include test, unity, source file for the test, mock c functions minus source file for test
C_SOURCES = \
Tests/Test_$(TEST).c \
Unity/unity.c \
$(wildcard ../../*/Src/$(TEST).c)	\
$(filter-out $(wildcard Mocks/*/Src/$(TEST).c), $(wildcard Mocks/*/Src/*))



$(foreach src,$(C_SOURCES),$(info --- $(src)))

#$(wildcard Stubs/*.c)
#echo "$(C_SOURCE) is c source"	

#######################################
# binaries
#######################################
PREFIX = #arm-none-eabi- 														not this time! ##
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# or it can be added to the PATH environment variable.
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size

HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

SF = st-flash
 
#######################################
# CFLAGS
#######################################
# cpu
#CPU = -mcpu=cortex-m4						commented out##

# float-abi
#FLOAT-ABI = -mfloat-abi=soft					co##

# mcu
#MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)		co##

# macros for gcc
# AS defines
#AS_DEFS = 										co##

# C defines
# C_DEFS =  \									co##
# -DSTM32F413_423xx	\
# -DUSE_STDPERIPH_DRIVER	\
# -D__FPU_PRESENT


# AS includes
#AS_INCLUDES = 									#co TEST=Pedals -> include Pedals.c and Pedals.h

C_INCLUDES :=	\
$(filter-out $(wildcard Mocks/*/$(TEST).h), $(wildcard Mocks/*/Inc))	\
-I../../Apps/Inc	\
-I../../Drivers/Inc	\
-I../../Config/Inc	\
-I../../BSP/Inc	\
-I../../Tests/Inc/	\
-I../../Tests/UnitTests	\
-I../../Tests/UnitTests/Unity	\
-IInc	
$(foreach src,$(C_INCLUDES),$(info --- $(src)))

 
ifeq ($(DEBUG), 1)
CFLAGS += -g3 -gdwarf-2 -DDEBUG
endif

ifeq ($(MOTOR_LOOPBACK), 1)
CFLAGS += -DMOTOR_LOOPBACK
endif

ifeq ($(CAR_LOOPBACK), 1)
CFLAGS += -DCAR_LOOPBACK
endif

 MOCKING=1
 export MOCKING
 CFLAGS += -DMOCKING

#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

##Added
TOCOMPILE := Test_$(TEST)
##									#  @$(CC) $(OBJECTS) -o $@

#added
$(BUILD_DIR)/$(TOCOMPILE).o: $(BUILD_DIR) $(TOCOMPILE).c
	@echo "LD $(<:../../%=%)"
	@echo "Mocking is $(MOCKING)"
	@$(CC) $(CFLAGS) $(C_INCLUDES) $(TOCOMPILE).c $(C_SOURCES) -o $(BUILD_DIR)/$(TOCOMPILE).o			
	@echo "SZ $(<:../../%=%)"
	- ./$(BUILD_DIR)/$(TOCOMPILE).o


# $(BUILD_DIR)/$(TOCOMPILE).o: $(TOCOMPILE).c Makefile | $(BUILD_DIR)
# 	@echo "CC $(<:../../%=%)"
# 	@$(CC) $(C_INCLUDES) -c $(CFLAGS) $< -o $@
# 	- ./$(BUILD_DIR)/$(TOCOMPILE).o

# $(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
# 	@echo "AS $(<:../../%=%)"
# 	@$(AS) -c $(CFLAGS) $< -o $@

# $(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
# 	@echo "LD $(<:../../%=%)"
# 	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
# 	@echo "SZ $(<:../../%=%)"
# 	@$(SZ) $@

# $(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
# 	@echo "HEX $(<:../../%=%)"
# 	@$(HEX) $< $@
	
# $(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
# 	@echo "BIN $(<:../../%=%)"
# 	@$(BIN) $< $@
	
#end add									^^ use to be @$(SZ) $@

	
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
