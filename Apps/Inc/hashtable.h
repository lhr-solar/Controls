// Copyright (c) 2023 Longhorn Racing Solar
/*
 * Author: Sidharth N. Babu, sidharth.n.babu@gmail.com
 * 
 * This file implements a hashtable.
 * 
 * In order to use it in another file, you must import it in 
 * a particular way.
 * 
 * 1. Define your data and key type, like so
 *    #define HASH_TABLE_DATA_TYPE int
 *    #define HASH_TABLE_KEY_TYPE int
 * 2. Define your hashtable size, like so NOTE: this must be less than or equal to 256.
 *    #define HASH_TABLE_SIZE 256
 * 3. Name your hashtable, like so
 *    #define HASH_TABLE_NAME my_fifo
 * 4. Import this file
 *    #include "hashtable.h"
 * 
 * This file includes some defaults, but they might not work for
 * your case!
 * 
 * Also, this file undef's everything at the end, so you can import
 * multiple times if you need.
 * 
 * If HASH_TABLE_NAME == my_hashtable, then your new data structure will be
 * called my_hashtable_table_t.
 * 
 * NOTE: importantly, this does not currently support usage from
 * header files. That is, all these types/functions are statically
 * declared, so there cannot be a non-static hashtable at the moment.
 */


#ifndef _HASHTABLE_COMMON
#define _HASHTABLE_COMMON
//This section contains resources common to any hashtable initialized
#include "common.h"
//This is the lookup table for the hash function
//It is a 256 element array, consisting of a random permutation of numbers from 0-255
uint8_t lookup[256] =  {232, 36, 152, 47, 166, 250, 86, 137, 184, 65, 144, 
                    175, 79, 221, 228, 109, 73, 185, 133, 58, 57, 242,
                    84, 225, 34, 156, 123, 174, 121, 4, 241, 224, 106,
                    124, 17, 111, 42, 155, 246, 50, 170, 159, 255, 139,
                    223, 76, 60, 29, 8, 173, 193, 214, 52, 122, 163, 0,
                    113, 53, 191, 77, 80, 162, 72, 88, 231, 85, 108, 234,
                    211, 227, 91, 205, 103, 134, 51, 243, 11, 28, 204,
                    167, 244, 30, 97, 87, 105, 237, 141, 206, 216, 226,
                    45, 43, 149, 236, 218, 158, 215, 61, 251, 2, 20,
                    190, 233, 160, 95, 212, 9, 119, 169, 117, 143,
                    202, 177, 142, 127, 92, 196, 116, 182, 98, 239,
                    148, 70, 219, 208, 154, 188, 56, 63, 129, 179,
                    203, 21, 35, 130, 59, 26, 153, 217, 18, 44, 69,
                    33, 229, 39, 81, 171, 195, 15, 102, 213, 78,
                    120, 71, 54, 176, 192, 199, 140, 3, 180, 10,
                    93, 90, 197, 19, 253, 25, 40, 14, 112, 99,
                    132, 101, 13, 252, 201, 22, 220, 1, 48, 146,
                    189, 23, 68, 110, 24, 125, 240, 107, 37, 7,
                    165, 161, 6, 128, 238, 164, 38, 135, 248, 16,
                    32, 178, 94, 200, 247, 168, 66, 41, 5, 145, 
                    198, 183, 89, 230, 126, 104, 75, 254, 249, 222,
                    31, 136, 150, 114, 157, 235, 147, 83, 151, 118,
                    138, 49, 194, 131, 74, 82, 210, 187, 96, 100, 27,
                    64, 209, 62, 67, 172, 46, 12, 181, 186, 207, 55,
                    245, 115};
#endif


//data type of hash table
#ifndef HASH_TABLE_DATA_TYPE
#define HASH_TABLE_DATA_TYPE int
#endif

//key type of hash table
#ifndef HASH_TABLE_KEY_TYPE
#define HASH_TABLE_KEY_TYPE int
#endif

//number of elements in the hashtable
#ifndef HASH_TABLE_SIZE
#define HASH_TABLE_SIZE 256
#endif

//name of the hashtable
#ifndef HASH_TABLE_NAME
#define HASH_TABLE_NAME new_hashtable
#endif

// Utility definitions
#define _CONCAT(A, B) A ## B
#define CONCAT(A, B) _CONCAT(A, B)

//Type names
#define HASH_ENTRY_STRUCT_NAME CONCAT(HASH_TABLE_NAME, _entry_s)
#define HASH_ENTRY_TYPE_NAME CONCAT(HASH_TABLE_NAME, _entry_t)
#define HASH_TABLE_STRUCT_NAME CONCAT(HASH_TABLE_NAME, _table_s)
#define HASH_TABLE_TYPE_NAME CONCAT(HASH_TABLE_NAME, _table_t)

//the struct def for the entries in the hash table
typedef struct HASH_ENTRY_STRUCT_NAME {
    HASH_TABLE_KEY_TYPE key;
    HASH_TABLE_DATA_TYPE data;
    bool inUse;
} HASH_ENTRY_TYPE_NAME;

//the struct def for the entire table
typedef struct HASH_TABLE_STRUCT_NAME {
    //actual data
    HASH_ENTRY_TYPE_NAME table[HASH_TABLE_SIZE]; 
    //other metadata
    int remaining; //number of remaining elements
} HASH_TABLE_TYPE_NAME;

/**
 * @brief Initialize a new hash table
 * 
 * If the type of the fifo is new_hashtable, then this function
 * will be called new_hashtable_init()
 * 
 * @return an empty hashtable
*/

