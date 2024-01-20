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
BUILD_DIR = Objects

######################################
# source

# C sources																			
# Need to include test, unity, real source file for the test, mock c functions minus source file for test
C_SOURCES = \
Tests/Test_$(TEST).c \
Unity/unity.c \
$(filter-out $(wildcard ../../Tests/*/Src/$(TEST).c), $(wildcard ../../*/Src/$(TEST).c))	\
$(wildcard Mocks/RTOS/os.c) \
$(filter-out $(wildcard Mocks/*/Src/$(TEST).c), $(wildcard Mocks/*/Src/*.c))



$(foreach src,$(C_SOURCES),$(info --- $(src)))

#######################################
# binaries
#######################################

CC = gcc
AS = gcc -x assembler-with-cpp
CP = objcopy
SZ = size

HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

SF = st-flash
 
#######################################
# C_INCLUDES
#######################################

#-iquote: used to stop the compiler from searching the current directory first. Another effect: -Idirs before it are only searched for "" files
# Whatever I put after iquote doesn't seem to be found, but putting it in twice works. This will break #include-next, though
C_INCLUDES :=	\
-I-	\
-I../../Tests/Inc/	\
-I../../Tests/UnitTests/Mocks/Apps/Inc	\
-I../../Tests/UnitTests/Mocks/Drivers/Inc	\
-I../../Tests/UnitTests/Mocks/BSP/Inc	\
-I../../Tests/UnitTests/Mocks/RTOS/	\
-I../../Apps/Inc	\
-I../../Drivers/Inc	\
-I../../BSP/Inc	\
-I../../Config/Inc	\
-I../../Tests/UnitTests	\
-I../../Tests/UnitTests/Unity	\
-I../../RTOS/uCOS-III-STM32F4/uCOS-III/Source/ \
-I../../RTOS/uCOS-III-STM32F4/uCOS-III/Ports/ARM-Cortex-M4/Generic/GNU/ \
-I../../RTOS/uCOS-III-STM32F4/uC-CPU/ \
-I../../RTOS/uCOS-III-STM32F4/uC-CPU/ARM-Cortex-M4/GNU/ \
-I../../RTOS/uCOS-III-STM32F4/uC-LIB/	


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
CFLAGS += -DMOCKING

# mocking -> LOOP = ''
CFLAGS += -D'LOOP='

FILETOTEST = $(shell echo '$(TEST)' | tr '[:lower:]' '[:upper:]') #Uses shell to make TEST file uppercase to define a macro
CFLAGS += -DTEST_$(FILETOTEST) #Defines a macro TEST_<FILENAME> to determine if we should use a mock or regular header file

#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))


##									#  @$(CC) $(OBJECTS) -o $@


$(BUILD_DIR)/$(TEST): $(BUILD_DIR) $(C_SOURCES)
	@echo "LD $(<:../../%=%)"
	@$(CC) $(CFLAGS) $(C_INCLUDES) $(C_SOURCES) -o $(BUILD_DIR)/Test_$(TEST).out			
	@echo "SZ $(<:../../%=%)"
	- ./$(BUILD_DIR)/Test_$(TEST).out


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
#flash:
#	$(SF) write $(BUILD_DIR)/$(TARGET).bin 0x8000000
# 	unsure if this works or if we would ever use it
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)
#don't think we use any .d files?
# *** EOF ***
