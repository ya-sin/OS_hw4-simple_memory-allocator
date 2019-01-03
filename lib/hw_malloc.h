#ifndef HW_MALLOC_H
#define HW_MALLOC_H

#include <stddef.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

// #define DEBUG
#ifdef DEBUG
#define PRINTERR(s)\
    fprintf(stderr, "\033[0;32;31m""%d: %s""\033[m", __LINE__, s);
#else
#define PRINTERR(s);
#endif
#define MMAP_THRESHOLD 32*1024
#define HEAP_SIZE 64*1024
typedef void *chunk_ptr_t;
typedef struct chunk_info_t chunk_info_t;
typedef struct tt tt;

// typedef void *chunk_info_t;
typedef long long chunk_size_t;
typedef long long chunk_flag_t;
typedef struct chunk_header chunk_header;

struct chunk_info_t {
    unsigned prev_chunk_size: 31;// header + data
    unsigned cur_chunk_size: 31;// header + data
    unsigned alloc_flag: 1;// chunk is allocate or free
    unsigned mmap_flag:1;// mmap or not
};
struct tt {
    int qq;
};
struct chunk_header {
    chunk_ptr_t prev;
    chunk_ptr_t next;
    tt * test;
    chunk_info_t* size_and_flag;
    chunk_size_t chunk_size;
    chunk_size_t prev_chunk_size;
    chunk_flag_t prev_free_flag;
};


// struct chunk_ptr_t {
//     struct chunk_ptr_t* prev;
//     struct chunk_ptr_t* next;
//     struct chunk_info_t* size_and_flag;
// };
typedef struct bin_t {
    chunk_ptr_t prev;
    chunk_ptr_t next;
    int size;
} bin_t;

extern void *hw_malloc(size_t bytes);
extern int hw_free(void *mem);
extern void *get_start_brk(void);
extern void show_bin(const int i);
extern void watch_heap();

#endif
