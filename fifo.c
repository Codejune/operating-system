/**
 * @file cfs.c
 * @author 김병준 (kbj9704@gmail.com)
 * @brief FIFO 페이징 기법 시뮬레이션을 위한 함수 구현
 */
#include "fifo.h"

extern uint8_t g_pg_frms;
extern uint8_t g_pg_ref[BUFFER_SIZE];
extern uint8_t g_pg_ref_cnt;

/**
 * @brief Main entry function
 * @return int OK 0, ERROR 1
 */
int main(void)
{
    init(); // Initialize header(Page frames, Page references)
    print_header(METHOD_FIFO);
    simulate(); // Simulate FIFO
    exit(EXIT_SUCCESS);
}

void simulate(void)
{
    uint8_t frms[MAX_FRAMES] = {0};

    
}