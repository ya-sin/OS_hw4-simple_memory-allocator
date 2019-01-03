#include "hw_malloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/mman.h>

bool has_init = false;
bool mmap_init = false;
void *start_brk = NULL;
bin_t s_bin[11] = {};
bin_t *bin[11];
bin_t s_mhead = {};
bin_t *mhead;
int slice_num = 1; // count the number of chunk
chunk_header *top[2];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*Static function*/
static chunk_header *create_chunk(chunk_header *base, const size_t need);
static chunk_header *split(chunk_header **ori, const size_t need);
static chunk_header *merge(chunk_header *h);
static int search_debin(const size_t need);
static int search_enbin(const size_t need);
static void en_bin(const int index, chunk_header *c_h);
static chunk_header *de_bin(const int index, const size_t need);
static void rm_chunk_from_bin(chunk_header *c);
static int check_valid_free(const void *mem);

void reorder(void)
{
    chunk_header *tmp;
    chunk_header *cur;
    chunk_header *qq;


    tmp = mhead->prev;
    cur = mhead->next;
    while(cur!=tmp) {

        if(cur->size_and_flag.cur_chunk_size < tmp->size_and_flag.cur_chunk_size) {
            cur = cur->next;
            // continue;
        } else {
            if(mhead->size==2) {
                mhead->next = tmp;
                tmp->prev = mhead;
                tmp->next = cur;
                cur->prev = tmp;
                // break;
            } else {
                qq = cur->prev;
                tmp->prev = cur->prev;
                qq->next = tmp;
                tmp->next = cur;
                cur->next = mhead;
                // break;
            }
            break;
        }
    }

}
void add2list(struct chunk_header* c_h,size_t need)
{
    c_h->size_and_flag.mmap_flag=1;
    c_h->size_and_flag.alloc_flag=1;
    c_h->size_and_flag.cur_chunk_size = need;
    c_h->size_and_flag.prev_chunk_size = 0;
    c_h->prev = NULL;
    c_h->next = NULL;
    if (mhead->size == 0) {
        mhead->next = c_h;
        c_h->prev = mhead;
        mhead->prev = c_h;
        c_h->next = mhead;
    } else {
        chunk_header *tmp;
        chunk_header *cur;

        tmp = mhead->prev;
        mhead->prev = c_h;
        c_h->next = mhead;
        tmp->next = c_h;
        c_h->prev = tmp;

    }
    mhead->size++;
}
void *hw_malloc(size_t bytes)
{
    if(bytes+24LL>MMAP_THRESHOLD) {
        if(!mmap_init) {
            mmap_init  = true;
            mhead = &s_mhead;
            mhead->prev = mhead;
            mhead->next = mhead;
            mhead->size = 0;
        }
        // printf("mmap %lld\n",bytes+24LL);
        size_t need = bytes%1024 != 0 ? (bytes/1024+1)*1024 : bytes;
        // printf("hey %d\n",need);
        void* addr = mmap(NULL,need,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,0,0);
        // printf("hey %d\n",need);

        add2list(addr, need);
        reorder();
        chunk_header * tmp = mhead->next;
        chunk_header * tmp2 = tmp->next;
        chunk_header * tmp3 = tmp2->next;
        // printf("yaya %d\n",tmp->size_and_flag.cur_chunk_size);
        // printf("yaya2 %d\n",tmp2->size_and_flag.cur_chunk_size);
        // printf("yaya3 %d\n",tmp3->size_and_flag.cur_chunk_size);
        // add2list(s);
        // printf("%d\n",addr);

        return (void *)((intptr_t)(void*)addr +
                        sizeof(chunk_header));
        // mmap()
        // chunk
        // bytes+header_size
        //mmap_alloc_list()
    } else {
        pthread_mutex_lock(&mutex);
        int bin_num=0;
        size_t tmp = bytes+24;
        while(tmp!=1) {
            tmp = tmp>>1;
            bin_num++;
            // printf("bin num %d\n",bin_num);
        }
        size_t need = bytes + 24LL + (bytes + 24LL == pow(2,bin_num) ? 0 : (pow(2,bin_num+1)-bytes - 24LL));
        // printf("bneed %lld\n",need);
        if (!has_init) {
            has_init = true;
            if (need > 64 * 1024) {
                PRINTERR("not enough space\n");
                has_init = false;
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
            for (int i = 0; i < 11; i++) {
                bin[i] = &s_bin[i];
                bin[i]->prev = bin[i];
                bin[i]->next = bin[i];
                bin[i]->size = 0;
            }
            start_brk = sbrk(HEAP_SIZE);
            top[0] = sbrk(24);
            top[1] = sbrk(24);
            top[0]->prev = NULL;
            top[0]->next = NULL;
            top[0]->size_and_flag.cur_chunk_size = 24;
            top[0]->size_and_flag.prev_chunk_size = HEAP_SIZE;
            top[0]->size_and_flag.mmap_flag = 0;
            top[0]->size_and_flag.alloc_flag = 0;
            // printf("1.chunk_size %d\n",top[0]->size_and_flag.prev_chunk_size);
            top[1]->size_and_flag.cur_chunk_size = 24;
            top[1]->size_and_flag.prev_chunk_size = 24;
            top[1]->size_and_flag.mmap_flag = 0;
            top[1]->size_and_flag.alloc_flag = 0;
            top[1]->prev = NULL;
            top[1]->next = NULL;

            chunk_header *s = create_chunk(get_start_brk(), HEAP_SIZE);
            chunk_header *c = split(&s, need);
            chunk_header test;
            // printf("!!!!chunk size%d %d %d %d\n",sizeof(test.prev),sizeof(test.next),sizeof(test.size_and_flag),sizeof(test));
            pthread_mutex_unlock(&mutex);
            return (void *)((intptr_t)(void*)c +
                            sizeof(chunk_header) -
                            (intptr_t)(void*)get_start_brk());
        } else {
            chunk_header *r = NULL;
            int bin_num = search_debin(need);
            // printf("bin_num%d\n",bin_num);
            if (bin_num == -1) {
                PRINTERR("search debin error\n");
            } else {
                // printf("need%lld\n",need);
                r = de_bin(bin_num, need);
                // printf("qqqq%d\n",r->size_and_flag.cur_chunk_size);
                pthread_mutex_unlock(&mutex);
                return (void *)((intptr_t)(void*)r +
                                sizeof(chunk_header) -
                                (intptr_t)(void*)get_start_brk());
            }
        }
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
}

int hw_free(void *mem)
{
    pthread_mutex_lock(&mutex);
    void *a_mem = (void *)((intptr_t)(void*)mem +
                           (intptr_t)(void*)get_start_brk());
    // printf("a_mem %d %d %d\n",a_mem,get_start_brk(),mem);
    if (!has_init || !check_valid_free(a_mem)) {
        pthread_mutex_unlock(&mutex);
        return 0;
    } else {
        chunk_header *h = (chunk_header *)((intptr_t)(void*)a_mem -
                                           (intptr_t)(void*)sizeof(chunk_header));
        chunk_header *nxt = (chunk_header *)((intptr_t)(void*)h +
                                             (intptr_t)(void*)((chunk_header *)h)->size_and_flag.cur_chunk_size);
        nxt->size_and_flag.alloc_flag = 1;
        chunk_header *m = merge(h);
        en_bin(search_enbin(m->size_and_flag.cur_chunk_size), m);
        pthread_mutex_unlock(&mutex);
        return 1;
    }
}

void *get_start_brk(void)
{
    return (void *)start_brk;
}
void show_mmap(void)
{
    if (!mmap_init) {
        return;
    }
    chunk_header *cur = mhead->next;
    // printf("???%d\n",cur->size_and_flag.cur_chunk_size);
    while ((void *)cur != (void *)mhead) {
        void *r_cur = (void *)((intptr_t)(void*)cur);
        printf("0x%012" PRIxPTR "--------%d\n", (uintptr_t)r_cur, cur->size_and_flag.cur_chunk_size);
        cur = cur->next;
    }
}
void show_bin(const int i)
{
    if (!has_init) {
        return;
    }
    // printf("bin size: %d\n", bin[i]->size);
    chunk_header *cur = bin[i]->next;
    while ((void *)cur != (void *)bin[i]) {
        void *r_cur = (void *)((intptr_t)(void*)cur -
                               (intptr_t)(void*)get_start_brk());
        printf("0x%012" PRIxPTR "--------%d\n", (uintptr_t)r_cur, cur->size_and_flag.cur_chunk_size);
        cur = cur->next;
    }
}

static chunk_header *create_chunk(chunk_header *base, const size_t need)
{
    if ((void *)base - get_start_brk() + need > 64 * 1024) {
        PRINTERR("heap not enough\n");
        return NULL;
    }
    chunk_header *ret = base;
    ret->size_and_flag.prev_chunk_size = 0;
    ret->size_and_flag.cur_chunk_size = need;
    ret->size_and_flag.alloc_flag = 0;
    ret->size_and_flag.mmap_flag = 0;
    ret->prev = NULL;
    ret->next = NULL;
    return ret;
}

static chunk_header *split(chunk_header **ori, const size_t need)
{
    // printf("chunk_size %d %lld\n",(*ori)->size_and_flag.cur_chunk_size,(*ori)->size_and_flag.cur_chunk_size);
    if ((*ori)->size_and_flag.cur_chunk_size/2 >= need ) {
        chunk_header *base = *ori;
        /*Change next chunk's prev_chunk_size*/
        chunk_header *nxt = (chunk_header *)((intptr_t)(void*)base +
                                             (intptr_t)(void*)((chunk_header *)base)->size_and_flag.cur_chunk_size);

        // printf("chunk_size %d\n",nxt->size_and_flag->prev_chunk_size);
        nxt->size_and_flag.prev_chunk_size -= nxt->size_and_flag.prev_chunk_size/2;
        // printf("chunk_size %lld\n",nxt->size_and_flag.prev_chunk_size);
        // nxt->size_and_flag->prev_chunk_size -= need;
        /*Create upper chunk by shifting need*/
        chunk_header *new = (void *)((intptr_t)(void*)base + nxt->size_and_flag.prev_chunk_size);
        new->size_and_flag.cur_chunk_size = nxt->size_and_flag.prev_chunk_size;
        // printf("newchunk_size %lld\n",new->size_and_flag.cur_chunk_size);
        new->size_and_flag.prev_chunk_size = nxt->size_and_flag.prev_chunk_size;
        new->size_and_flag.alloc_flag = 0;
        *ori = new;
        chunk_header *ret = create_chunk(base, nxt->size_and_flag.prev_chunk_size);
        /*Insert upper chunk into bin*/
        en_bin(search_enbin((*ori)->size_and_flag.cur_chunk_size), (*ori));
        slice_num++;
        if(ret->size_and_flag.cur_chunk_size/2>=need) {
            // printf("hi");
            split(&ret,need);
        } else {
            // printf("ret %lld\n",ret->size_and_flag.cur_chunk_size);
            return ret;
        }
    } else {
        printf("hello\n\n");
        /*If chunk size is not enough to split, return whole chunk*/
        chunk_header *nxt = (chunk_header *)((intptr_t)(void*)(*ori) +
                                             (intptr_t)(void*)((chunk_header *)(*ori))->size_and_flag.cur_chunk_size);
        nxt->size_and_flag.alloc_flag = 0;
        return (*ori);
    }
}

static chunk_header *merge(chunk_header *h)
{
    chunk_header *nxt = (chunk_header *)((intptr_t)(void*)h +
                                         (intptr_t)(void*)((chunk_header *)h)->size_and_flag.cur_chunk_size);
    chunk_header *nnxt = (chunk_header *)((intptr_t)(void*)nxt +
                                          (intptr_t)(void*)((chunk_header *)nxt)->size_and_flag.cur_chunk_size);
    if (nnxt->size_and_flag.alloc_flag == 1) {
        /*If next chunk is free, being able to merge*/
        nnxt->size_and_flag.prev_chunk_size += h->size_and_flag.cur_chunk_size;
        rm_chunk_from_bin(nxt);
        bin[search_enbin(nxt->size_and_flag.cur_chunk_size)]->size--;
        h->size_and_flag.cur_chunk_size += nxt->size_and_flag.cur_chunk_size;
        nxt->size_and_flag.cur_chunk_size = 0;
    }
    if (h->size_and_flag.alloc_flag == 1) {
        chunk_header *nxt = (chunk_header *)((intptr_t)(void*)h +
                                             (intptr_t)(void*)((chunk_header *)h)->size_and_flag.cur_chunk_size);
        chunk_header *pre = (chunk_header *)((intptr_t)(void*)h -
                                             (intptr_t)(void*)((chunk_header *)h)->size_and_flag.prev_chunk_size);
        nxt->size_and_flag.prev_chunk_size += pre->size_and_flag.cur_chunk_size;
        rm_chunk_from_bin(pre);
        bin[search_enbin(pre->size_and_flag.cur_chunk_size)]->size--;
        pre->size_and_flag.cur_chunk_size += h->size_and_flag.cur_chunk_size;
        h->size_and_flag.cur_chunk_size = 0;
        h->prev = NULL;
        h->next = NULL;
        return pre;
    } else {
        h->prev = NULL;
        h->next = NULL;
        return h;
    }
}

static int search_debin(const size_t need)
{
    for (int i = 0; i < 11; i++) {
        if (bin[i]->size == 0) {
            continue;
        }
        if (need <=  pow(2,(i + 5))) {
            return i;
        }
    }
    PRINTERR("not any free chunk\n");
    return -1;
}

static int search_enbin(const size_t size)
{
    int chunk_size=size,bin=0;
    while(chunk_size!=1) {
        chunk_size = chunk_size>>1;
        bin++;
        // printf("bin num %d %d\n",i,bin);
    }
    // printf("%lld %d",size,bin);
    return bin-5;
}

static void en_bin(const int index, chunk_header *c_h)
{
    if (bin[index]->size == 0) {
        bin[index]->next = c_h;
        c_h->prev = bin[index];
        bin[index]->prev = c_h;
        c_h->next = bin[index];
    } else {
        chunk_header *tmp;
        chunk_header *cur;

        tmp = bin[index]->prev;
        bin[index]->prev = c_h;
        c_h->next = bin[index];
        tmp->next = c_h;
        c_h->prev = tmp;
        // if (bin[6]->size > 0 &&
        //         c_h->size_and_flag.cur_chunk_size > ((chunk_header *)bin[6]->next)->size_and_flag.cur_chunk_size) {
        //     tmp = bin[index]->next;
        //     bin[index]->next = c_h;
        //     c_h->prev = bin[index];
        //     tmp->prev = c_h;
        //     c_h->next = tmp;
        // } else {
        //     cur = bin[6]->prev;
        //     while ((void *)cur != (void *)bin[6]) {
        //         if (c_h->size_and_flag.cur_chunk_size <= cur->size_and_flag.cur_chunk_size) {
        //             tmp = cur->next;
        //             cur->next = c_h;
        //             c_h->prev = cur;
        //             tmp->prev = c_h;
        //             c_h->next = tmp;
        //             break;
        //         }
        //         cur = cur->prev;
        //     }
        // }
        // break;
    }
    bin[index]->size++;
}

static chunk_header *de_bin(const int index, const size_t need)
{
    if (bin[index]->size == 0) {
        PRINTERR("size = 0\n");
        return NULL;
    } else {
        chunk_header *ret;
        // chunk_header *cur;
        chunk_header *c;
        chunk_header *s;
        ret = bin[index]->next;
        // printf("bin size %d\n",ret->chunk_size);
        if(ret->size_and_flag.cur_chunk_size/2>=need) {
            rm_chunk_from_bin(ret);
            s = create_chunk(ret,ret->size_and_flag.cur_chunk_size);
            c = split(&s,need);
            // printf("here\n");
            int chunk_size=c->size_and_flag.cur_chunk_size,bin_num=0;
            while(chunk_size!=1) {
                chunk_size = chunk_size>>1;
                bin_num++;
                // printf("bin num %d %d\n",i,bin);
            }
            bin[index]->size--;
            return c;
        } else {
            rm_chunk_from_bin(ret);
            bin[index]->size--;
            return ret;
        }
    }
}

static void rm_chunk_from_bin(chunk_header *c)
{
    /*Used to reconnect linked list when removing a chunk*/
    if (c->prev == bin[0] || c->prev == bin[1] ||
            c->prev == bin[2] || c->prev == bin[3] ||
            c->prev == bin[4] || c->prev == bin[5] ||
            c->prev == bin[6] || c->prev == bin[7] ||
            c->prev == bin[8] || c->prev == bin[9] ||
            c->prev == bin[10]
       ) {
        ((bin_t *)c->prev)->next = c->next;
    } else {
        ((chunk_header *)c->prev)->next = c->next;
    }
    if (c->next == bin[0] || c->next == bin[1] ||
            c->next == bin[2] || c->next == bin[3] ||
            c->next == bin[4] || c->next == bin[5] ||
            c->next == bin[6] || c->prev == bin[7] ||
            c->prev == bin[8] || c->prev == bin[9] ||
            c->prev == bin[10]
       ) {
        ((bin_t *)c->next)->prev = c->prev;
    } else {
        ((chunk_header *)c->next)->prev = c->prev;
    }
    c->prev = NULL;
    c->next = NULL;
}

void watch_heap()
{
    chunk_header *cur = get_start_brk();
    int count = 0;
    printf("slice: %d\n", slice_num);
    while (count++ < slice_num + 1) {
        printf("----------\n");
        printf("0x%08" PRIxPTR "(",
               (uintptr_t)(void *)((intptr_t)(void*)cur - (intptr_t)(void*)get_start_brk()));
        printf("0x%08" PRIxPTR ")\n",
               (uintptr_t)(void *)cur);
        printf("chun_size:%lld\n", cur->size_and_flag.cur_chunk_size);
        printf("prev_size:%lld\n", cur->size_and_flag.prev_chunk_size);
        printf("prev_free:%lld\n", cur->size_and_flag.alloc_flag);
        cur = (void *)((intptr_t)(void*)cur + (intptr_t)(void*)cur->size_and_flag.cur_chunk_size);
    }
}

static int check_valid_free(const void *a_mem)
{
    chunk_header *cur = get_start_brk();
    int count = 0;
    while (count++ < slice_num + 1) {
        if ((intptr_t)(void*)cur > (intptr_t)(void*)a_mem - 24) {
            return 0;
        }
        if (cur == a_mem - 24) {
            void *nxt;
            nxt = (void *)((intptr_t)(void*)cur +
                           (intptr_t)(void*)cur->size_and_flag.cur_chunk_size);
            if ((intptr_t)(void*)nxt - (intptr_t)(void*)get_start_brk() <= 65536 &&
                    ((chunk_header *)nxt)->size_and_flag.alloc_flag == 0) {
                return 1;
            } else {
                return 0;
            }
        }
        cur = (void *)((intptr_t)(void*)cur + (intptr_t)(void*)cur->size_and_flag.cur_chunk_size);
    }
    return 0;
}
