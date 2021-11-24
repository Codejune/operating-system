/**
 * @file lru.c
 * @author 김병준 (kbj9704@gmail.com)
 * @brief LRU 페이징 기법 시뮬레이션을 위한 함수 구현
 */
#include "lru.h"

/**
 * @brief Main entry function
 * @return int OK 0, ERROR 1
 */
int main(void)
{
    init(); // Initialize header(Page frames, Page references)
    print_header(METHOD_LRU);
    simulate(); // Simulate FIFO
    exit(EXIT_SUCCESS);
}

/**
 * @brief Simulte LRU paging method
 */
void simulate(void)
{
    int16_t frms[MAX_FRAMES];
    uint8_t ru_queue[MAX_FRAMES] = {0}, ru_queue_end = -1, lrup;
    bool is_pg_fault, is_pg_full;
    uint8_t pg_fault_cnt = 0;
    uint8_t i, j, k;

    // Initialize frame array
    memset(frms, -1, sizeof(frms));

    for (i = 0; i < g_pg_ref_cnt; i++)
    {
        // Reset page fault status
        is_pg_fault = true;
        is_pg_full = true;

        // Find page reference in page frames
        for (j = 0; j < g_frms_cnt; j++)

            // If page reference is in page frames
            if (g_pg_ref[i] == frms[j])
            {
                // Update page fault status
                is_pg_fault = false;

                // Find last using frame in queue
                for (k = 0; k < ru_queue_end; k++)
                    if (ru_queue[k] == j)
                    {
                        lrup = k;
                        break;
                    }

                // Update queue
                for (k = lrup; k < ru_queue_end; k++)
                    ru_queue[k] = ru_queue[k + 1];
                ru_queue[ru_queue_end] = j;
                break;
            }

        // If page reference is not in page frames
        if (is_pg_fault)
        {
            // Find empty frame
            for (j = 0; j < g_frms_cnt; j++)

                // If empty frame is found
                if (frms[j] < 0)
                {
                    // Set page reference to empty frame
                    frms[j] = g_pg_ref[i];
                    ru_queue[ru_queue_end++] = j;
                    is_pg_full = false;
                    break;
                }

            // If all frames are full
            if (is_pg_full)
            {
                // Pop page reference from page queue
                lrup = ru_queue[0];

                // Update frame
                frms[lrup] = g_pg_ref[i];

                // Update queue
                for (j = 0; j < ru_queue_end; j++)
                    ru_queue[j] = ru_queue[j + 1];
                ru_queue[ru_queue_end] = lrup;
            }
            pg_fault_cnt++;
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