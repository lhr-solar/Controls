/* Copyright (c) 2018-2022 UT Longhorn Racing Solar */
/*RTOS_ library includes all the wrapper functions for a functioning RTOS - needed for cleaner code and portability
 */
#ifndef RTOS_H
#define RTOS_H

#include "os.h" // for RTOS stuff
#include <stdint.h>

// Currently using Micrium
typedef OS_MUTEX RTOS_MUTEX;
typedef OS_SEM RTOS_SEM;
typedef OS_OPT RTOS_OPT;
typedef OS_ERR RTOS_ERR;
typedef OS_SEM RTOS_SEM;
typedef OS_TCB RTOS_TCB;
typedef CPU_TS RTCPU_TS;
typedef OS_SEM_CTR RTOS_SEM_CTR;
typedef OS_TICK RTOS_TICK;

// Custom typedefs for RTOS_TaskCreate function currently using micirium
typedef OS_TCB RTOS_TCB;
typedef CPU_STK RTOS_CPU_STK;

/**
 * @brief Pends a RTOS_Semaphore.
 * @param *sem - pointer to a sempaphore to pend
 * @param tick - time in clock ticks to timeout for
 * @param opt - pend option
 * @return the semaphore count, or 0 if not available
 */
RTOS_SEM_CTR RTOS_SemPend(RTOS_SEM *sem, RTOS_OPT opt);

/**
 * @brief Posts a semaphore
 * @param sem4 this is a semaphore pointer to post to
 * @param opt determines the type of POST performed
 * @return The current value of the semaphore counter or 0 upon error
 */
RTOS_SEM_CTR RTOS_SemPost(RTOS_SEM *sem4, RTOS_OPT opt);

/**
 * @brief Initializes a mutex object.
 * @param *mut - pointer to a mutex to initialize
 * @param name - char* of the name of the mutex
 * @return none
 */
void RTOS_MutexCreate(RTOS_MUTEX *mut, char *name);

/**
 * @brief   Function waits for Mutex
 * @param   *mutex - pointer to mutex
 * @param   options - determines what the mutex will do, ie: block or not block
 */
void RTOS_MutexPend(RTOS_MUTEX *mutex, RTOS_OPT opt);

/**
 * @brief   Posts the specified Mutex
 * @param   *mutex - pointer to the specified RTOS Mutex object
 * @param   options - a parameter which determines what kind of Post MutexPost performs
 * @return  none
 */
void RTOS_MutexPost(RTOS_MUTEX *mutex, RTOS_OPT options);

/**
 * @brief   Creates a task that will be handled by the OS
 * @param   *p_tcb - pointer to the tcb
 * @param   *p_name - pointer to task name
 * @param   p_task - pointer to the task
 * @param   *p_args - pointer to task function arguments
 * @param   *prio - task priority
 * @param   *p_stk_base - the stack
 * @param   stk_size - size of the stack
 * @return  nothing to see here
 */
void RTOS_TaskCreate(
    RTOS_TCB *p_tcb,
    char *p_name,
    void *p_task,
    void *p_arg,
    uint8_t prio,
    RTOS_CPU_STK *p_stk_base,
    uint64_t stk_size);

/**
 * @brief Creates a Second-based Time Delay.
 * @param dly Defines how many seconds to delay for.
 * @return none
 */
void RTOS_DelaySecs(uint16_t dly);

/**
 * @brief Creates a Millisecond-based Time Delay.
 * @param dly Defines how many milliseconds to delay for.
 * @return none
 */
void RTOS_DelayMs(uint16_t dly);

/**
 * @brief Creates a semaphore with the initially specified count
 *
 * @param sem - pointer to a semaphore object to create and initialize
 * @param name - name of the semaphore
 * @param count - initial count for the semaphore
 */
void RTOS_SemCreate(RTOS_SEM *sem, char *name, uint32_t count);

/**
 * @brief Creates a Tick-based Time Delay.
 * @param dly Defines how many ticks to delay for.
 * @return none
 */
void RTOS_DelayTick(RTOS_TICK dly);

#endif
