#include "print.h"
#include "init.h"
#include "interrupt.h"
#include "process.h"
#include "console.h"
#include "syscall_init.h"
#include "syscall.h"
#include "stdio.h"
#include "memory.h"
#include "dir.h"
#include "fs.h"

void k_thread_function_a(void* args);
void k_thread_function_b(void* args);
void user_process_a(void* args);
void user_process_b(void* args);
int prog_a_pid = 0, prog_b_pid = 0;

int main(void) {
    // 这里不能使用console_put_str，因为还没有初始化
    put_str("kernel_init\n");
    init_all();
    /********  测试代码  ********/
    printf("/dir1 content before delete /dir1/subdir1:\n");
    struct dir* dir = sys_opendir("/dir1/");
    char* type = NULL;
    struct dir_entry* dir_e = NULL;
    while((dir_e = sys_readdir(dir))) { 
        if (dir_e->f_type == FT_REGULAR) {
        type = "regular";
        } else {
        type = "directory";
        }
        printf("      %s   %s\n", type, dir_e->filename);
    }
    printf("try to delete nonempty directory /dir1/subdir1\n");
    if (sys_rmdir("/dir1/subdir1") == -1) {
        printf("sys_rmdir: /dir1/subdir1 delete fail!\n");
    }

    printf("try to delete /dir1/subdir1/file2\n");
    if (sys_rmdir("/dir1/subdir1/file2") == -1) {
        printf("sys_rmdir: /dir1/subdir1/file2 delete fail!\n");
    } 
    if (sys_unlink("/dir1/subdir1/file2") == 0 ) {
        printf("sys_unlink: /dir1/subdir1/file2 delete done\n");
    }
    
    printf("try to delete directory /dir1/subdir1 again\n");
    if (sys_rmdir("/dir1/subdir1") == 0) {
        printf("/dir1/subdir1 delete done!\n");
    }

    printf("/dir1 content after delete /dir1/subdir1:\n");
    sys_rewinddir(dir);
    while((dir_e = sys_readdir(dir))) { 
        if (dir_e->f_type == FT_REGULAR) {
        type = "regular";
        } else {
        type = "directory";
        }
        printf("      %s   %s\n", type, dir_e->filename);
    }

    /********  测试代码  ********/
    while (1);
    return 0;
}

void k_thread_function_a(void* args) {
    void* addr1 = sys_malloc(256);
    void* addr2 = sys_malloc(255);
    void* addr3 = sys_malloc(254);

    console_put_str("thread_a malloc addr: 0x");
    console_put_int((int)addr1);
    console_put_char(',');
    console_put_int((int)addr2);
    console_put_char(',');
    console_put_int((int)addr3);
    console_put_char('\n');

    int cpu_delay = 100000;
    while(cpu_delay-- > 0);
    sys_free(addr1);
    sys_free(addr2);
    sys_free(addr3);
    while (1);
}

void k_thread_function_b(void* args) {
    void* addr1 = sys_malloc(256);
    void* addr2 = sys_malloc(255);
    void* addr3 = sys_malloc(254);

    console_put_str("thread_b malloc addr: 0x");
    console_put_int((int)addr1);
    console_put_char(',');
    console_put_int((int)addr2);
    console_put_char(',');
    console_put_int((int)addr3);
    console_put_char('\n');

    int cpu_delay = 100000;
    while(cpu_delay-- > 0);
    sys_free(addr1);
    sys_free(addr2);
    sys_free(addr3);
    while (1);
}

void user_process_a(void* args) {
    void* addr1 = malloc(256);
    void* addr2 = malloc(255);
    void* addr3 = malloc(254);
    printf("prog_a malloc addr: 0x%x,0x%x,0x%x\n", (int)addr1, (int)addr2, (int)addr3);

    int cpu_delay = 100000;
    while(cpu_delay-- > 0);
    free(addr1);
    free(addr2);
    free(addr3);
    while (1);
}

void user_process_b(void* args) {
    void* addr1 = malloc(256);
    void* addr2 = malloc(255);
    void* addr3 = malloc(254);
    printf("prog_b malloc addr: 0x%x,0x%x,0x%x\n", (int)addr1, (int)addr2, (int)addr3);

    int cpu_delay = 100000;
    while(cpu_delay-- > 0);
    free(addr1);
    free(addr2);
    free(addr3);
    while (1);
}
