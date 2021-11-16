/**
 * @file fifo.h
 * @author 김병준 (kbj9704@gmail.com)
 * @brief FIFO 페이징 기법 구현에 필요한 헤더, 함수 및 매크로 정의
 */
#ifndef FIFO_H
#define FIFO_H

// header
#include "common.h"

// Macro

// Global Variable
extern uint8_t g_frms_cnt;
extern uint8_t g_pg_ref[BUFFER_SIZE];
extern uint8_t g_pg_ref_cnt;

// Function prototype
void simulate(void);

#endif