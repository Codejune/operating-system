/**
 * @file common.h
 * @author 김병준 (kbj9704@gmail.com)
 * @brief 프로그램 구현에 필요한 헤더, 함수 및 매크로 정의
 */
#ifndef COMMON_H
#define COMMON_H

// header
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <wait.h>
#include <errno.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>

// Macro
#define SECOND_TO_MICRO 1000000
#define SLAVE_COUNT 21
#define NICE_LOW -20
#define NICE_MID 0
#define NICE_HIGH 19

// Function prototype
void product(uint64_t n);
void print_runtime(struct timeval *begin_t, struct timeval *end_t);

#endif