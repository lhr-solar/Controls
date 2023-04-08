/* Copyright (c) 2018-2022 UT Longhorn Racing Solar */
/*RTOS_ library includes all the wrapper functions for a functioning RTOS - needed for cleaner code and portability
 */

#include "RTOS_Wrapper.h"
#include "Tasks.h" // for OS errors

    /**
     * @brief   Creates a task that will be handled by the OS
     * @param   *p_tcb - pointer to the tcb
     * @param   *p_name - pointer to task name
     * @param   *p_task - pointer to the task
     * @param   *p_args - pointer to task function arguments
     * @param   prio - task priority
     * @param   *p_stk_base - the stack
     * @param   stk_size - size of the stack
     * @param   *p_err - return error code
     * @return  nothing to see here
     */
    void
    RTOS_TaskCreate(RTOS_TCB *p_tcb, char *p_name, void *p_task, void *p_arg, uint8_t prio, RTOS_CPU_STK *p_stk_base, uint64_t stk_size)
{
    #ifdef MICRIUM
    RTOS_ERR err;
    OSTaskCreate(p_tcb, p_name, p_task, p_arg, prio, p_stk_base, stk_size, stk_size, 0, 10, (void *)0, OS_OPT_TASK_STK_CHK | OS_OPT_TASK_SAVE_FP, &err);
    assertOSError(OS_RTOS_WRAPPER_LOC, err);
    #endif

    #ifdef FREE_RTOS

    #endif
}

/**
 * @brief Creates a semaphore with the initially specified count
 *
 * @param sem - pointer to a semaphore object to create and initialize
 * @param name - name of the semaphore
 * @param count - initial count for the semaphore
 */
void RTOS_SemCreate(RTOS_SEM *sem, char *name, uint32_t count)
{
    #ifdef MICRIUM
    RTOS_ERR err;
    OSSemCreate(sem, name, count, &err);
    assertOSError(OS_RTOS_WRAPPER_LOC, err);
    #endif

    #ifdef FREE_RTOS

    #endif
}

/**
 * @brief Pends a RTOS_Semaphore.
 * @param sem - pointer to a sempaphore to pend
 * @param opt - pend option
 * @return the semaphore count, or 0 if not available
 */
RTOS_SEM_CTR RTOS_SemPend(RTOS_SEM *sem, RTOS_OPT opt)
{
    #ifdef MICRIUM
    RTOS_ERR err;
    RTOS_SEM_CTR count = OSSemPend(sem, 0, opt, 0, &err); // we don't need timestamp
    assertOSError(OS_RTOS_WRAPPER_LOC, err);
    return count;
    #endif

    #ifdef FREE_RTOS

    #endif
}

/**
 * @brief Posts a semaphore
 * @param sem4 this is a semaphore pointer to post to
 * @param opt determines the type of POST performed
 * @return The current value of the semaphore counter or 0 upon error
 */
RTOS_SEM_CTR RTOS_SemPost(RTOS_SEM *sem4, RTOS_OPT opt)
{
    #ifdef MICRIUM
    RTOS_ERR err;
    RTOS_SEM_CTR counter = OSSemPost(sem4, opt, &err);
    assertOSError(OS_RTOS_WRAPPER_LOC, err);
    return counter;
    #endif

    #ifdef FREE_RTOS

    #endif
}

/**
 * @brief Initializes a mutex object.
 * @param *mut - pointer to a mutex to initialize
 * @param name - char* of the name of the mutex
 * @return none
 */
void RTOS_MutexCreate(RTOS_MUTEX *mut, char *name)
{
    #ifdef MICRIUM
    RTOS_ERR err;
    OSMutexCreate(mut, name, &err);
    assertOSError(OS_RTOS_WRAPPER_LOC, err);
    #endif

    #ifdef FREE_RTOS
    mut* = xSemaphoreCreateMutex(void);
    #endif
}

/**
 * @brief   Waits for Mutex, assigns timestamp and any error to err and ticks
 * @param   *mutex - pointer to mutex
 * @param   options - determines what the mutex will do, ie: block or not block (havent implemented for FREE_RTOS but it explains how to do it)
 * @return  none
 */
void RTOS_MutexPend(RTOS_MUTEX *mutex, RTOS_OPT opt)
{
    #ifdef MICRIUM
    RTOS_ERR err;
    OSMutexPend(mutex, 0, opt, (void *)0, &err);
    assertOSError(OS_RTOS_WRAPPER_LOC, err);
    #endif

    #ifdef FREE_RTOS
    xSemaphoreTake( mutex*, 0); //wrong and unfinished
#endif
}

