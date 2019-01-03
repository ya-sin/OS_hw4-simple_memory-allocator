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
            // void *mem = (void *)(uintptr_t)strtol(get_argv(input), NULL, 16);
            scanf("%p",&addr);
            printf("%s\n", hw_free(addr) == 1 ? "success" : "fail");
            // printf("%s\n",addr);
            // printf("in free");
            // tmp = hw_free(addr);
            // if(tmp)
            //     printf("success");
            // else
            //     printf("fail");
        } else if(!strcmp(input,"print")) {
            int bin_num;
            char q[20];
            // int i = input[10] - '0';
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

char *get_argv(const char *command)
{
    char delim[] = " ";
    char *s = (char *)strndup(command, 20);
    char *pos;
    if ((pos = strchr(s, '\n')) != NULL)
        * pos = '\0';
    // split
    char *token;
    int argc = 0;
    for (token = strsep(&s, delim); token != NULL; token = strsep(&s, delim)) {
        if (argc == 1) {
            return token;
        }
        argc++;
    }
    return "";
}