#define INIT CONCAT(HASH_TABLE_NAME, _init)
static inline HASH_TABLE_TYPE_NAME INIT(){
    HASH_TABLE_TYPE_NAME table;
    memset(&table, 0, sizeof(table));
    table.remaining = HASH_TABLE_SIZE;
    return table;
}

/**
 * @brief Insert an element into the hash table
 * 
 * If the type of the fifo is new_hashtable, then this function
 * will be called new_hashtable_insert()
 * 
 * @param table the table to insert into
 * @param key the key to insert
 * @param data the data to insert
 * @return true if the insertion was successful
*/
#define INSERT CONCAT(HASH_TABLE_NAME, _insert)
static inline bool INSERT(HASH_TABLE_TYPE_NAME *table, HASH_TABLE_KEY_TYPE key, HASH_TABLE_DATA_TYPE data){
    //if the table is full, return false
    if(table->remaining == 0){
        return false;
    }
    //get the hash of the key using a pearson hash
    uint8_t hash = 0;
    uint8_t *keyref = (uint8_t*)&key;
    for(int i = 0; i < sizeof(key); ++i){
        hash = lookup[hash ^ keyref[i]];
    }
    hash = hash % HASH_TABLE_SIZE;

    //if the hash is already taken, go to next available slot
    while(table->table[hash].inUse){
        hash = (hash + 1) % HASH_TABLE_SIZE;
    }
    //otherwise, insert the data
    table->table[hash].key = key;
    table->table[hash].data = data;
    table->table[hash].inUse = true;
    table->remaining--;
    return true;
}

/**
 * @brief Lookup an element in the hash table
 * 
 * If the type of the fifo is new_hashtable, then this function
 * will be called new_hashtable_lookup()
 * 
 * @param table the table to lookup in
 * @param key the key to lookup
 * @param data pointer to store the result in
 * @return true if the lookup was successful, false otherwise
*/
#define LOOKUP CONCAT(HASH_TABLE_NAME, _lookup)
static inline bool LOOKUP(HASH_TABLE_TYPE_NAME *table, HASH_TABLE_KEY_TYPE key, HASH_TABLE_DATA_TYPE *data){
    //get the hash of the key using a pearson hash
    uint8_t hash = 0;
    uint8_t *keyref = (uint8_t*)&key;
    for(int i = 0; i < sizeof(key); ++i){
        hash = lookup[hash ^ keyref[i]];
    }
    hash = hash % HASH_TABLE_SIZE;

    for(int i = 0;i<HASH_TABLE_SIZE;++i){
        //if we are on an active slot, and the keys match, return data and true
        if(table->table[hash].inUse && table->table[hash].key == key){
            *data = table->table[hash].data;
            return true;
        }
        //go to next slot
        hash = (hash + 1) % HASH_TABLE_SIZE;
    }
    return false;
}

/**
 * @brief Delete an element from the hash table
 * 
 * If the type of the fifo is new_hashtable, then this function
 * will be called new_hashtable_delete()
 * 
 * @param table the table to delete from
 * @param key the key to delete
 * @return true if the deletion was successful, false otherwise
*/
#define DELETE CONCAT(HASH_TABLE_NAME, _delete)
static inline bool DELETE(HASH_TABLE_TYPE_NAME *table, HASH_TABLE_KEY_TYPE key){
    //get the hash of the key using a pearson hash
    uint8_t hash = 0;
    uint8_t *keyref = (uint8_t*)&key;
    for(int i = 0; i < sizeof(key); ++i){
        hash = lookup[hash ^ keyref[i]];
    }
    hash = hash % HASH_TABLE_SIZE;

    for(int i = 0;i<HASH_TABLE_SIZE;++i){
        //if we are on an active slot, and the keys match, delete the data and return true
        if(table->table[hash].inUse && table->table[hash].key == key){
            table->table[hash].inUse = false;
            ++(table->remaining);
            return true;
        }
        //go to next slot
        hash = (hash + 1) % HASH_TABLE_SIZE;
    }
    return false;
}

/**
 * @brief Check if the hash table is empty
*/
#define EMPTY CONCAT(HASH_TABLE_NAME, _empty)
static inline bool EMPTY (HASH_TABLE_TYPE_NAME *table){
    return table->remaining == HASH_TABLE_SIZE;
}

/**
 * @brief Get the number of elements in the hash table
*/
#define SIZE CONCAT(HASH_TABLE_NAME, _size)
static inline int SIZE(HASH_TABLE_TYPE_NAME *table){
    return HASH_TABLE_SIZE - table->remaining;
}

/**
 * @brief Returns the maxmimum amount of elements the table can store
*/
#define MAX_SIZE CONCAT(HASH_TABLE_NAME, _max_size)
static inline int MAX_SIZE(){
    return HASH_TABLE_SIZE;
}

/**
 * @brief Check if the hash table is full
*/
#define FULL CONCAT(HASH_TABLE_NAME, _full)
static inline bool FULL(HASH_TABLE_TYPE_NAME *table){
    return table->remaining == 0;
}

/**
 * @brief Clear the hash table
*/
#define CLEAR CONCAT(HASH_TABLE_NAME, _clear)
static inline void CLEAR(HASH_TABLE_TYPE_NAME *table){
    memset(table, 0, sizeof(*table));
    table->remaining = HASH_TABLE_SIZE;
}

#undef INIT
#undef INSERT
#undef LOOKUP
#undef DELETE
#undef EMPTY
#undef SIZE
#undef MAX_SIZE
#undef FULL
#undef CLEAR