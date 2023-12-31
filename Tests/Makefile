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

ifeq ($(OS),Windows_NT)
  ifeq ($(shell uname -s),) # not in a bash-like shell
	CLEANUP = del /F /Q
	MKDIR = mkdir
  else # in a bash-like shell, like msys
	CLEANUP = rm -f
	MKDIR = mkdir -p
  endif
	TARGET_EXTENSION=.exe
else
	CLEANUP = rm -f
	MKDIR = mkdir -p
	TARGET_EXTENSION=.out
endif



LEADER = controls-leader.out

all: clean default

UNITY_ROOT=../

<<<<<<< Updated upstream
SRC_FILES1= unity.c Test_UnitTestExample.c ../Apps/Src/scalar.c test_runners/TestUnitTest.c
=======
SRC_FILES1=../Unity/src/unity.c Test_UnitTestContactors.c ../Drivers/Src/Contactors.c test_runners/TestContactors_Runner.c
>>>>>>> Stashed changes

C_COMPILER=gcc
ifeq ($(shell uname -s), Darwin)
C_COMPILER=clang
endif


CFLAGS=-std=c89
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wpointer-arith
CFLAGS += -Wcast-align
CFLAGS += -Wwrite-strings
CFLAGS += -Wswitch-default
CFLAGS += -Wunreachable-code
CFLAGS += -Winit-self
CFLAGS += -Wmissing-field-initializers
CFLAGS += -Wno-unknown-pragmas
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wundef
CFLAGS += -Wold-style-definition

TARGET_BASE1=test1
TARGET1 = $(TARGET_BASE1)$(TARGET_EXTENSION)

C_INCLUDES =  \
-I../Apps/Inc	\
-I../Drivers/Inc	\
-I../Config/Inc	\
-I../BSP/Inc	\
-I../CMSIS/Device/ST/STM32F4xx/Include	\
-I../CMSIS/Include	\
-I../BSP/STM32F413/STM32F4xx_StdPeriph_Driver/Inc \
-I../RTOS/uCOS-III-STM32F4/uCOS-III/Source/ \
-I../RTOS/uCOS-III-STM32F4/uCOS-III/Ports/ARM-Cortex-M4/Generic/GNU/ \
-I../RTOS/uCOS-III-STM32F4/uC-CPU/ \
-I../RTOS/uCOS-III-STM32F4/uC-CPU/ARM-Cortex-M4/GNU/ \
-I../RTOS/uCOS-III-STM32F4/uC-LIB/ \
-I../Tests/Inc/ \


CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) -Wall -fdata-sections -ffunction-sections -g -Wextra

CFLAGS += $(C_INCLUDES)

default: $(SRC_FILES1)
	$(C_COMPILER) $(CFLAGS) $(SRC_FILES1) -o $(TARGET1)
	- ./$(TARGET1)

clean:
	$(CLEANUP) $(TARGET1) 

ci: default
