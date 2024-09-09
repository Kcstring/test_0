#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <sys/types.h>  // 引入pid_t

#define MAX_CMD 512

int count = 0;

// 定义一个进程的结构体
typedef struct {
    pid_t pid;
    pid_t ppid;
    char proc_name[MAX_CMD];
} process;

// 处理获得的proc_name(去掉括号)
void remove_parentheses(char* str) {
    int len = strlen(str);

    if (len > 1 && str[0] == '(' && str[len - 1] == ')') {
        // 左移字符串内容，去掉首尾的括号
        memmove(str, str + 1, len - 2);
        str[len - 2] = '\0';  // 添加字符串结束符
    }
}

// 判断是否为进程相关的文件夹（数字）
int is_number(const char *str) {
    if (*str == '\0') return 0;
    while (*str) {
        if (!isdigit(*str++)) return 0;
    }
    return 1;
}

// 获得进程相关信息
void get_process(process *proce) {
    struct dirent *entry;
    DIR *dp = opendir("/proc");
    if (dp == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    FILE *fp;
    char path[MAX_CMD];
    int i = 0;
    while ((entry = readdir(dp))) {
        // 仅处理数字的文件（进程）
        if (is_number(entry->d_name)) {
            proce[i].pid = (pid_t)atoi(entry->d_name);
            snprintf(path, sizeof(path), "/proc/%s/stat", entry->d_name);
            fp = fopen(path, "r");
            if (fp) {
                fscanf(fp, "%*d %255s %*s %d", proce[i].proc_name, &proce[i].ppid);
                remove_parentheses(proce[i].proc_name);
                fclose(fp);
                ++i;
            } else {
                perror("fopen");
            }
        }
    }
    closedir(dp);  // 关闭目录流
    printf("%d\n", i-1);
    count = i-1;
}

// 比较函数，用于 qsort
int compare_pid(const void *a, const void *b) {
    return ((process *)a)->pid - ((process *)b)->pid;
}

// 打印进程树
void show_proc_tree(process proc[], int proc_num, pid_t parent_id, int level, int flag1, int flag2) {
    process children[proc_num];
    int child_count = 0;

    for (int i = 0; i < proc_num; ++i) {
        if (proc[i].ppid == parent_id) {
            children[child_count++] = proc[i];
        }
    }

    if (flag2 == 1)
        qsort(children, child_count, sizeof(process), compare_pid);

    for (int i = 0; i < child_count; ++i) {
        for (int j = 0; j < level; ++j) {
            printf("   ");  // 每层缩进
        }
        if (flag1 == 0)
            printf("├── %s \n", children[i].proc_name);
        if (flag1 == 1)
            printf("├── %s (PID: %d)\n", children[i].proc_name, children[i].pid);
        show_proc_tree(proc, proc_num, children[i].pid, level+1, flag1, flag2);
    }
}

int main(int argc, char *argv[]) {
    int flag1 = 1, flag2 = 0;
    process process_array[1024];
    get_process(process_array);
    if (argc <= 1) {
        flag1 = 0;
        show_proc_tree(process_array, count, 0, 0, flag1, flag2); 
    } else if (argc == 2) {
        if (strcmp(argv[1], "-p") == 0 || strcmp(argv[1], "--show-pids") == 0) {
            flag1 = 1;
            show_proc_tree(process_array, count, 0, 0, flag1, flag2); 
            return 0;
        }
        if (strcmp(argv[1], "-n") == 0 || strcmp(argv[1], "--numeric-sort") == 0) {
            flag2 = 1;
            show_proc_tree(process_array, count, 0, 0, flag1, flag2);
            return 0;
        }
        if (strcmp(argv[1], "-V") == 0 || strcmp(argv[1], "--version") == 0) {
            printf("Process Tree Printer Version: 1.0.0\n");
            printf("Author: Kstring\n");
            printf("Description: This program prints the process tree of the current system.\n");
            printf("Usage:\n");
            printf("  -p, --show-pids      Print the process ID for each process.\n");
            printf("  -n, --numeric-sort   Sort processes by PID in ascending order.\n");
            printf("  -V, --version        Show version information.\n");
            printf("  -h, --help           Show this help message.\n");
            return 0;
        }
        printf("illegal argv\n");
        return 1;
    } else {
        printf("Illegal arguments number\n");
        return 1;
    }
}
