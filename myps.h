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
#include <signal.h>
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
typedef struct Process
{
    uint32_t pid;              // PID
    char user_name[16];        // USER
    int8_t priority;          // PR
    int8_t nice;              // NI
    uint64_t virtual_memory;   // VIRT
    uint64_t physical_memeory; // RES
    uint64_t shared_memory;    // SHR
    char status;               // S
    double cpu_utilization;     // %CPU
    double memory_utilization;  // %MEM
    char time[16];             // TIME+
    char command[30];          // COMMAND
} process;

// Glabal variable
// Function prototype
void parse_option(int argc, char *argv[]);
void print_help(void);
#endif