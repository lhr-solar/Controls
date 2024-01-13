/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "BSP_UART.h"
#include "common.h"

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

int _write(int fd, char *buffer, unsigned int len) {  // NOLINT
    if (buffer != NULL) {
        BspUartWrite(kUart2, buffer, len);
    }
    return (int)len;
}

int _read(int const fd, const char *buffer, unsigned const len) {  // NOLINT
    if (buffer != NULL) {}
    return 1;
}

int _close(int file) { return -1; }  // NOLINT

int _lseek(int file, int ptr, int dir) { return 0; }  // NOLINT
