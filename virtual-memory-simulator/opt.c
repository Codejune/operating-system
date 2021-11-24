/**
 * @file opt.c
 * @author 김병준 (kbj9704@gmail.com)
 * @brief OPT 페이징 기법 시뮬레이션을 위한 함수 구현
 */
#include "opt.h"

/**
 * @brief Main entry function
 * @return int OK 0, ERROR 1
 */
int main(void)
{
    init(); // Initialize header(Page frames, Page references)
    print_header(METHOD_OPT);
    simulate(); // Simulate OPT
    exit(EXIT_SUCCESS);
}

/**
 * @brief Simulte OPT paging method
 */
void simulate(void)
{
    int16_t frms[MAX_FRAMES], lgst_frm_idx, lgst_pg_idx;
    bool is_pg_fault, is_pg_full, is_pg_used;
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
                lgst_pg_idx = i; // Reference page index which will not be used for the most longest time

                for (j = 0; j < g_frms_cnt; j++)
                {
                    is_pg_used = false;

                    // Find future page reference
                    for (k = i + 1; k < g_pg_ref_cnt; k++)
                        if (g_pg_ref[k] == frms[j])
                        {
                            if (k > lgst_pg_idx)
                            {
                                lgst_frm_idx = j;
                                lgst_pg_idx = k;
                            }
                            is_pg_used = true;
                            break;
                        }

                    // If cannot find page which using future
                    if (!is_pg_used)
                    {
                        lgst_frm_idx = j;
                        break;
                    }
                }

                frms[lgst_frm_idx] = g_pg_ref[i];
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