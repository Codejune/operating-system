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
    int16_t frms[MAX_FRAMES] = {-1};
    uint8_t last_idx = 0;
    uint8_t i, j;
    bool is_pg_fault = true;
    uint8_t pg_fault_cnt = 0;

    // Initialize frame array
    memset(frms, -1, sizeof(frms));

    for (i = 0; i < g_pg_ref_cnt; i++)
    {
        // Find page reference in page frames
        for (j = 0; j < g_pg_frms; j++)
            // If page reference is in page frames
            if (g_pg_ref[i] == frms[j])
            {
                is_pg_fault = false;
                break;
            }

        // If page reference is not in page frames
        if (is_pg_fault) {
            frms[last_idx++] = g_pg_ref[i];
            pg_fault_cnt++;
        }

        // If page frames is full
        if (last_idx == g_pg_frms)
            last_idx = 0;

        // Print sequence of page frames
        printf("%hhd\t\t", i + 1);
        for (j = 0; j < g_pg_frms; j++)
            if (frms[j] != -1)
                printf("%hhd\t", frms[j]);
            else
                printf("\t");
        if (is_pg_fault)
            printf("F");
        printf("\n");

        // Reset page fault status
        is_pg_fault = true;
    }
    printf("Number of page faults: %hhd times\n", pg_fault_cnt);
}