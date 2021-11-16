/**
 * @file common.c
 * @brief 프로젝트에서 공통적으로 사용되는 함수 정의
 * @author 김병준 (kbj9704@gmail.com)
 */
#include "common.h"

uint8_t g_frms_cnt = 0;               // Page frames
uint8_t g_pg_ref[BUFFER_SIZE] = {0}; // Page reference list
uint8_t g_pg_ref_cnt = 0;            // Page reference count

/**
 * @brief Initialization to global variables
 */
void init(void)
{
    FILE *fp;
    char buffer[BUFFER_SIZE];

    // Get input file path
    printf("Input file path > ");
    scanf("%s", buffer);

    // Open input file
    if ((fp = fopen(buffer, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", buffer);
        exit(EXIT_FAILURE);
    }

    // Read and set page frms
    fscanf(fp, "%hhd\n%[^\n]", &g_frms_cnt, buffer);

    // Set page references
    set_page_references(buffer);
}

/**
 * @brief Tokenize buffer and set page references
 * @param buffer Buffer to tokenize
 */
void set_page_references(char *buffer)
{
    char *token;

    token = strtok(buffer, " ");
    while (token != NULL)
    {
        g_pg_ref[g_pg_ref_cnt++] = atoi(token);
        token = strtok(NULL, " ");
    }
}

/**
 * @brief Print virtual memory paging method simulator result
 * @param method Paging method
 */
void print_header(uint8_t method)
{
    uint8_t i;
    printf("Used method : ");
    switch (method)
    {
    case METHOD_OPT:
        printf("OPT\n");
        break;
    case METHOD_FIFO:
        printf("FIFO\n");
        break;
    case METHOD_LRU:
        printf("LRU\n");
        break;
    case METHOD_SC:
        printf("SC\n");
        break;
    }
    printf("Page frames: %hhd\n", g_frms_cnt);
    printf("Page reference string: ");
    for (i = 0; i < g_pg_ref_cnt; i++)
        printf("%hhd ", g_pg_ref[i]);
    printf("\n\n\tframe");
    for (uint8_t i = 0; i < g_frms_cnt; i++)
        printf("\t%d", i + 1);
    printf("\tpage fault\ntime\n");
}