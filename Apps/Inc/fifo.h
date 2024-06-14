/**
 * @file fifo.h
 * @brief
 *
 */

/*
 * This file implements a fifo.
 *
 * In order to use it in another file, you must import it in
 * a particular way.
 *
 * 1. Define your data type, like so
 *    #define FIFO_TYPE int
 * 2. Define your fifo size, like so
 *    #define FIFO_SIZE (128)
 * 3. Name your fifo
 *    #define FIFO_NAME my_fifo
 * 4. Import this file
 *    #include "fifo.h"
 *
 * This file includes some defaults, but they might not work for
 * your case!
 *
 * Also, this file undef's everything at the end, so you can import
 * multiple times if you need.
 *
 * If FIFO_NAME == my_fifo, then your new data structure will be
 * called my_fifo_t.
 *
 * NOTE: importantly, this does not currently support usage from
 * header files. That is, all these types/functions are statically
 * declared, so there cannot be a non-static fifo at the moment.
 */

// The header guard only guard the import,
// since this file can be imported multiple times
#ifndef FIFO_H
#define FIFO_H
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#endif

// The type of the fifo
#ifndef FIFO_TYPE
#define FIFO_TYPE int
#endif

// The number of elements in the fifo
#ifndef FIFO_SIZE
#define FIFO_SIZE 64
#endif

// The name of the fifo (minus the _t)
#ifndef FIFO_NAME
#define FIFO_NAME new_fifo
#endif

// Utility definitions
#define _CONCAT(A, B) A##B
#define CONCAT(A, B) _CONCAT(A, B)

// Type names
#define FIFO_STRUCT_NAME FIFO_NAME
#define FIFO_TYPE_NAME FIFO_NAME

// The actual structure
typedef struct FIFO_STRUCT_NAME {
    FIFO_TYPE buffer[FIFO_SIZE];
    int put;
    int get;
} FIFO_TYPE_NAME;

// Define some names for our functions
#define IS_EMPTY CONCAT(FIFO_NAME, IsEmpty)
#define IS_FULL CONCAT(FIFO_NAME, IsFull)
#define GET CONCAT(FIFO_NAME, Get)
#define PUT CONCAT(FIFO_NAME, Put)
#define NEW CONCAT(FIFO_NAME, New)
#define PEEK CONCAT(FIFO_NAME, Peek)
#define POPBACK CONCAT(FIFO_NAME, PopBack)
#define RENEW CONCAT(FIFO_NAME, Renew)

/**
 * @brief Initialize a new fifo
 *
 * If the type of the fifo is myfifo_t, then this function
 * will be called myfifoNew().
 *
 * @return an empty fifo
 */
static inline FIFO_TYPE_NAME __attribute__((unused)) NEW() {
    FIFO_TYPE_NAME fifo;
    memset(&fifo, 0, sizeof(fifo));
    return fifo;
}

/**
 * @brief Initialize a fifo by reference.
 *
 * This is going to be faster than New().
 * This does not erase all of the contents of the fifo, but
 * rather marks it as "empty".
 */
static inline void __attribute__((unused)) RENEW(FIFO_TYPE_NAME *fifo) {
    if (fifo != NULL) {
        fifo->get = fifo->put;
    }
}

/**
 * @brief Determine whether the fifo is empty.
 *
 * If the type of the fifo is myfifo_t, then this function
 * will be called myfifoIsEmpty().
 *
 * @param fifo A pointer to the fifo
 * @return true If empty
 * @return false If not empty
 */
static bool IS_EMPTY(FIFO_TYPE_NAME *fifo) { return fifo->put == fifo->get; }

/**
 * @brief Determine whether the fifo is full.
 *
 * If the type of the fifo is myfifo_t, then this function
 * will be called myfifioIsFull().
 *
 * @param fifo A pointer to the fifo
 * @return true If full
 * @return false If not full
 */
static bool IS_FULL(FIFO_TYPE_NAME *fifo) {
    return (fifo->put + 1) % FIFO_SIZE == fifo->get;
}

/**
 * @brief Get the next element from the fifo
 *
 * If the type of the fifo is myfifo_t, then this function
 * will be called myfifoGet().
 *
 * @param fifo A pointer to the fifo
 * @param elem A pointer to an element to use for storage
 * @return true if successful
 * @return false if unsuccessful
 */
static bool __attribute__((unused)) GET(FIFO_TYPE_NAME *fifo, FIFO_TYPE *elem) {
    if (!IS_EMPTY(fifo)) {
        *elem = fifo->buffer[fifo->get];
        fifo->get = (fifo->get + 1) % FIFO_SIZE;
        return true;
    }

    return false;
}

/**
 * @brief Put an element into the fifo
 *
 * If the type of the fifo is myfifo_t, then this function
 * will be called myfifoPut().
 *
 * @param fifo A pointer to the fifo
 * @param elem The element to put
 * @return true if successful
 * @return false if unsuccessful
 */
static bool __attribute__((unused)) PUT(FIFO_TYPE_NAME *fifo, FIFO_TYPE elem) {
    if (!IS_FULL(fifo)) {
        fifo->buffer[fifo->put] = elem;
        fifo->put = (fifo->put + 1) % FIFO_SIZE;
        return true;
    }

    return false;
}

/**
 * @brief Peek into the fifo
 *
 * If the type of the fifo is myfifo_t, then this function
 * will be called myfifoPeek().
 *
 * @param fifo A pointer to the fifo
 * @param elem A pointer to space where the next element will be put
 * @return true if successful
 * @return false if unsuccessful
 */
static bool __attribute__((unused))
PEEK(FIFO_TYPE_NAME *fifo, FIFO_TYPE *elem) {
    if (!IS_EMPTY(fifo)) {
        *elem = fifo->buffer[fifo->get];
        return true;
    }

    return false;
}

/**
 * @brief Take the last element of the fifo (most recent)
 *
 * If the type of the fifo is myfifo_t, then this function
 * will be called myfifoPopBack().
 *
 * @param fifo A pointer to the fifo
 * @param elem A pointer to space to put the element
 * @return true if successful
 * @return false if unsuccessful
 */
static bool __attribute__((unused))
POPBACK(FIFO_TYPE_NAME *fifo, FIFO_TYPE *elem) {
    if (!IS_EMPTY(fifo)) {
        fifo->put = (fifo->put + FIFO_SIZE - 1) % FIFO_SIZE;
        *elem = fifo->buffer[fifo->put];
        return true;
    }

    return false;
}

#undef IS_EMPTY
#undef IS_FULL
#undef FIFO_SIZE
#undef FIFO_TYPE
#undef FIFO_NAME
#undef FIFO_TYPE_NAME
#undef FIFO_STRUCT_NAME
#undef GET
#undef PUT
#undef NEW
#undef PEEK
#undef POPBACK
#undef CONCAT
#undef _CONCAT
