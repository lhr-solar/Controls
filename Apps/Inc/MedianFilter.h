/**
 * @file MedianFilter.h
 * @brief 
 * 
 */

/*
 * This file implements a median filter.
 * 
 * In order to use it in another file, you must import it in 
 * a particular way.
 * 
 * 1. Define your data type, like so
 *    #define MEDIAN_FILTER_TYPE int
 * 2. Define your filter depth, like so
 *    #define MEDIAN_FILTER_DEPTH (128)
 * 3. Define the number of channels in your filter, like so
 *    #define MEDIAN_FILTER_CHANNELS (31)
 * 3. Name your median filter
 *    #define MEDIAN_FILTER_NAME my_fifo
 * 4. Import this file
 *    #include "MedianFilter.h"
 * 
 * This file includes some defaults, but they might not work for
 * your case!
 * 
 * Also, this file undef's everything at the end, so you can import
 * multiple times if you need.
 * 
 * If MEDIAN_FILTER_NAME == my_filter, then your new data structure will be
 * called my_filter_t.
 * 
 * NOTE: importantly, this does not currently support usage from
 * header files. That is, all these types/functions are statically
 * declared, so there cannot be a non-static fifo at the moment.
 */

// The header guard only guard the import,
// since this file can be imported multiple times 
#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H
#include <stdint.h>
#include <string.h>
#endif

// The type of the median filter
#ifndef MEDIAN_FILTER_TYPE
#define MEDIAN_FILTER_TYPE int32_t
#endif

// The depth of the median filter
#ifndef MEDIAN_FILTER_DEPTH
#define MEDIAN_FILTER_DEPTH 3
#endif

// The number of channels in the median filter
#ifndef MEDIAN_FILTER_CHANNELS
#define MEDIAN_FILTER_CHANNELS 1
#endif

// The name of the median filter (minus the _t)
#ifndef MEDIAN_FILTER_NAME
#define MEDIAN_FILTER_NAME define_your_filter_type
#endif

// Utility definitions
#define _CONCAT(A, B) A ## B
#define CONCAT(A, B) _CONCAT(A, B)

// some shorthand
#define MF_TYPE     MEDIAN_FILTER_TYPE
#define MF_DEPTH    MEDIAN_FILTER_DEPTH
#define MF_CHANNELS MEDIAN_FILTER_CHANNELS
#define MF_NAME     MEDIAN_FILTER_NAME

// Type names
#define MEDIAN_FILTER_STRUCT_NAME CONCAT(MF_NAME, _s)
#define MEDIAN_FILTER_TYPE_NAME CONCAT(MF_NAME, _t)

// more shorthand
#define MF_STRUCT_NAME  MEDIAN_FILTER_STRUCT_NAME
#define MF_TYPE_NAME    MEDIAN_FILTER_TYPE_NAME 

// The actual structure
typedef struct MF_STRUCT_NAME {
    MF_TYPE raw[MF_CHANNELS][MF_DEPTH];
    MF_TYPE filtered[MF_CHANNELS];
    uint32_t index;
} MF_TYPE_NAME;

// Define some names for our functions
#define MEDIAN      CONCAT(MF_NAME, _median)
#define INIT        CONCAT(MF_NAME, _init)
#define GET         CONCAT(MF_NAME, _get)
#define PUT         CONCAT(MF_NAME, _put)
#define GETSINGLE   CONCAT(MF_NAME, _getSingle)

/**
 * @brief Helper function to find the median of an array of MF_TYPE with length MF_DEPTH.
 *        DO NOT call this function directly.
 * 
 * @param channel   the channel in the median filter to find the median of
 */
static inline MF_TYPE __attribute__((unused))
MEDIAN (MF_TYPE *channel) {
    static MF_TYPE sorted[MF_DEPTH];

    // copy channels into temporary array
    memcpy(sorted, channel, MF_DEPTH * sizeof(MF_TYPE));

    // sort temporary array
    for (uint32_t i = 0; i < MF_DEPTH; ++i) {
        MF_TYPE min = sorted[i];
        uint32_t minIdx = i;
        for (uint32_t j = i + 1; j < MF_DEPTH; ++j) {
            if (sorted[j] < min) {
                min = sorted[j];
                minIdx = j;
            }
        }
        sorted[minIdx] = sorted[i];
        sorted[i] = min;
    }

    // return median
    return sorted[MF_DEPTH >> 1];
}

/**
 * @brief Initialize a new median filter
 * 
 * If the type of the filter is myfilter_t, then this function
 * will be called myfilter_init().
 * 
 * @param filter    a pointer to the median filter to initialize
 * @param low       a value that is below the range of expected values
 * @param high      a value that is above the range of expected values
 */
static inline void __attribute__((unused))
INIT (MF_TYPE_NAME *filter, MF_TYPE low, MF_TYPE high) {
    // intialize the filter with alternating low and high values, so it will be stable at startup
    for (uint32_t channel = 0; channel < MF_CHANNELS; ++channel) {
        for (uint32_t i = 0; i < MF_DEPTH - 1; ++i) {
            filter->raw[channel][i] = (i & 1) ? high : low;
        }
        filter->filtered[channel] = MEDIAN(filter->raw[channel]);
    }

    filter->index = MF_DEPTH - 1;
}

/**
 * @brief update the median filter by giving it a new set of values for all channels
 * 
 * @param filter    a pointer to the median filter
 * @param channels  a complete set of new values for all channels to add to the median filter
 * 
 */
static inline void __attribute__((unused))
PUT (MF_TYPE_NAME *filter, MF_TYPE *channels) {
    // put the new data into the filter
    for (uint32_t channel = 0; channel < MF_CHANNELS; ++channel) {
        filter->raw[channel][filter->index] = channels[channel];
    }
    (filter->index) = (filter->index + 1) % MF_DEPTH;

    // update the list of filtered values
    for (uint32_t channel = 0; channel < MF_CHANNELS; ++channel) {
        filter->filtered[channel] = MEDIAN(filter->raw[channel]);
    }
}

/**
 * @brief get a complete set of filtered values for all channels
 * 
 * @param filter    a pointer to the median filter
 * @param dest      a pointer to a buffer to store all of the filtered values
 */
static inline void __attribute__((unused))
GET (MF_TYPE_NAME *filter, MF_TYPE *dest) {
    memcpy(dest, filter->filtered, sizeof(MF_TYPE) * MF_CHANNELS);
}

/**
 * @brief get a filtered value for a single channel in the median filter
 * 
 * @param filter    a pointer to the median filter
 * @param channel   the channel to read
 * @return the filtered value
 */
static inline MF_TYPE __attribute__((unused))
GETSINGLE (MF_TYPE_NAME *filter, uint32_t channel) {
    return filter->filtered[channel];
}

// undef everything, so this file can be included multiple times
#undef MEDIAN_FILTER_TYPE
#undef MEDIAN_FILTER_DEPTH
#undef MEDIAN_FILTER_CHANNELS
#undef MEDIAN_FILTER_NAME
#undef _CONCAT
#undef CONCAT
#undef MF_TYPE
#undef MF_DEPTH
#undef MF_CHANNELS
#undef MF_NAME
#undef MEDIAN_FILTER_STRUCT_NAME
#undef MEDIAN_FILTER_TYPE_NAME
#undef MF_STRUCT_NAME
#undef MF_TYPE_NAME
#undef MEDIAN
#undef INIT
#undef GET
#undef PUT
#undef GETSINGLE 

