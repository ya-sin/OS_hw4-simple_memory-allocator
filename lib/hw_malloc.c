#include "hw_malloc.h"
void chunk_info_init(struct chunk_info_t** info)
{
    *info = (struct chunk_info_t*)malloc(sizeof(struct chunk_info_t));
    (*info)->prev_chunk_size = sizeof(struct chunk_ptr_t);
    (*info)->cur_chunk_size = sizeof(struct chunk_ptr_t);
    (*info)->alloc_flag = 0;
    (*info)->mmap_flag = 0;
    printf("done\n");
}
void bin_init(struct chunk_ptr_t** Bin)
{
    *Bin = (struct chunk_ptr_t*)malloc(sizeof(struct chunk_ptr_t));
    (*Bin)->prev = (*Bin);
    (*Bin)->next = (*Bin);
    chunk_info_init((&(*Bin)->size_and_flag));
}
void chunk_ptr_init(struct chunk_ptr_t* tmp,size_t bytes,unsigned prev_chunk_size,unsigned alloc_flag,unsigned mmap_flag)
{

    struct chunk_info_t* tmp2;
    tmp2 = (struct chunk_info_t*)malloc(sizeof(struct chunk_info_t));
    tmp2->prev_chunk_size = prev_chunk_size;
    tmp2->cur_chunk_size = bytes;
    tmp2->alloc_flag = alloc_flag;
    tmp2->mmap_flag = mmap_flag;
    tmp->size_and_flag = tmp2;
    printf("in chunl_ptr_init \n");
    tmp->prev = NULL;
    tmp->next = NULL;
    printf("new alloc_FLAG %d\n",tmp->size_and_flag->alloc_flag);

}
struct chunk_ptr_t* get_uchunk_header(struct chunk_ptr_t* add_pos)
{
    // assert(add_pos != NULL);
    struct chunk_ptr_t* re_header = (struct chunk_ptr_t*)((
                                        void*)add_pos+add_pos->size_and_flag->cur_chunk_size);
    if((void*)re_header >= (void*)HEAP->start_brk+HEAP_SIZE) {
        re_header = (void*)re_header-HEAP_SIZE;
        printf("kkk\n");
    }
    return re_header;
}
void renew_upper_chunk(struct chunk_ptr_t* entry)
{
    struct chunk_ptr_t* upper_chunk = get_uchunk_header(entry);
    // assert(upper_chunk!=NULL);
    upper_chunk->size_and_flag->prev_chunk_size = entry->size_and_flag->cur_chunk_size;
    upper_chunk->size_and_flag->alloc_flag = 0;
}
void split(struct chunk_ptr_t* tmp)
{

}
void add2bin(struct heap_t* HEAP,struct chunk_ptr_t* tmp)
{
    int chunk_size,bin=0;
    chunk_size = (int)tmp->size_and_flag->cur_chunk_size;
    while(chunk_size!=1) {
        chunk_size = chunk_size>>1;
        bin++;
        // printf("bin num %d %d\n",i,bin);
    }
    if(bin>=16) {
        split(tmp);
    } else {
        renew_upper_chunk(tmp);
        struct chunk_ptr_t* listptr;
        struct chunk_ptr_t* bin_head = HEAP->Bin[bin];
        for(listptr = bin_head->next; ; listptr=listptr->next) {
            if(tmp->size_and_flag->cur_chunk_size > listptr->size_and_flag->cur_chunk_size) {
                // __list_add(tmp,listptr->prev,listptr);
                break;
            }
        }
    }
}
void heap_init(struct heap_t** HEAP)
{
    *HEAP = (struct heap_t*)malloc(sizeof(struct heap_t));
    (*HEAP)->start_brk = (void*)sbrk(HEAP_SIZE);// fail -> (void*)-1
    if((*HEAP)->start_brk == (void*)-1)
        printf("fail heap\n");
    else
        printf("%p\n",(*HEAP)->start_brk);
    // Bin init
    for(int i = 0; i<BIN_SIZE; i++) {
        // printf("init");
        bin_init((&(*HEAP)->Bin[i]));
    }
    // from start_brk to start up a new chunk header
    chunk_ptr_init((struct chunk_ptr_t*)(*HEAP)->start_brk,HEAP_SIZE,1,0,0);
    // append to bin
    add2bin(*HEAP,(struct chunk_ptr_t*)(*HEAP)->start_brk);
}
void what_bytes(size_t* bytes)
{
    *bytes = ((*bytes-1)/8+1)*8;
}
void *hw_malloc(size_t bytes)
{
    int header_size = 24;
    printf("in function");
    if(bytes+header_size>MMAP_THRESHOLD) {
        printf("mmap %ld\n",bytes+header_size);
        // mmap()
        // chunk
        // bytes+header_size
        //mmap_alloc_list()
    } else {
        printf("heap");
        what_bytes(&bytes);
        // chunk(header+data) and bin
        // heap initialization
        //Heap Initialization
        if(HEAP==NULL) {
            printf("NULL\n");
            heap_init(&HEAP);
            printf("alloc_FLAG %d\n",HEAP->Bin[0]->size_and_flag->alloc_flag);
            printf("mmap_FLAG %d\n",HEAP->Bin[0]->size_and_flag->mmap_flag);
            printf("prev_chunksizet %d\n",HEAP->Bin[0]->size_and_flag->prev_chunk_size);
            printf("cur_chunk_size %d\n",HEAP->Bin[0]->size_and_flag->cur_chunk_size);
            printf("start_brk %p\n",HEAP->start_brk);

        }
        //if heap is null,then init a heap
        // init_heap()
        // -> use sbrk() to allocate a 64 *1024 bytes heap
        // range is start_brk ~ start_brk + 0x00010000
        // then split the range to fit the size the caller ask
        // finally,return the addr to the caller
        // the remain chunk shoud put into the BIN
        // split (and merge)
        // bytes+header_size {the best fit size}

        //BIN
        //BIN->head and BIN->tail
        // manage free chunk
        // circular double-linked list
        // add to the rear
        // bin[0]~bin[10] with fixed size 2^5~2^15
        // !!!choose the lowest addr chunk

    }
    return (void*)100;
    // return valid virtual starting addr
    // return NULL
}

int hw_free(void *mem)
{
    return 0;
}

void *get_start_sbrk(void)
{
    return NULL;
}