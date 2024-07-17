/* Copyright (c) 2018-2022 UT Longhorn Racing Solar */
/*
 * This file implements a circular fifo supporting blocked puts and gets via memcpy.
 * Note that this only really improves performance for smaller datatypes (e.g. char, short)
 * 
 * In order to use it in another file, you must import it in 
 * a particular way.
 * 
 * 1. Define your data type, like so
 *    #define FAST_FIFO_TYPE int
 * 2. Define your fifo size, like so
 *    #define FAST_FIFO_SIZE (128)
 * 3. Name your fifo
 *    #define FAST_FIFO_NAME my_fifo
 * 4. Import this file
 *    #include "fastfifo.h"
 * 
 * Also, this file undef's everything at the end, so you can import
 * multiple times if you need.
 * 
 * If FAST_FIFO_NAME == my_fifo, then your new data structure will be
 * called my_fifo_t.
 * 
 * NOTE: importantly, this does not currently support usage from
 * header files. That is, all these types/functions are statically
 * declared, so there cannot be a non-static fifo at the moment.
 */

// The header guard only guard the import,
// since this file can be imported multiple times 
#ifndef __FAST_FIFO_H
#define __FAST_FIFO_H
#include <stdbool.h>
#include <string.h>
#include "common.h" // for MIN
#endif

#ifndef FFIFO_TYPE
#error "FFIFO_TYPE must be defined"
#endif
#ifndef FFIFO_SIZE
#error "FFIFO_SIZE must be defined"
#endif
#ifndef FFIFO_NAME
#error "FFIFO_NAME must be defined"
#endif

// Utility definitions
#define _CONCAT(A, B) A ## B
#define CONCAT(A, B) _CONCAT(A, B)

// Type names
#define FFIFO_STRUCT_NAME CONCAT(FFIFO_NAME, _s)
#define FFIFO_TYPE_NAME CONCAT(FFIFO_NAME, _t)

// The actual structure
typedef struct FFIFO_STRUCT_NAME {
    FFIFO_TYPE buffer[FFIFO_SIZE];
    int head;
    int tail;
    int len;
} FFIFO_TYPE_NAME;

// Define some names for our functions
#define GET      CONCAT(FFIFO_NAME, _get)
#define PUT      CONCAT(FFIFO_NAME, _put)
#define NEW      CONCAT(FFIFO_NAME, _new)
#define RENEW    CONCAT(FFIFO_NAME, _renew)
#define LEN      CONCAT(FFIFO_NAME, _len)

/**
 * @brief Initialize a new fifo
 * 
 * If the type of the fifo is myfifo_t, then this function
 * will be called myfifo_new().
 * @note PLEASE DON'T USE THIS -- returns on stack and will probably overflow
 * @deprecated
 * 
 * @return an empty fifo
 */
static inline FFIFO_TYPE_NAME __attribute__((unused))
NEW () {
    FFIFO_TYPE_NAME fifo;
    memset(&fifo, 0, sizeof(fifo));
    return fifo;
}

/**
 * @brief Initialize a fifo by reference.
 * 
 * This is going to be faster than _new().
 * This does not erase all of the contents of the fifo, but
 * rather marks it as "empty".
 */
static inline void __attribute__((unused))
RENEW (FFIFO_TYPE_NAME *fifo) {
    if (fifo) {
        fifo->head = fifo->tail = 0;
        fifo->len = 0;
    }
}

/**
 * @brief Get the current length of the fifo.
 * 
 * If the type of the fifo is myfifo_t, then this function
 * will be called myfifo_len().
 * 
 * @param fifo A pointer to the fifo
 * @return true If empty
 * @return false If not empty
 */
static int __attribute__((unused))
LEN (FFIFO_TYPE_NAME *fifo) {
    return fifo->len;
}

/**
 * @brief Get the next n elements from the fifo
 * 
 * If the type of the fifo is myfifo_t, then this function
 * will be called myfifo_get().
 * 
 * @param fifo A pointer to the fifo
 * @param data Pointer to the data buffer. 
 *             Size of buffer should be at least size * sizeof(FFIFO_TYPE) bytes
 * @param size The number of elements to get
 * @return true if successful
 * @return false if unsuccessful
 */
static bool __attribute__((unused))
GET (FFIFO_TYPE_NAME *fifo, FFIFO_TYPE *data, int size) {
    if (size > fifo->len) {
        return false;
    }

    int hi = MIN(size, FFIFO_SIZE - fifo->head);
    int lo = size - hi;

    memcpy(data, &fifo->buffer[fifo->head], hi * sizeof(FFIFO_TYPE));
    memcpy(&data[hi], &fifo->buffer[0], lo * sizeof(FFIFO_TYPE));

    fifo->head = (fifo->head + size) % FFIFO_SIZE;
    fifo->len -= size;
    return true;
}

/**
 * @brief Put an element into the fifo
 * 
 * If the type of the fifo is myfifo_t, then this function
 * will be called myfifo_put().
 * 
 * @param fifo A pointer to the fifo
 * @param data Pointer to the data buffer. 
 *             Size of buffer should be at least size * sizeof(FFIFO_TYPE) bytes
 * @param size The number of elements to get
 * @return true if successful
 * @return false if unsuccessful
 */
static bool __attribute__((unused))
PUT (FFIFO_TYPE_NAME *fifo, FFIFO_TYPE *data, int size) {
    if (size > (FFIFO_SIZE - fifo->len)){
        return false;
    }

    int hi = MIN(size, FFIFO_SIZE - fifo->tail);
    int lo = size - hi;

    memcpy(&fifo->buffer[fifo->tail], data, hi * sizeof(FFIFO_TYPE));
    memcpy(&fifo->buffer[0], &data[hi], lo * sizeof(FFIFO_TYPE));

    fifo->tail = (fifo->tail + size) % FFIFO_SIZE;
    fifo->len += size;
    return true;
}

#undef FFIFO_SIZE
#undef FFIFO_TYPE
#undef FFIFO_NAME
#undef FFIFO_TYPE_NAME
#undef FFIFO_STRUCT_NAME
#undef GET
#undef PUT
#undef NEW
#undef RENEW
#undef LEN
#undef CONCAT
#undef _CONCAT
