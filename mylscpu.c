/**
 * @file mylscpu.c
 * @author 김병준 (kbj9704@gmail.com)
 * @brief mylscpu 명령어 기능 함수 구현
 */
#include "mylscpu.h"

/**
 * @brief Main entry function 
 * @param argc Argument count
 * @param argv Argument verse
 * @return int Ok 0, Error 1
 */
int main(int argc, char *argv[])
{
    if (ioctl(0, TIOCGWINSZ, (char *)&WINDOW_SIZE) < 0)
    {
        fprintf(stderr, "ioctl error\n");
        exit(EXIT_FAILURE);
    }

    get_cpu_info();
    get_cpu_online();
    get_cpu_clock();
    get_vulnerability();
    get_lcache_info();
    get_numa_node();
    get_architecture();
    get_byte_order();
    print_lscpu();
    exit(EXIT_SUCCESS);
}

/**
 * @brief Get CPU information from /proc/cpuinfo
 */
void get_cpu_info(void)
{
    char buffer[BUFFER_SIZE] = {0};
    char **flag_tokens;
    FILE *fp;
    uint8_t sibling_count, socket_count;
    uint16_t i;
    bool is_64bit = false;

    if ((fp = fopen(CPU_INFO_PATH, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for /proc/stat\n");
        exit(EXIT_FAILURE);
    }

    g_cpuinfo.socket_count = 0;
    while (true)
    {
        if (fscanf(fp, "%[^:]", buffer) == EOF)
            break;
        trim_string(buffer);

        if (strstr(buffer, "vendor_id") != NULL)
            fscanf(fp, ": %s", g_cpuinfo.vendor_id);
        else if (strstr(buffer, "cpu family") != NULL)
            fscanf(fp, ": %hhd", &g_cpuinfo.cpu_family);
        else if (strstr(buffer, "model") != NULL)
        {
            // Model name
            if (!strncmp(buffer, "model name", 10))
                fscanf(fp, ": %[^\n]", g_cpuinfo.model_name);
            else
                fscanf(fp, ": %hhd", &g_cpuinfo.model_number);
        }
        else if (strstr(buffer, "stepping") != NULL)
            fscanf(fp, ": %hhd", &g_cpuinfo.stepping);
        else if (strstr(buffer, "cpu MHz") != NULL)
            fscanf(fp, ": %f", &g_cpuinfo.cpu_clock);
        else if (strstr(buffer, "bogomips") != NULL)
            fscanf(fp, ": %f", &g_cpuinfo.bogo_mips);
        else if (strstr(buffer, "flags") != NULL)
            fscanf(fp, ": %[^\n]", g_cpuinfo.flags);
        // Core(s) per socket
        else if (strstr(buffer, "cpu cores") != NULL)
            fscanf(fp, ": %hhd\n", &g_cpuinfo.core_per_socket);
        else if (strstr(buffer, "siblings") != NULL)
            fscanf(fp, ": %hhd\n", &sibling_count);
        else if (strstr(buffer, "physical id") != NULL)
        {
            fscanf(fp, ": %hhd\n", &socket_count);
            if (g_cpuinfo.socket_count < socket_count)
                g_cpuinfo.socket_count = socket_count;
        }
        else if (strstr(buffer, "address sizes") != NULL)
            fscanf(fp, ": %[^\n]", g_cpuinfo.address_size);
        else
        {
            fscanf(fp, "%[^\n]", buffer);
            fgetc(fp);
        }
    }
    // Socket(s)
    g_cpuinfo.socket_count++;

    /**
     * lm : Long Mode (64 bit)
     * tm : Transparent Mode ( 32 bit)
     * rm : Real Mode (16 bit)
     */
    sprintf(g_cpuinfo.virtualization, "full");
    flag_tokens = get_tokenization(g_cpuinfo.flags, ' ');
    sprintf(g_cpuinfo.flags, "%s", get_serialization(flag_tokens, g_cpuinfo.flag_count));

    for (i = 0; i < g_cpuinfo.flag_count; i++)
    {
        // Intel virtualization
        if (!strcmp(flag_tokens[i], "vmx"))
            sprintf(g_cpuinfo.virtualization, "VT-x");
        // AMD virtualization
        else if (!strcmp(flag_tokens[i], "vmx"))
            sprintf(g_cpuinfo.virtualization, "AMD-V");
        // 64 Bit
        else if (!strcmp(flag_tokens[i], "lm"))
            is_64bit = true;
    }

    for (i = 0; i < g_cpuinfo.flag_count; i++)
        free(flag_tokens[i]);
    free(flag_tokens);

    // CPU op-mode(s)
    sprintf(g_cpuinfo.op_mode, "32-bit, ");
    if (is_64bit)
        strcat(g_cpuinfo.op_mode, "64-bit");

    // Thread(s) per core
    g_cpuinfo.thread_per_core = g_cpuinfo.core_per_socket / sibling_count;

    // CPU(s)
    g_cpuinfo.cpu_count = get_nprocs_conf();
}

/**
 * @brief Remove trash value and duplicate space from string
 * @param buffer Buffer
 */
void trim_string(char *buffer)
{
    char tmp[BUFFER_SIZE] = {0};
    uint16_t i, j = 0;
    bool is_space = false;

    for (i = 0; i < strlen(buffer); i++)
    {
        if ((97 <= buffer[i] && buffer[i] <= 122) || (65 <= buffer[i] && buffer[i] <= 90) || buffer[i] == 95)
            tmp[j++] = buffer[i];
        else if (buffer[i] == 32)
        {
            if (!is_space)
            {
                is_space = true;
                tmp[j++] = buffer[i];
            }
            else
            {
                tmp[j - 1] = '\0';
                sprintf(buffer, "%s", tmp);
                return;
            }
        }
        else
            continue;
    }
    sprintf(buffer, "%s", tmp);
}

/**
 * @brief Get token list divide by divider from string
 * 
 * @param buffer Buffer
 * @param divider Divider character
 * @return char** Token list
 */
char **get_tokenization(char *buffer, char divider)
{
    uint16_t i, count = 0;
    char tmp[MAX_BUFFER_SIZE] = {0}, *p;
    char **tokens;

    strcpy(tmp, buffer);
    p = strtok(tmp, &divider);
    while (p != NULL)
    {
        ++count;
        p = strtok(NULL, &divider);
    }

    tokens = (char **)malloc(count * sizeof(char *));
    for (i = 0; i < count; i++)
        tokens[i] = (char *)malloc(MAX_BUFFER_SIZE * sizeof(char));

    strcpy(tmp, buffer);
    i = 0;
    p = strtok(tmp, &divider);
    while (p != NULL)
    {
        sprintf(tokens[i++], "%s", p);
        p = strtok(NULL, &divider);
    }

    g_cpuinfo.flag_count = count;

    return tokens;
}

char *get_serialization(char **tokens, uint32_t token_count)
{
    uint16_t i;
    char *buffer = (char *)calloc(MAX_BUFFER_SIZE, sizeof(char));

    strcpy(buffer, tokens[0]);
    for (i = 1; i < token_count; i++)
    {
        strcat(buffer, " ");
        strcat(buffer, tokens[i]);
    }

    return buffer;
}

/**
 * @brief Get information of CPU online
 */
void get_cpu_online(void)
{
    FILE *fp;
    char *path = CPU_ONLINE_PATH;

    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%s", g_cpuinfo.online_cpu);
    fclose(fp);
}

/**
 * @brief Get CPU minimum/maximum clock
 */
void get_cpu_clock(void)
{
    FILE *fp;
    char *path = CPU_ONLINE_PATH;

    path = CPU_MIN_FREQUENCY_PATH;
    if (access(path, F_OK) == 0)
    {
        if ((fp = fopen(path, "r")) == NULL)
        {
            fprintf(stderr, "fopen error for %s\n", path);
            exit(EXIT_FAILURE);
        }
        fscanf(fp, "%f", &g_cpuinfo.cpu_clock_min);
        fclose(fp);
    }

    path = CPU_MAX_FREQUENCY_PATH;
    if (access(path, F_OK) == 0)
    {
        if ((fp = fopen(path, "r")) == NULL)
        {
            fprintf(stderr, "fopen error for %s\n", path);
            exit(EXIT_FAILURE);
        }
        fscanf(fp, "%f", &g_cpuinfo.cpu_clock_max);
        fclose(fp);
    }
}

/**
 * @brief Get vulnerability information from /sys/devices/system/cpu/vulnerabilities
 */
void get_vulnerability(void)
{
    FILE *fp;
    char *path;

    // Vulnerability Itlb multihit
    path = VULNERABILITY_ITLB_MULTIHIT_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%[^\n]", g_cpuinfo.vulnerability_itlb_multihit);
    fclose(fp);

    // Vulnerability L1tf
    path = VULNERABILITY_L1TF_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%[^\n]", g_cpuinfo.vulnerability_l1tf);
    fclose(fp);

    // Vulnerability Mds
    path = VULNERABILITY_MDS_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%[^\n]", g_cpuinfo.vulnerability_mds);
    fclose(fp);

    // Vulnerability Meltdown
    path = VULNERABILITY_MELTDOWN_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%[^\n]", g_cpuinfo.vulnerability_meltdown);
    fclose(fp);

    // Vulnerability Spec store bypass
    path = VULNERABILITY_SEPC_STORE_BYPASS_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%[^\n]", g_cpuinfo.vulnerability_spec_store_bypass);
    fclose(fp);

    // Vulnerability Spectre v1
    path = VULNERABILITY_SPECTRE_V1_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%[^\n]", g_cpuinfo.vulnerability_spectre_v1);
    fclose(fp);

    // Vulnerability Spectre v2
    path = VULNERABILITY_SPECTRE_V2_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%[^\n]", g_cpuinfo.vulnerability_spectre_v2);
    fclose(fp);

    // Vulnerability Srbds
    path = VULNERABILITY_SRBDS_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%[^\n]", g_cpuinfo.vulnerability_srbds);
    fclose(fp);

    // Vulnerability Tsx async abort
    path = VULNERABILITY_TSX_ASYNC_ABORT_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%[^\n]", g_cpuinfo.vulnerability_tsx_async_abort);
    fclose(fp);
}

/**
 * @brief Get each level of cache information
 */
void get_lcache_info(void)
{
    FILE *fp;
    char *path;

    // L1d cache
    g_cpuinfo.l1d_cache.one_size = sysconf(_SC_LEVEL1_DCACHE_SIZE);
    g_cpuinfo.l1d_cache.all_size = g_cpuinfo.l1d_cache.one_size * g_cpuinfo.cpu_count;
    path = L1D_CACHE_WAY_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%hhd", &g_cpuinfo.l1d_cache.ways);
    fclose(fp);
    path = L1D_CACHE_TYPE_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%s", g_cpuinfo.l1d_cache.type);
    fclose(fp);
    path = L1D_CACHE_LEVEL_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%hhd", &g_cpuinfo.l1d_cache.level);
    fclose(fp);

    // L1i cache
    g_cpuinfo.l1i_cache.one_size = sysconf(_SC_LEVEL1_ICACHE_SIZE);
    g_cpuinfo.l1i_cache.all_size = g_cpuinfo.l1i_cache.one_size * g_cpuinfo.cpu_count;
    path = L1I_CACHE_WAY_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%hhd", &g_cpuinfo.l1i_cache.ways);
    fclose(fp);
    path = L1I_CACHE_TYPE_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%s", g_cpuinfo.l1i_cache.type);
    fclose(fp);
    path = L1I_CACHE_LEVEL_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%hhd", &g_cpuinfo.l1i_cache.level);
    fclose(fp);

    // L2 cache
    g_cpuinfo.l2_cache.one_size = sysconf(_SC_LEVEL2_CACHE_SIZE);
    g_cpuinfo.l2_cache.all_size = g_cpuinfo.l2_cache.one_size * g_cpuinfo.cpu_count;
    path = L2_CACHE_WAY_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%hhd", &g_cpuinfo.l2_cache.ways);
    fclose(fp);
    path = L2_CACHE_TYPE_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%s", g_cpuinfo.l2_cache.type);
    fclose(fp);
    path = L2_CACHE_LEVEL_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%hhd", &g_cpuinfo.l2_cache.level);
    fclose(fp);

    // L3 cache
    g_cpuinfo.l3_cache.one_size = sysconf(_SC_LEVEL3_CACHE_SIZE);
    g_cpuinfo.l3_cache.all_size = g_cpuinfo.l3_cache.one_size * g_cpuinfo.socket_count;
    path = L3_CACHE_WAY_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%hhd", &g_cpuinfo.l3_cache.ways);
    fclose(fp);
    path = L3_CACHE_TYPE_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%s", g_cpuinfo.l3_cache.type);
    fclose(fp);
    path = L3_CACHE_LEVEL_PATH;
    if ((fp = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", path);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%hhd", &g_cpuinfo.l3_cache.level);
    fclose(fp);
}

/**
 * @brief Get the numa node information
 */
void get_numa_node(void)
{
    FILE *fp;
    char *path;
    struct dirent **namelist;
    uint32_t i, file_count;
    char buffer[MAX_BUFFER_SIZE] = {0};

    // NUMA node(s)
    path = NUMA_NODE_PATH;
    g_cpuinfo.numa_node_count = 0;
    file_count = scandir(path, &namelist, NULL, alphasort);
    for (i = 0; i < file_count; i++)
    {
        if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, ".."))
            continue;
        else if (!strncmp(namelist[i]->d_name, "node", 4))
            g_cpuinfo.numa_node_count++;
    }

    // NUMA node0 CPU(s)
    g_cpuinfo.numa_node = (numa *)malloc(g_cpuinfo.numa_node_count * sizeof(numa));
    for (i = 0; i < g_cpuinfo.numa_node_count; i++)
    {
        sprintf(buffer, "%s/node%d/cpulist", path, i);
        if ((fp = fopen(buffer, "r")) == NULL)
        {
            fprintf(stderr, "fopen error for %s\n", path);
            exit(EXIT_FAILURE);
        }
        fscanf(fp, "%s", g_cpuinfo.numa_node[i].cpulist);
        fclose(fp);
    }

    for (i = 0; i < file_count; i++)
        free(namelist[i]);
    free(namelist);
}

/**
 * @brief Get system architecture information
 */
void get_architecture(void)
{
    struct utsname buffer;
    if (uname(&buffer) < 0)
    {
        perror("uname");
        exit(EXIT_FAILURE);
    }
    sprintf(g_cpuinfo.architecture, "%s", buffer.machine);
}

/**
 * @brief Get cpu byte order information
 */
void get_byte_order(void)
{
    volatile uint32_t i = 0x01234567;
    if ((*((uint8_t *)(&i))) == 0x67)
        sprintf(g_cpuinfo.byte_order, "Little Endian");
    else
        sprintf(g_cpuinfo.byte_order, "Big Endian");
}

void print_lscpu(void)
{
    uint32_t i;
    char buffer[BUFFER_SIZE] = {0};

    // Architecture
    if (strlen(g_cpuinfo.architecture))
    {
        printf("%-33s", "Architecture:");
        printf("%s\n", g_cpuinfo.architecture);
    }
    // CPU op-mode(s)
    if (strlen(g_cpuinfo.op_mode))
    {
        printf("%-33s", "CPU op-mode(s):");
        printf("%s\n", g_cpuinfo.op_mode);
    }
    // Byte Order
    if (strlen(g_cpuinfo.byte_order))
    {
        printf("%-33s", "Byte Order:");
        printf("%s\n", g_cpuinfo.byte_order);
    }
    // Address sizes
    if (strlen(g_cpuinfo.address_size))
    {
        printf("%-33s", "Address sizes:");
        printf("%s\n", g_cpuinfo.address_size);
    }
    // CPU(s)
    printf("%-33s", "CPU(s):");
    printf("%d\n", g_cpuinfo.cpu_count);
    // On-line CPU(s) list:
    if (strlen(g_cpuinfo.online_cpu))
    {
        printf("%-33s", "On-line CPU(s) list:");
        printf("%s\n", g_cpuinfo.online_cpu);
    }
    // Thread(s) per core
    printf("%-33s", "Thread(s) per core:");
    printf("%d\n", g_cpuinfo.thread_per_core);
    // Core(s) per socket
    printf("%-33s", "Core(s) per socket:");
    printf("%d\n", g_cpuinfo.core_per_socket);
    // Socket(s)
    printf("%-33s", "Socket(s):");
    printf("%d\n", g_cpuinfo.socket_count);
    // NUMA node(s)
    if (g_cpuinfo.numa_node_count)
    {
        printf("%-33s", "NUMA node(s):");
        printf("%d\n", g_cpuinfo.numa_node_count);
    }
    // Vendor ID
    if (strlen(g_cpuinfo.vendor_id))
    {
        printf("%-33s", "Vendor ID:");
        printf("%s\n", g_cpuinfo.vendor_id);
    }
    // CPU family
    printf("%-33s", "CPU family:");
    printf("%d\n", g_cpuinfo.cpu_family);
    // Model
    if (g_cpuinfo.model_number)
    {
        printf("%-33s", "Model:");
        printf("%d\n", g_cpuinfo.model_number);
    }
    // Model name
    if (strlen(g_cpuinfo.model_name))
    {
        printf("%-33s", "Model name:");
        printf("%s\n", g_cpuinfo.model_name);
    }
    // Stepping
    printf("%-33s", "Stepping:");
    printf("%d\n", g_cpuinfo.stepping);
    // CPU MHz
    if (g_cpuinfo.cpu_clock)
    {
        printf("%-33s", "CPU MHz:");
        printf("%.3f\n", g_cpuinfo.cpu_clock);
    }
    // CPU max MHz
    if (g_cpuinfo.cpu_clock_max)
    {
        printf("%-33s", "CPU max MHz:");
        printf("%.3f\n", g_cpuinfo.cpu_clock_max);
    }
    // CPU min MHz
    if (g_cpuinfo.cpu_clock_min)
    {
        printf("%-33s", "CPU min MHz:");
        printf("%.3f\n", g_cpuinfo.cpu_clock_min);
    }
    // BogoMIPS
    if (g_cpuinfo.bogo_mips)
    {
        printf("%-33s", "BogoMIPS:");
        printf("%.2f\n", g_cpuinfo.bogo_mips);
    }
    // Virtualization
    if (strlen(g_cpuinfo.virtualization))
    {
        printf("%-33s", "Virtualization:");
        printf("%s\n", g_cpuinfo.virtualization);
    }
    // L1d cache
    if (g_cpuinfo.l1d_cache.all_size)
    {
        printf("%-33s", "L1d cache:");
        printf("%s\n", byte_to_human(g_cpuinfo.l1d_cache.all_size));
    }
    // L1i cache
    if (g_cpuinfo.l1i_cache.all_size)
    {
        printf("%-33s", "L1i cache:");
        printf("%s\n", byte_to_human(g_cpuinfo.l1i_cache.all_size));
    }
    // L2 cache
    if (g_cpuinfo.l2_cache.all_size)
    {
        printf("%-33s", "L2 cache:");
        printf("%s\n", byte_to_human(g_cpuinfo.l2_cache.all_size));
    }
    // L3 cache
    if (g_cpuinfo.l3_cache.all_size)
    {
        printf("%-33s", "L3 cache:");
        printf("%s\n", byte_to_human(g_cpuinfo.l3_cache.all_size));
    }
    // NUMA node
    for (i = 0; i < g_cpuinfo.numa_node_count; i++)
    {
        sprintf(buffer, "NUMA node%d CPU(s):", i);
        printf("%-33s", buffer);
        printf("%s\n", g_cpuinfo.numa_node[i].cpulist);
    }
    // Vulnerability Itlb multihit
    if (strlen(g_cpuinfo.vulnerability_itlb_multihit))
    {
        printf("%-33s", "Vulnerability Itlb multihit:");
        if (strlen(g_cpuinfo.vulnerability_itlb_multihit) > (size_t)WINDOW_SIZE.ws_col - 33)
        {
            i = 0;
            while (true)
            {
                printf("%.*s\n", WINDOW_SIZE.ws_col - 33, g_cpuinfo.vulnerability_itlb_multihit + i);
                i += WINDOW_SIZE.ws_col - 33;
                if (g_cpuinfo.vulnerability_itlb_multihit[i] != '\0')
                    printf("%33s", " ");
                else
                {
                    printf("\n");
                    break;
                }
            }
        }
        else
            printf("%s\n", g_cpuinfo.vulnerability_itlb_multihit);
    }
    // Vulnerability L1tf
    if (strlen(g_cpuinfo.vulnerability_l1tf))
    {
        printf("%-33s", "Vulnerability L1tf:");
        if (strlen(g_cpuinfo.vulnerability_l1tf) > (size_t)WINDOW_SIZE.ws_col - 33)
        {
            i = 0;
            while (true)
            {
                printf("%.*s\n", WINDOW_SIZE.ws_col - 33, g_cpuinfo.vulnerability_l1tf + i);
                i += WINDOW_SIZE.ws_col - 33;
                if (g_cpuinfo.vulnerability_l1tf[i] != '\0')
                    printf("%33s", " ");
                else
                {
                    printf("\n");
                    break;
                }
            }
        }
        else
            printf("%s\n", g_cpuinfo.vulnerability_l1tf);
    }
    // Vulnerability Mds
    if (strlen(g_cpuinfo.vulnerability_mds))
    {
        printf("%-33s", "Vulnerability Mds:");
        if (strlen(g_cpuinfo.vulnerability_mds) > (size_t)WINDOW_SIZE.ws_col - 33)
        {
           i = 0;
            while (true)
            {
                printf("%.*s\n", WINDOW_SIZE.ws_col - 33, g_cpuinfo.vulnerability_mds + i);
                i += WINDOW_SIZE.ws_col - 33;
                if (g_cpuinfo.vulnerability_mds[i] != '\0')
                    printf("%33s", " ");
                else
                {
                    printf("\n");
                    break;
                }
            }
        }
        else
            printf("%s\n", g_cpuinfo.vulnerability_mds);
    }
    // Vulnerability Meltdown
    if (strlen(g_cpuinfo.vulnerability_meltdown))
    {
        printf("%-33s", "Vulnerability Meltdown:");
        if (strlen(g_cpuinfo.vulnerability_meltdown) > (size_t)WINDOW_SIZE.ws_col - 33)
        {
            i = 0;
            while (true)
            {
                printf("%.*s\n", WINDOW_SIZE.ws_col - 33, g_cpuinfo.vulnerability_meltdown + i);
                i += WINDOW_SIZE.ws_col - 33;
                if (g_cpuinfo.vulnerability_meltdown[i] != '\0')
                    printf("%33s", " ");
                else
                {
                    printf("\n");
                    break;
                }
            }
        }
        else
            printf("%s\n", g_cpuinfo.vulnerability_meltdown);
    }
    // Vulnerability Spec store bypass
    if (strlen(g_cpuinfo.vulnerability_spec_store_bypass))
    {
        printf("%-33s", "Vulnerability Spec store bypass:");
        if (strlen(g_cpuinfo.vulnerability_spec_store_bypass) > (size_t)WINDOW_SIZE.ws_col - 33)
        {
            i = 0;
            while (true)
            {
                printf("%.*s\n", WINDOW_SIZE.ws_col - 33, g_cpuinfo.vulnerability_spec_store_bypass + i);
                i += WINDOW_SIZE.ws_col - 33;
                if (g_cpuinfo.vulnerability_spec_store_bypass[i] != '\0')
                    printf("%33s", " ");
                else
                {
                    printf("\n");
                    break;
                }
            }
        }
        else
            printf("%s\n", g_cpuinfo.vulnerability_spec_store_bypass);
    }
    // Vulnerability Spectre v1
    if (strlen(g_cpuinfo.vulnerability_spectre_v1))
    {
        printf("%-33s", "Vulnerability Spectre v1:");
        if (strlen(g_cpuinfo.vulnerability_spectre_v1) > (size_t)WINDOW_SIZE.ws_col - 33)
        {
            i = 0;
            while (true)
            {
                printf("%.*s\n", WINDOW_SIZE.ws_col - 33, g_cpuinfo.vulnerability_spectre_v1 + i);
                i += WINDOW_SIZE.ws_col - 33;
                if (g_cpuinfo.vulnerability_spectre_v1[i] != '\0')
                    printf("%33s", " ");
                else
                {
                    printf("\n");
                    break;
                }
            }
        }
        else
            printf("%s\n", g_cpuinfo.vulnerability_spectre_v1);
    }
    // Vulnerability Spectre v2
    if (strlen(g_cpuinfo.vulnerability_spectre_v2))
    {
        printf("%-33s", "Vulnerability Spectre v2:");
        if (strlen(g_cpuinfo.vulnerability_spectre_v2) > (size_t)WINDOW_SIZE.ws_col - 33)
        {
            i = 0;
            while (true)
            {
                printf("%.*s\n", WINDOW_SIZE.ws_col - 33, g_cpuinfo.vulnerability_spectre_v2 + i);
                i += WINDOW_SIZE.ws_col - 33;
                if (g_cpuinfo.vulnerability_spectre_v2[i] != '\0')
                    printf("%33s", " ");
                else
                {
                    printf("\n");
                    break;
                }
            }
        }
        else
            printf("%s\n", g_cpuinfo.vulnerability_spectre_v2);
    }
    // Vulnerability Srbds
    if (strlen(g_cpuinfo.vulnerability_srbds))
    {
        printf("%-33s", "Vulnerability Srbds:");
        if (strlen(g_cpuinfo.vulnerability_srbds) > (size_t)WINDOW_SIZE.ws_col - 33)
        {
            i = 0;
            while (true)
            {
                printf("%.*s\n", WINDOW_SIZE.ws_col - 33, g_cpuinfo.vulnerability_srbds + i);
                i += WINDOW_SIZE.ws_col - 33;
                if (g_cpuinfo.vulnerability_srbds[i] != '\0')
                    printf("%33s", " ");
                else
                {
                    printf("\n");
                    break;
                }
            }
        }
        else
            printf("%s\n", g_cpuinfo.vulnerability_srbds);
    }
    // Vulnerability Tsx async abort
    if (strlen(g_cpuinfo.vulnerability_tsx_async_abort))
    {
        printf("%-33s", "Vulnerability Tsx async abort:");
        if (strlen(g_cpuinfo.vulnerability_tsx_async_abort) > (size_t)WINDOW_SIZE.ws_col - 33)
        {
            i = 0;
            while (true)
            {
                printf("%.*s\n", WINDOW_SIZE.ws_col - 33, g_cpuinfo.vulnerability_tsx_async_abort + i);
                i += WINDOW_SIZE.ws_col - 33;
                if (g_cpuinfo.vulnerability_tsx_async_abort[i] != '\0')
                    printf("%33s", " ");
                else
                {
                    printf("\n");
                    break;
                }
            }
        }
        else
            printf("%s\n", g_cpuinfo.vulnerability_tsx_async_abort);
    }
    // Flags
    if (strlen(g_cpuinfo.flags))
    {
        printf("%-33s", "Flags:");
        if (strlen(g_cpuinfo.flags) > (size_t)WINDOW_SIZE.ws_col - 33)
        {
            i = 0;
            while (true)
            {
                printf("%.*s\n", WINDOW_SIZE.ws_col - 33, g_cpuinfo.flags + i);
                i += WINDOW_SIZE.ws_col - 33;
                if (g_cpuinfo.flags[i] != '\0')
                    printf("%33s", " ");
                else
                {
                    printf("\n");
                    break;
                }
            }
        }
        else
            printf("%s\n", g_cpuinfo.flags);
    }
}

/**
 * @brief Convert bytes to human readable
 * @param bytes Bytes
 * @return char* Human readable string
 */
char *byte_to_human(uint64_t bytes)
{
    char *suffix[] = {"B", "KiB", "MiB", "GiB", "TiB"};
    char length = sizeof(suffix) / sizeof(suffix[0]);
    uint32_t i = 0;
    double d_bytes = bytes;

    if (bytes > 1024)
    {
        for (i = 0; (bytes / 1024) > 0 && i < (uint32_t)length - 1; i++, bytes /= 1024)
            d_bytes = bytes / 1024.0;
    }

    static char output[200];
    sprintf(output, "%.0lf %s", d_bytes, suffix[i]);
    return output;
}