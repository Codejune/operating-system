/**
 * @file sc.c
 * @author 김병준 (kbj9704@gmail.com)
 * @brief SC 페이징 기법 시뮬레이션을 위한 함수 구현
 */
#include "sc.h"

/**
 * @brief Main entry function
 * @return int OK 0, ERROR 1
 */
int main(void)
{
    init(); // Initialize header(Page frames, Page references)
    print_header(METHOD_SC);
    simulate(); // Simulate FIFO
    exit(EXIT_SUCCESS);
}

/**
 * @brief Simulte Second Chance paging method
 */
void simulate(void)
{
    int16_t frms[MAX_FRAMES], last_idx = 0;
    bool rf_frms[MAX_FRAMES];
    bool is_pg_fault, is_pg_full, is_updated;
    uint8_t pg_fault_cnt = 0;
    uint8_t i, j;

    // Initialize frame array
    memset(frms, -1, sizeof(frms));
    memset(rf_frms, false, sizeof(rf_frms));

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
                // Update status
                is_pg_fault = false;
                rf_frms[j] = true;
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
                    is_pg_full = false;
                    break;
                }

            // If all frames are full
            if (is_pg_full)
            {
                is_updated = false;

                while (!is_updated)
                {
                    for (j = last_idx; j < g_frms_cnt; j++)
                    {
                        if (rf_frms[j])
                            rf_frms[j] = false;
                        else
                        {
                            last_idx++;
                            frms[j] = g_pg_ref[i];
                            rf_frms[j] = true;
                            is_updated = true;
                            break;
                        }
                    }

                    if (last_idx == g_frms_cnt)
                        last_idx = 0;
                }
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