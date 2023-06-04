#ifndef __FAULT_STATE_H
#define __FAULT_STATE_H

#include "Tasks.h"

void EnterFaultState(void);

/**
 * @brief   Assert Error if non OS function call fails
 * @param   exception non OS Error that occurred
 */
void assertExceptionError(exception_t exception);



#endif