/**
 * @brief   Posts the specified Mutex. (For future reference, Post is the same as Give)
 * @param   *mutex - pointer to the specified RTOS Mutex object
 * @param   options - a parameter which determines what kind of Post MutexPost performs (havent implemented for FREE_RTOS but it explains how to do it)
 * @return  none
 */
void RTOS_MutexPost(RTOS_MUTEX *mutex, RTOS_OPT options)
{
    #ifdef MICRIUM
    RTOS_ERR err;
    OSMutexPost(mutex, options, &err);
    assertOSError(OS_RTOS_WRAPPER_LOC, err);
    #endif

    #ifdef FREE_RTOS
    xSemaphoreGive(mutex *); // wrong and unfinished
#endif
}

/**
 * @brief: Creates a Millisecond/second/minutes/hour-Based Time Delay.
 * @param milli Defines how many milliseconds to delay for.
 * @param seconds Defines how many seconds to delay for.
 * @param minutes Defines how many minutes to delay for.
 * @param hours Defines how many hours to delay for.
 * @note if a (hours + minutes + seconds + milli) value is passed that is less time than the resolution of 1 tick, this code will error out
 * @return none
 */
void RTOS_DelayHMSM(CPU_INT16U hours, CPU_INT16U minutes, CPU_INT16U seconds, CPU_INT32U milli)
{
    #ifdef MICRIUM
    RTOS_ERR err;
    OSTimeDlyHMSM((CPU_INT16U)hours, (CPU_INT16U)minutes, (CPU_INT16U)seconds, (CPU_INT32U)milli, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    assertOSError(OS_RTOS_WRAPPER_LOC, err);
    #endif

    #ifdef FREE_RTOS

    #endif
}

/**
 * @brief: Creates a Tick-Based Time Delay.
 * @param dly Defines how many ticks to delay for.
 * @return none
 */
void RTOS_DelayTick(RTOS_TICK dly)
{
    #ifdef MICRIUM
    RTOS_ERR err;
    OSTimeDly(dly, OS_OPT_TIME_DLY, &err);
    assertOSError(OS_RTOS_WRAPPER_LOC, err);
    #endif

    #ifdef FREE_RTOS

    #endif
}

/**
 * @brief: Creates a Tick-Based Time Delay.
 * @param p_tmr is the pointer to the timer.
 * @param p_name is name of the timer as a char string.
 * @param dly Defines how many ticks to delay for.
 * @param period Defines how many ticks to set the period to.
 * @param P_callback is the pointer to the function to be called when timer finishes.
 * @param p_callback_arg i dont know.
 * @param timerID i have no clue: FREE_RTOS describes it as "an identifier to the timer," but the FREE_RTOS create timer function already returns the timer handler(timer create intitalizes the timer rather than intitalizing and feeding it in as a paramater)
 * @return none
 */
void RTOS_TimerCreate(RTOS_TMR p_tmr, char *p_name, RTOS_TICK dly, RTOS_TICK period, RTOS_TMR_CALLBACK_PTR P_callback, void *p_callback_arg, void *const timerID)
{
    #ifdef MICRIUM
    RTOS_ERR err;
    OSTmrCreate(
        &p_tmr,
        p_name,
        dly,
        period,
        OS_OPT_TMR_PERIODIC,
        P_callback,
        p_callback_arg,
        &err);
    assertOSError(OS_READ_CAN_LOC, err);
    #endif
    
#ifdef FREE_RTOS
    RTOS_ERR err;
    p_tmr = xTimerCreate(
        (char *)p_name,
        (RTOS_TICK)period,
        (UBaseType_t)pdFALSE,
        (void *const)timerID,
        (RTOS_TMR_CALLBACK_PTR)P_callback);
    assertOSError(OS_READ_CAN_LOC, err);
#endif
}

/**
 * @brief: Creates a Tick-Based Time Delay.
 * @param p_tmr specifies which timer handler to start.
 * @return none
 */
void RTOS_TimerStart(RTOS_TMR p_tmr)
{
    #ifdef MICRIUM
    RTOS_ERR err;
    OSTmrStart(&p_tmr, &err);
    assertOSError(OS_READ_CAN_LOC, err);
    #endif

    #ifdef FREE_RTOS
    BaseType_t WasQueueSuccessful;
    WasQueueSuccessful = xTimerStart(
        (RTOS_TMR)p_tmr,
        (RTOS_TICK)0); // blocking function: blocks start for __ ticks
    #endif
}