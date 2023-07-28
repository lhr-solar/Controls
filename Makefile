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

DEBUG ?= 1
export DEBUG

USE_TEST_RUNNER ?= 0
export USE_TEST_RUNNER

# Note: ?= will define the variable only if it is not already set, which means the user can override any of
# the TEST_LEADER, TEST_MOTOR, or TEST_CAR assignments with their own test files.

# Check if test file exists for the leader.
ifneq (,$(wildcard Tests/Leader/Test_$(TEST).c))
	TEST_LEADER ?= Tests/Leader/Test_$(TEST).c
else
	TEST_LEADER ?= Apps/Src/main.c
endif

# Check if test file exists for Motor simulator.
ifneq (,$(wildcard Tests/MotorSim/Test_$(TEST).c))
	TEST_MOTOR ?= Tests/MotorSim/Test_$(TEST).c	
else
	TEST_MOTOR ?= Tests/MotorSim/Test_MotorSim.c
endif

# Check if test file exists for Car simulator.
ifneq (,$(wildcard Tests/CarSim/Test_$(TEST).c))
	TEST_CAR ?= Tests/CarSim/Test_$(TEST).c
else
	TEST_CAR ?= Tests/CarSim/Test_CarSim.c	
endif

LEADER = controls-leader
MOTORSIM = motor-sim
CARSIM = car-sim

all:
	@echo "${RED}Not enough arguments. Call: ${ORANGE}make help${NC}"

simulator: leader motor-sim car-sim
	@echo "${ORANGE}Test Files: $(TEST_LEADER)	$(TEST_MOTOR)	$(TEST_CAR)"
	@echo "${BLUE}Compiled for simulator! Jolly Good!${NC}"

leader:
	@echo "${YELLOW}Compiling for leader...${NC}"
	$(MAKE) -C BSP -C STM32F413 -j TARGET=$(LEADER) TEST=$(TEST_LEADER)
	@echo "${BLUE}Compiled for leader! Jolly Good!${NC}"

motor-sim:
	@echo "${YELLOW}Compiling for motor sim...${NC}"
	$(MAKE) -C BSP -C STM32F413 -j TARGET=$(MOTORSIM) TEST=$(TEST_MOTOR)
	@echo "${BLUE}Compiled for motor sim! Jolly Good!${NC}"

car-sim:
	@echo "${YELLOW}Compiling for car sim...${NC}"
	$(MAKE) -C BSP -C STM32F413 -j TARGET=$(CARSIM) TEST=$(TEST_CAR)
	@echo "${BLUE}Compiled for car sim! Jolly Good!${NC}"

stm32f413: leader

flash:
	$(MAKE) -C BSP -C STM32F413 flash

help:
	@echo "Format: ${ORANGE}make ${BLUE}<BSP type>${NC}${ORANGE}TEST=${PURPLE}<Test type>${NC}"
	@echo "BSP types (required):"
	@echo "	${BLUE}simulator${NC}"
	@echo "	${BLUE}stm32f413${NC}"
	@echo ""
	@echo "Test types (optional):"
	@echo "	Set TEST only if you want to build a test."
	@echo "	Otherwise, don't include TEST in the command."
	@echo "	To build a test, replace ${PURPLE}<Test type>${NC} with the name of the file"
	@echo "	excluding the file type (.c) e.g. say you want to test Voltage.c, call"
	@echo "		${ORANGE}make ${BLUE}stm32f413 ${ORANGE}TEST=${PURPLE}Voltage${NC}"


clean:
	rm -fR Objects
	rm -f *.out