
# Build path
BUILD_DIR = Objects#../../Objects

######################################
# source

#																							changed to
C_SOURCES = \
../$(TEST)

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
#AS_INCLUDES = 									#co

C_INCLUDES :=	\
-I../../Apps/Inc	\
-I../../Drivers/Inc	\
-I../../Config/Inc	\
-I../../BSP/Inc	\
-I../../Tests/Inc/	\
-I../../Tests/UnitTests \
-IInc

ifeq ($(DEBUG), 1)
CFLAGS += -g3 -gdwarf-2 -DDEBUG
endif

ifeq ($(MOTOR_LOOPBACK), 1)
CFLAGS += -DMOTOR_LOOPBACK
endif

ifeq ($(CAR_LOOPBACK), 1)
CFLAGS += -DCAR_LOOPBACK
endif


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
TOCOMPILE := $(TEST_LEADER)
##									#  @$(CC) $(OBJECTS) -o $@

#added
$(BUILD_DIR)/$(TARGET).o: $(BUILD_DIR) $(TOCOMPILE).c
	@echo "LD $(<:../../%=%)"
	@$(CC) $(C_INCLUDES) $(TOCOMPILE).c -o $(BUILD_DIR)/$(TOCOMPILE).o			
	@echo "SZ $(<:../../%=%)"
	- ./$(BUILD_DIR)/$(TOCOMPILE).o
	
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
