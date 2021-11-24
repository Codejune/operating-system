/**
 * @file fifo.c
 * @author 김병준 (kbj9704@gmail.com)
 * @brief FIFO 페이징 기법 시뮬레이션을 위한 함수 구현
 */
#include "fifo.h"

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

/**
 * @brief Simulte FIFO paging method
 */
void simulate(void)
{
    int16_t frms[MAX_FRAMES], last_idx = 0;
    bool is_pg_fault;
    uint8_t pg_fault_cnt = 0;
    uint8_t i, j;

    // Initialize frame array
    memset(frms, -1, sizeof(frms));

    for (i = 0; i < g_pg_ref_cnt; i++)
    {
        // Reset page fault status
        is_pg_fault = true;

        // Find page reference in page frames
        for (j = 0; j < g_frms_cnt; j++)
            // If page reference is in page frames
            if (g_pg_ref[i] == frms[j])
            {
                is_pg_fault = false;
                break;
            }

        // If page reference is not in page frames
        if (is_pg_fault)
        {
            pg_fault_cnt++;
            frms[last_idx++] = g_pg_ref[i];
            // If page frames is full
            if (last_idx == g_frms_cnt)
                last_idx = 0;
        }

        // Print sequence of page frames
        printf("%hhd\t\t", i + 1);
        for (j = 0; j < g_frms_cnt; j++)
            if (frms[j] != -1)
                printf("%hhd\t", frms[j]);
            else
                printf("\t");
        if (is_pg_fault)
            printf("F");
        printf("\n");
    }
    printf("Number of page faults: %hhd times\n", pg_fault_cnt);
}