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

# Check if test file exists for the leader.
ifneq (,$(wildcard Tests/Leader/Test_$(TEST).c))
	TEST_LEADER ?= Tests/Leader/Test_$(TEST).c
else
	TEST_LEADER ?= Apps/Src/main.c
endif

LEADER = controls-leader

all:
	@echo "${RED}Not enough arguments. Call: ${ORANGE}make help${NC}"

.PHONY: stm32f413
leader:
	@echo "${YELLOW}Compiling for leader...${NC}"
	$(MAKE) -C BSP -C STM32F413 -j TARGET=$(LEADER) TEST=$(TEST_LEADER)
	@echo "${BLUE}Compiled for leader! Jolly Good!${NC}"

flash:
	$(MAKE) -C BSP -C STM32F413 flash

docs:
	doxygen Docs/doxyfile
	$(MAKE) -C Docs html

help:
	@echo "Format: ${ORANGE}make ${BLUE}<BSP type>${NC}${ORANGE}TEST=${PURPLE}<Test type>${NC}"
	@echo "BSP types (required):"
	@echo "	${BLUE}stm32f413/leader${NC}"
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
	rm -fr Docs/doxygen
	rm -fr Docs/build