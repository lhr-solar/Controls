/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file BSP_OS.h
 * @brief 
 * 
 * @defgroup BSP_OS
 * @addtogroup BSP_OS
 * @{
 */

#ifndef BSP_OS_H
#define BSP_OS_H

typedef struct {
    callback_t pend;
    callback_t post;
} bsp_os_t;

#endif


/* @} */
