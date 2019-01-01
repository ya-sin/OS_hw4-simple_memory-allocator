#include "lib/hw_malloc.h"
#include "hw4_mm_test.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    char input[50];
    // char print_mode[20];
    void* addr;
    int bin_num;
    int size;
    int tmp;
    // long long int offset;

    while(scanf("%s",input)!=EOF) {
        if(!strcmp(input,"alloc")) {
            scanf("%d",&size);
            size_t tsize = size;
            printf("%d\n",tsize);
            void* tmp = hw_malloc(tsize);
            if(tmp != NULL)
                printf("0x%08llx\n",(long long int)tmp);
            else
                printf("some problem %p\n",tmp);
        } else if(!strcmp(input,"free")) {
            scanf("%p",&addr);
            // printf("%s\n",addr);
            printf("in free");
            tmp = hw_free(addr);
            if(tmp)
                printf("success");
            else
                printf("fail");
        } else if(!strcmp(input,"print")) {
            scanf(" bin[%d]",&bin_num);
            printf("%d\n",bin_num);
            // scanf("%s", &print_mode);
            // if(!strcmp(print_mode,"mmap_alloc_list"))
            //     printf("mmap");
            // else{

            // }
        } else {
            printf("Wrong command!");
        }

    }

    return 0;
}
