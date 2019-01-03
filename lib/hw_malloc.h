#ifndef HW_MALLOC_H
#define HW_MALLOC_H

#include <stddef.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/mman.h>
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
typedef struct chunk_header chunk_header;

struct chunk_info_t {
    size_t prev_chunk_size: 31;// header + data
    size_t cur_chunk_size: 31;// header + data
    int alloc_flag: 1;// chunk is allocate or free
    int mmap_flag:1;// mmap or not
};
struct chunk_header {
    chunk_ptr_t prev;
    chunk_ptr_t next;
    chunk_info_t size_and_flag;
};

typedef struct bin_t {
    chunk_ptr_t prev;
    chunk_ptr_t next;
    int size;
} bin_t;

extern void *hw_malloc(size_t bytes);
extern int hw_free(void *mem);
extern void *get_start_brk(void);
extern void show_bin(const int i);

/* function*/
void show_mmap(void);
void reorder(void);
void add2list(struct chunk_header* c_h,size_t need);
chunk_header *create_chunk(chunk_header *base, const size_t need);
chunk_header *split(chunk_header **ori, const size_t need);
chunk_header *merge(chunk_header *h);
int search_debin(const size_t need);
int search_enbin(const size_t need);
void en_bin(const int index, chunk_header *c_h);
chunk_header *de_bin(const int index, const size_t need);
void rm_chunk_from_bin(chunk_header *c);
int check_valid_free(const void *mem);

#endif
