/* Copyright (c) 2020 UT Longhorn Racing Solar */
#ifndef BSP_OS_H
#define BSP_OS_H

typedef struct {
    void (*pend)(void);
    void (*post)(void);
} bsp_os_t;

#endif