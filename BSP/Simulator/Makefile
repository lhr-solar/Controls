TARGET = bps-simulator.out
TARGET_PATH = ../../

CC = gcc

# since current path is in the BSP folder, go to the top level with ../../
SRC = \
$(wildcard ../../BSP/Simulator/Src/*.c) \
$(wildcard ../../Drivers/Src/*.c) \
$(wildcard ../../Config/Src/*.c)

ifneq ($(TEST), none)
TEST_FILE := Test_$(TEST).c
# This line adds everything in Apps/Src/*.c except for main.c
SRC += \
$(filter-out ../../Apps/Src/main.c, $(wildcard ../../Apps/Src/*.c))	\
$(wildcard ../../Tests/$(TEST_FILE))
else
SRC += $(wildcard ../../Apps/Src/*.c)
endif

INC_DIR = \
-I../../Apps/Inc \
-I../../BSP/Inc \
-I../../Drivers/Inc \
-I../../Config/Inc

LIB = \
-lm

FLAGS = -Wall -g -std=c11 $(INC_DIR)

BUILD_DIR = ../../Objects
OBJ = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC:.c=.o)))
vpath %.c $(sort $(dir $(SRC)))

all: $(TARGET_PATH)/$(TARGET)

$(BUILD_DIR)/%.o: %.c $(BUILD_DIR)
	$(CC) -c -o $@ $< $(FLAGS)

$(TARGET_PATH)/$(TARGET): $(OBJ) $(BUILD_DIR)
	$(CC) -o $@ $(OBJ) $(FLAGS) $(LIB)

$(BUILD_DIR):
	mkdir $@

.PHONY: clean

clean:
	rm -fR $(BUILD_DIR)
