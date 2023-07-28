/**
 * @file Test_Runner.h
 * @brief Function definitions for Test Runner. Please define TEST_USE_RTOS as 0 or 1 if including this file.
 */

#include <assert.h>
#include "bsp.h"
#include "config.h"
#include "Tasks.h"

void Test_Setup();
void Test_Start();

void __attribute__((unused)) Verify(bool blocking);