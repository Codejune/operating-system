/**
 * @file myps.h
 * @author 김병준 (kbj9704@gmail.com)
 * @brief myps 명령어 구현에 필요한 헤더, 함수 및 매크로 정의
 */
#ifndef MYPS_H
#define MYPS_H

// Header
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>

// Macro
#define MIN_BUFFER_SIZE 64
#define BUFFER_SIZE 256
#define MAX_BUFFER_SIZE 1024
#define HERTZ sysconf(_SC_CLK_TCK)

// Struct
typedef struct Process
{
    char user_name[16];
    uint32_t pid;
    double cpu;
    double memory;
    uint32_t vsz;
    uint32_t rss;
    char tty[10];
    char stat[6];
    char start[10];
    char time[10];
    char command[BUFFER_SIZE];
} process;

// Glabal variable
bool g_option_a = false;
bool g_option_u = false;
bool g_option_x = false;
struct winsize WINDOW_SIZE;
uint64_t g_cpu_total_time = 0;
uint64_t g_memory_total_size = 0;
uint64_t g_process_count = 0;

// Function prototype
void parse_option(int argc, char *argv[]);
void print_help(void);
uint64_t get_cpu_total_time(void);
uint64_t get_memory_total_size(void);
process **get_processes(void);
uint16_t get_tty(uint32_t pid);
process *get_process_status(uint32_t pid);
void get_user_name(uint32_t uid, char *buffer);
void get_terminal(uint16_t tty, char *buffer);
void get_time_format(uint64_t time, char *buffer);
void get_start_time(uint64_t start_time, char *buffer);
time_t get_up_time(void);
void get_command(uint32_t pid, char *buffer);
void sort_by_pid(process **p);
void print_processes(process **p);
#endif