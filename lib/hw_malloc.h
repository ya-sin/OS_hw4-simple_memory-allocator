#ifndef HW_MALLOC_H
#define HW_MALLOC_H

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>

#define MMAP_THRESHOLD 32*1024
#define HEAP_SIZE 64*1024
// #define CHUNK_SIZE long long
#define BIN_SIZE 11

struct chunk_info_t {
    unsigned prev_chunk_size: 31;// header + data
    unsigned cur_chunk_size: 31;// header + data
    unsigned alloc_flag: 1;// chunk is allocate or free
    unsigned mmap_flag:1;// mmap or not
};

struct chunk_ptr_t {
    struct chunk_ptr_t* prev;
    struct chunk_ptr_t* next;
    struct chunk_info_t* size_and_flag;
};
struct heap_t {
    struct chunk_ptr_t* Bin[11];
    void* start_brk;
};

struct heap_t* HEAP;

void *hw_malloc(size_t bytes);
int hw_free(void *mem);
void *get_start_sbrk(void);

void chunk_ptr_init(struct chunk_ptr_t* tmp,size_t bytes,unsigned prev_chunk_size,unsigned alloc_flag,unsigned mmap);
void chunk_info_init(struct chunk_info_t** info);
void bin_init(struct chunk_ptr_t** Bin);
void heap_init();
void what_bytes(size_t* bytes);

#endif
