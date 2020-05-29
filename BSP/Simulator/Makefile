######################################
# Target
######################################
TARGET = controls-simulator.out
TARGET_PATH = ../..

######################################
# GCC
######################################
CC = gcc

######################################
# Source
######################################
SRC = $(wildcard Src/*.c)	# BSP code

ifneq ($(TEST), none)
TEST_FILE := Test_$(TEST).c
SRC += \
$(filter-out ../../Apps/Src/main.c, $(wildcard ../../Apps/Src/*.c)) \
$(wildcard ../../Tests/$(TEST_FILE))		# Apps (not main)
else
SRC += $(wildcard ../../Apps/Src/*.c)		# Apps (with  main)
endif

SRC += \
$(wildcard ../../RTOS/Simulator/uC-CPU/*.c) \
$(wildcard ../../RTOS/Simulator/uC-CPU/Posix/GNU/*.c) \
$(wildcard ../../RTOS/Simulator/uC-LIB/*.c) \
$(wildcard ../../RTOS/Simulator/uCOS-III/Source/*.c) \
$(wildcard ../../RTOS/Simulator/uCOS-III/Ports/POSIX/GNU/*.c)	# RTOS code

######################################
# Objects
######################################
BUILD_DIR = ../../Objects
OBJ = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC:.c=.o)))	# Create object file list
vpath %.c $(sort $(dir $(SRC)))		# In case files aren't found

######################################
# Flags
######################################
INC_DIR = \
-I../../Apps/Inc \
-I../../BSP/Inc \
-I../../Config/Inc \
-I../../RTOS/Simulator/uC-CPU \
-I../../RTOS/Simulator/uC-CPU/Posix/GNU \
-I../../RTOS/Simulator/uC-LIB \
-I../../RTOS/Simulator/uCOS-III/Source \
-I../../RTOS/Simulator/uCOS-III/Ports/POSIX/GNU \

LIB = -lm -lpthread		# Standard libraries used

FLAGS = -Wall -g -std=c11 $(INC_DIR)

######################################
# Defines
######################################
DEF = -D_XOPEN_SOURCE=600

######################################
# Build
######################################
all: $(TARGET_PATH)/$(TARGET)

$(TARGET_PATH)/$(TARGET): $(OBJ) $(BUILD_DIR)
	$(CC) -o $@ $(OBJ) $(FLAGS) $(LIB)

$(BUILD_DIR)/%.o: %.c $(BUILD_DIR)
	$(CC) -c -o $@ $< $(FLAGS) $(DEF)

$(BUILD_DIR):
	mkdir $@
