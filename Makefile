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

TEST_LEADER = none
TEST_MOTOR = none
TEST_CAR = none

DEBUG = 1
export DEBUG

LEADER = controls-leader
MOTORSIM = motor-sim
CARSIM = car-sim

all:
	@echo "${RED}Not enough arguments. Call: ${ORANGE}make help${NC}"

simulator: leader motor-sim car-sim
	@echo "${BLUE}Compiled for simulator! Jolly Good!${NC}"

leader:
	@echo "${YELLOW}Compiling for leader...${NC}"
ifeq ($(TEST_LEADER), none)
	$(MAKE) -C BSP -C STM32F413 -j TARGET=$(LEADER) TEST=none
else
	$(MAKE) -C BSP -C STM32F413 -j TARGET=$(LEADER) TEST=Tests/Test_$(TEST_LEADER)
endif
	@echo "${BLUE}Compiled for leader! Jolly Good!${NC}"

motor-sim:
ifneq ($(TEST_MOTOR), none)
	@echo "${YELLOW}Compiling for motor sim...${NC}"
	$(MAKE) -C BSP -C STM32F413 -j TARGET=$(MOTORSIM) TEST=MotorSim/Test_$(TEST_MOTOR)
	@echo "${BLUE}Compiled for motor sim! Jolly Good!${NC}"
endif

car-sim:
	@echo "${YELLOW}Compiling for car sim...${NC}"
ifneq ($(TEST_CAR), none)
	$(MAKE) -C BSP -C STM32F413 -j TARGET=$(CARSIM) TEST=CarSim/Test_$(TEST_CAR)
	@echo "${BLUE}Compiled for car sim! Jolly Good!${NC}"
endif

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