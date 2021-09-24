/**
 * @file mytop.h
 * @author 김병준 (kbj9704@gmail.com)
 * @brief 프로젝트 공통 헤더, 함수 및 매크로 정의
 */
#ifndef MYTOP_H
#define MYTOP_H

// Header
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <termios.h>
#include <ncurses.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>

// Macro
#define BUFFER_SIZE 256
#define MAX_BUFFER_SIZE 1024

// Struct
typedef struct Header
{
    char current_time[9]; // hh:mm:ss format
    float uptime;
    uint8_t user_count;
    float load_average[3];     // 1, 5, 15
    uint32_t task[5];          // total, running, sleeping, stopping, zombie
    uint64_t cpu[8];           // user, system, nice, idle, IO-wait, h/w interrupts, s/w interrupts, stolen
    double physical_memory[4]; // total, free, used, buff/cache
    double virtual_memory[4];  // total, free, used, avail Mem
} header;

typedef struct Process
{
    uint32_t pid;              // PID
    char user_name[16];        // USER
    int16_t priority;         // PR
    int16_t nice;             // NI
    uint64_t virtual_memory;   // VIRT
    uint64_t physical_memeory; // RES
    uint64_t shared_memory;    // SHR
    char status;               // S
    float cpu_utilization;     // %CPU
    float memory_utilization;  // %MEM
    char time[16];             // TIME+
    char command[30];          // COMMAND
} process;

// Global variable
uint16_t REFRESH_PERIOD = 3;
uint32_t TARGET_PID = 0;
struct winsize WINDOW_SIZE;
uint32_t VIEW_POSITION = 0;
uint64_t LAST_CPU_STATUS[8] = {0};
uint64_t **LAST_CPU_UPTIME;
uint32_t LAST_PROCESS_COUNT;
uint64_t TOTAL_CPU_TIME = 0;
uint64_t TOTAL_MEMORY_SIZE = 0;

// Function prototype
int parse_option(int argc, char *argv[]);
void print_help(void);
void alarm_handler(int signo);
void update(void);
header *get_header(void);
void get_current_time(header *h);
void get_uptime(header *h);
void get_user_count(header *h);
void get_load_average(header *h);
void get_tasks_status(header *h);
void get_cpu_status(header *h);
void get_memmory_status(header *h);
void print_header(header *h);
process *get_processes(header *h);
void get_user_name(int uid, char *buffer);
void get_time_format(uint64_t total_time, char *buffer);
void print_processes(header *h, process *p);
void sort_by_pid(process *p, uint32_t process_count);
void sort_by_cpu(process *p, uint32_t process_count);
#endif // MYTOP_H