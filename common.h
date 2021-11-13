/**
 * @file common.h
 * @author 김병준 (kbj9704@gmail.com)
 * @brief 시뮬레이터 구현에 필요한 공통 헤더, 함수 및 매크로 정의
 */
#ifndef COMMON_H
#define COMMON_H

// header
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>

// Macro
#define BUFFER_SIZE 256
#define METHOD_OPT 1
#define METHOD_FIFO 2
#define METHOD_LRU 3
#define METHOD_SC 4
#define MAX_FRAMES 32

// Global Variable

// Function prototype
void init(void);
void set_page_references(char *buffer);
void print_header(uint8_t method);

#endif