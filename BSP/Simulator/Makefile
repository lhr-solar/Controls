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

SRC += $(wildcard ../../RTOS/Simulator/*.c)	# RTOS code

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

LIB = # Standard libraries used

FLAGS = -Wall -g -std=c11 $(INC_DIR)

######################################
# Build
######################################
all: $(TARGET_PATH)/$(TARGET)

$(TARGET_PATH)/$(TARGET): $(OBJ) $(BUILD_DIR)
	$(CC) -o $@ $(OBJ) $(FLAGS) $(LIB)

$(BUILD_DIR)/%.o: %.c $(BUILD_DIR)
	$(CC) -c -o $@ $< $(FLAGS)

$(BUILD_DIR):
	mkdir $@
