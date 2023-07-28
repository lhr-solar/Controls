/**
 * @file Test_Runner.h
 * @brief Function definitions for Test Runner.
 */

#include <assert.h>
#include "bsp.h"
#include "config.h"
#include "Tasks.h"

void Init();

void Test_Setup();
void Test_Start();

void Test_Start_Task();

void __attribute__((unused)) Verify(bool blocking);