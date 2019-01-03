#include "hw4_mm_test.h"

int main()
{
    char input[20];
    int size;
    void* addr;
    while(scanf("%s",input)!=EOF) {
        if(!strcmp(input,"alloc")) {
            scanf("%d",&size);
            size_t need = size;
            void *ptr = hw_malloc(need);
            if (ptr != NULL) {
                printf("0x%012" PRIxPTR "\n", (uintptr_t)ptr);
            } else {
                printf("%p\n", ptr);
            }
        } else if(!strcmp(input,"free")) {
            int tmp;
            scanf("%p",&addr);
            tmp = hw_free(addr);
            if(tmp)
                printf("success\n");
            else
                printf("fail\n");
        } else if(!strcmp(input,"print")) {
            int bin_num;
            char q[20];
            scanf("%s",q);
            bin_num = q[4]-'0';
            if(q[5]-'0'==0)
                bin_num = 10;
            // printf("%d\n",bin_num);
            // scanf(" bin[%d]",&bin_num);
            // printf("%d\n",bin_num);
            // printf("alloc %d\n",i);

            if(bin_num==47)
                show_mmap();
            else {
                if (bin_num < 0 || bin_num > 10) {
                    printf("COMMAND ERROR\n");
                    continue;
                }
                show_bin(bin_num);
            }
        } else {
            printf("Wrong command!");
        }

    }
    return 0;
}
