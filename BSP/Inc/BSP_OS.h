/* Copyright (c) 2020 UT Longhorn Racing Solar */
#ifndef BSP_OS_H
#define BSP_OS_H

typedef struct {
    callback_t pend;
    callback_t post;
} bsp_os_t;

#endif