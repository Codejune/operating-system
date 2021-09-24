#include "mytop.h"

// Running option
// Spacebar / Enter: 화면 갱신
// k: kill 실행
// r: 우선순위 변경
// z: 변경되는 값과 변경되지 않은 값의 경계

/**
 * @brief Main entry function 
 * @param argc Argument count
 * @param argv Argument verse
 * @return int Ok 0, Error -1
 */
int main(int argc, char *argv[])
{
    uint8_t key;

    if (parse_option(argc, argv) < 0)
        exit(1);

    initscr();
    signal(SIGALRM, alarm_handler);
    while (true)
    {
        ioctl(2, TIOCGWINSZ, &WINDOW_SIZE); // 터미널크기 측정
        alarm(REFRESH_PERIOD);
        update();
        refresh();
        key = getch();
        // Input 'q'
        if (key == 113)
            break;
        else if (key == 65 && !(VIEW_POSITION == 0)) // KEY_DOWN
            VIEW_POSITION--;
        else if (key == 66 && !(VIEW_POSITION == LAST_PROCESS_COUNT)) // KEY_UP
            VIEW_POSITION++;
        clear();
    }
    endwin();
    exit(0);
}

/**
 * @brief Parse command argument option
 * @param argc Argument count
 * @param argv Argument verse
 * @return int Ok 0, Error -1
 */
int parse_option(int argc, char *argv[])
{
    char opt;
    while ((opt = getopt(argc, argv, "hd:p:")) != -1)
    {
        switch (opt)
        {
        // Print help
        case 'h':
            print_help();
            return -1;
        // Set refresh cycle
        case 'd':
            REFRESH_PERIOD = atoi(optarg);
            break;
        // Set target pid
        case 'p':
            TARGET_PID = atoi(optarg);
            break;
        // Unknown or invalid option
        case '?':
            print_help();
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Print help text to screen
 */
void print_help(void)
{
    printf("Usage:\n  ");
    printf("mytop -hv | ");
    printf("-bcEHiOSs1 ");
    printf("-d secs ");
    printf("-n max ");
    printf("-u|U user ");
    printf("-p pid(s) ");
    printf("-o field ");
    printf("-w [cols]\n");
}

/**
 * @brief Signal alarm handler for refresh screen
 * @param signo Signal number
 */
void alarm_handler(int signo)
{
    if (signo == SIGALRM)
    {
        clear();
        alarm(REFRESH_PERIOD);
        update();
        refresh();
    }
}

/**
 * @brief Get system status
 */
void update(void)
{
    header *new_header;
    process *new_processes;
    new_header = get_header();
    print_header(new_header);
    new_processes = get_processes(new_header);
    print_processes(new_header, new_processes);
    printw("											\n");
}

/**
 * @brief Get header
 * @return header* header pointer
 */
header *get_header(void)
{
    header *h = (header *)malloc(sizeof(header));
    get_current_time(h);
    get_uptime(h);
    get_user_count(h);
    get_load_average(h);
    get_tasks_status(h);
    get_cpu_status(h);
    get_memmory_status(h);
    return h;
}

/**
 * @brief Get current time 
 * @param h header struct pointer
 */
void get_current_time(header *h)
{
    struct tm now_tm;
    time_t now_t;
    time(&now_t);
    now_tm = *localtime(&now_t);
    sprintf(h->current_time, "%02d:%02d:%02d", now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec);
}

/**
 * @brief Get uptime from /proc/uptime
 * @param h header struct pointer
 */
void get_uptime(header *h)
{
    FILE *fp;
    char buffer[BUFFER_SIZE] = {0};

    if ((fp = fopen("/proc/uptime", "r")) == NULL)
    {
        fprintf(stderr, "fopen error for /proc/uptime\n");
        exit(1);
    }

    fscanf(fp, "%[^ ]", buffer);
    h->uptime = atoll(buffer);
    fclose(fp);
}

/**
 * @brief Get user count from /etc/passwd
 * @param h header struct pointer
 */
void get_user_count(header *h)
{
    FILE *fp;
    char buffer[BUFFER_SIZE] = {0};

    if ((fp = fopen("/etc/passwd", "r")) == NULL)
    {
        fprintf(stderr, "fopen error for /etc/passwd");
        exit(1);
    }

    h->user_count = 0;

    while (fscanf(fp, "%[^\n]", buffer) != 0)
        if (strstr(buffer, "/bin/bash") != NULL)
            h->user_count++;

    fclose(fp);
}

/**
 * @brief Get load average from /proc/loadavg
 * @param h header struct pointer
 */
void get_load_average(header *h)
{
    FILE *fp;

    if ((fp = fopen("/proc/loadavg", "r")) == NULL)
    {
        fprintf(stderr, "fopen error for /proc/loadavg");
        exit(1);
    }

    fscanf(fp, "%f %f %f", &h->load_average[0], &h->load_average[1], &h->load_average[2]);
    fclose(fp);
}

/**
 * @brief Get status count of tasks from /proc
 * @param h header struct pointer
 */
void get_tasks_status(header *h)
{
    FILE *fp;
    uint32_t i, file_count, pid = 1;
    char file_name[MAX_BUFFER_SIZE] = {0}, buffer[BUFFER_SIZE];
    struct dirent **namelist;

    for (i = 0; i < 5; i++)
        h->task[i] = 0;

    file_count = scandir("/proc", &namelist, NULL, alphasort);

    for (i = 0; i < file_count; i++)
    {
        if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..") || !atoi(namelist[i]->d_name))
            continue;

        // memset(file_name, 0, sizeof(file_name));
        sprintf(file_name, "/proc/%s/stat", namelist[i]->d_name);

        if ((fp = fopen(file_name, "r")) == NULL)
            continue;

        fscanf(fp, "%*s%*s%s%*s", buffer);

        // Total
        h->task[0]++;
        // Running
        if (!strcmp(buffer, "R"))
            h->task[1]++;
        // Sleeping
        else if (!strcmp(buffer, "S") || !strcmp(buffer, "I"))
            h->task[2]++;
        // Stopping
        else if (!strcmp(buffer, "T") || !strcmp(buffer, "t"))
            h->task[3]++;
        // Zombie
        else if (!strcmp(buffer, "Z"))
            h->task[4]++;

        pid++;

        fclose(fp);
    }

    /*************************************************************************/
    // 첫 실행인 경우, old cpu amount table 초기화
    if (LAST_CPU_UPTIME == NULL)
    {
        LAST_CPU_UPTIME = (uint64_t **)malloc(file_count * sizeof(uint64_t *));
        LAST_PROCESS_COUNT = file_count;
        for (i = 0; i < file_count; i++)
        {
            LAST_CPU_UPTIME[i] = (uint64_t *)malloc(2 * sizeof(uint64_t));
            LAST_CPU_UPTIME[i][0] = 0;
            LAST_CPU_UPTIME[i][1] = 0;
        }
    }
    /*************************************************************************/

    for (i = 0; i < file_count; i++)
        free(namelist[i]);
    free(namelist);
}

/**
 * @brief Get cpu status for utilization
 * @param h header struct pointer
 */
void get_cpu_status(header *h)
{
    uint64_t cpu_status[8];
    uint8_t i;
    FILE *fp;

    if ((fp = fopen("/proc/stat", "r")) == NULL)
    {
        fprintf(stderr, "fopen error for /proc/stat\n");
        exit(1);
    }

    fscanf(fp, "%*s %ld %ld %ld %ld %ld %ld %ld %ld", &cpu_status[0], &cpu_status[1], &cpu_status[2], &cpu_status[3], &cpu_status[4], &cpu_status[5], &cpu_status[6], &cpu_status[7]);

    // user
    h->cpu[0] = cpu_status[0] - LAST_CPU_STATUS[0];
    // system
    h->cpu[1] = cpu_status[2] - LAST_CPU_STATUS[2];
    // nice
    h->cpu[2] = cpu_status[1] - LAST_CPU_STATUS[1];
    // idle, IO-wait, h/w interrupts, s/w interrupts, stolen
    for (i = 3; i < 8; i++)
        h->cpu[i] = cpu_status[i] - LAST_CPU_STATUS[i];

    memcpy(LAST_CPU_STATUS, cpu_status, sizeof(cpu_status));

    TOTAL_CPU_TIME = 0;
    for (i = 0; i < 8; i++)
        TOTAL_CPU_TIME += h->cpu[i];

    fclose(fp);
}

/**
 * @brief Get memmory status from /proc/meminfo
 * @param h header struct pointer
 */
void get_memmory_status(header *h)
{
    uint8_t i;
    uint64_t size, buffer_cache = 0;
    FILE *fp;

    if ((fp = fopen("/proc/meminfo", "r")) == NULL)
    {
        fprintf(stderr, "fopen error for /proc/meminfo\n");
        exit(1);
    }

    for (i = 0; i < 22; i++)
    {
        fscanf(fp, "%*s%ld%*s\n", &size);

        switch (i)
        {
        case 0: // MemTotal
            h->physical_memory[0] = (double)size / 1024;
            TOTAL_MEMORY_SIZE = size;
            break;
        case 1: // MemFree
            h->physical_memory[1] = (double)size / 1024;
            break;
        case 2: // MemAvailable
            h->virtual_memory[3] = (double)size / 1024;
            break;
        case 3: // Buffers
            buffer_cache += size;
            break;
        case 4: // Cached
            buffer_cache += size;
            break;
        case 14: // SwapTotal
            h->virtual_memory[0] = (double)size / 1024;
            break;
        case 15: // SwapFree
            h->virtual_memory[1] = (double)size / 1024;
            h->virtual_memory[2] = h->virtual_memory[0] - h->virtual_memory[1];
            break;
        case 21: // Reclaim
            buffer_cache += size;
            h->physical_memory[3] = (double)buffer_cache / 1024;
            h->physical_memory[2] = h->physical_memory[0] - h->physical_memory[1] - h->physical_memory[3];
            break;
        }
    }

    fclose(fp);
}

/**
 * @brief Print header information
 * @param h header struct pointer
 */
void print_header(header *h)
{
    char buffer[MAX_BUFFER_SIZE] = {0};
    uint8_t hour, min;

    min = (int)h->uptime / 60 % 60;
    hour = h->uptime / (60 * 60);

    // 1 Line
    if (hour == 0)
        sprintf(buffer, "mytop - %s up %02d minutes,  %d users,  load average: %.2f, %.2f, %.2f",
                h->current_time, min, h->user_count, h->load_average[0], h->load_average[1], h->load_average[2]);
    else if (hour > 23)
        sprintf(buffer, "mytop - %s up %d days, %02d:%02d,  %d users,  load average: %.2f, %.2f, %.2f",
                h->current_time, hour / 24, hour % 24, min, h->user_count, h->load_average[0], h->load_average[1], h->load_average[2]);
    else
        sprintf(buffer, "mytop - %s up %02d:%02d,  %d users,  load average: %.2f, %.2f, %.2f",
                h->current_time, hour, min, h->user_count, h->load_average[0], h->load_average[1], h->load_average[2]);
    buffer[WINDOW_SIZE.ws_col - 1] = '\0';
    printw("%s\n", buffer);
    // 2 Line
    sprintf(buffer, "Tasks: %3d total, %3d running, %3d sleeping, %3d stopped, %3d zombie",
            h->task[0], h->task[1], h->task[2], h->task[3], h->task[4]);
    buffer[WINDOW_SIZE.ws_col - 1] = '\0';
    printw("%s\n", buffer);
    // 3 Line
    sprintf(buffer, "%%Cpu(s): %.1f us, %.1f sy, %.1f ni, %.1f id, %.1f wa, %.1f hi, %.1f si, %.1f st",
            (double)h->cpu[0] / TOTAL_CPU_TIME * 100,
            (double)h->cpu[1] / TOTAL_CPU_TIME * 100,
            (double)h->cpu[2] / TOTAL_CPU_TIME * 100,
            (double)h->cpu[3] / TOTAL_CPU_TIME * 100,
            (double)h->cpu[4] / TOTAL_CPU_TIME * 100,
            (double)h->cpu[5] / TOTAL_CPU_TIME * 100,
            (double)h->cpu[6] / TOTAL_CPU_TIME * 100,
            (double)h->cpu[7] / TOTAL_CPU_TIME * 100);
    buffer[WINDOW_SIZE.ws_col - 1] = '\0';
    printw("%s\n", buffer);
    // 4 Line
    sprintf(buffer, "MiB Mem : %9.1f total, %9.1f free, %9.1f used, %9.1f buff/cache",
            h->physical_memory[0],
            h->physical_memory[1],
            h->physical_memory[2],
            h->physical_memory[3]);
    buffer[WINDOW_SIZE.ws_col - 1] = '\0';
    printw("%s\n", buffer);
    // 5 Line
    sprintf(buffer, "MiB Swap: %9.1f total, %9.1f free, %9.1f used, %9.1f avail Mem",
            h->virtual_memory[0],
            h->virtual_memory[1],
            h->virtual_memory[2],
            h->virtual_memory[3]);
    buffer[WINDOW_SIZE.ws_col - 1] = '\0';
    printw("%s\n\n", buffer);
}

/**
 * @brief Get processes list
 * @param h header struct pointer
 * @return process* process struct pointer
 */
process *get_processes(header *h)
{
    uint32_t file_count, current_pid = 0, i, j;
    uint64_t utime, stime, cutime, cstime, start_time, seconds, total_time;
    char file_name[MAX_BUFFER_SIZE] = {0}, buffer[BUFFER_SIZE] = {0};
    FILE *stat_fp, *status_fp;
    struct dirent **namelist;
    struct stat statbuf;
    process *p = (process *)malloc(h->task[0] * sizeof(process));

    file_count = scandir("/proc", &namelist, NULL, alphasort);
    for (i = 0; i < file_count; i++)
    {
        if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..") || !atoi(namelist[i]->d_name))
            continue;

        sprintf(file_name, "/proc/%s", namelist[i]->d_name);
        stat(file_name, &statbuf);
        get_user_name(statbuf.st_uid, p[current_pid].user_name);
        strcat(file_name, "/stat");

        if ((stat_fp = fopen(file_name, "r")) == NULL)
            continue;

        strcat(file_name, "us");

        if ((status_fp = fopen(file_name, "r")) == NULL)
            continue;

        /**
         * https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat
         * CPU utilization calculation
         * total_time = utime + stime
         * total_time = total_time + cutime + cstime
         * seconds = uptime - (starttime / hertz)
         * cpu_usage = 100 * ((total_time / hertz) / seconds)
         */
        // CPU utilization
        start_time = utime = stime = cutime = cstime = seconds = 0;
        for (j = 0; j < 23; j++)
        {
            fscanf(stat_fp, "%[^ ]", buffer);
            fgetc(stat_fp);

            switch (j)
            {
            // pid
            case 0:
                p[current_pid].pid = atoi(buffer);
                break;
            // command
            case 1:
                sprintf(p[current_pid].command, "%.*s", (int)strlen(buffer) - 2, buffer + 1);
                break;
            // status
            case 2:
                p[current_pid].status = buffer[0];
                break;
            // utime
            case 13:
                utime = atoll(buffer);
                break;
            // stime
            case 14:
                stime = atoll(buffer);
                get_time_format(utime + stime, p[current_pid].time);
                break;
            // cutime
            case 15:
                cutime = atoll(buffer);
                break;
            // cstime
            case 16:
                cstime = atoll(buffer);
                break;
            // priority
            case 17:
                p[current_pid].priority = atoi(buffer);
                break;
            // nice
            case 18:
                p[current_pid].nice = atoi(buffer);
                break;
            // starttime
            case 21:
                start_time = atoll(buffer);
                break;
            }
        }
        total_time = utime + stime;
        seconds = h->uptime - (start_time / sysconf(_SC_CLK_TCK));

        if (total_time == 0 || start_time == 0 || seconds == 0)
            p[current_pid].cpu_utilization = 0;
        else
            p[current_pid].cpu_utilization = 100 * ((total_time / sysconf(_SC_CLK_TCK)) / seconds);
        p[current_pid].virtual_memory = 0;
        p[current_pid].physical_memeory = 0;
        p[current_pid].shared_memory = 0;

        // Memory utilization
        while (true)
        {
            if (fscanf(status_fp, "%[^:]", buffer) == EOF)
                break;

            fgetc(status_fp);

            if (!strcmp(buffer, "VmSize"))
            {
                fscanf(status_fp, "%skB%*s\n", buffer);
                p[current_pid].virtual_memory = atoll(buffer);
            }
            else if (!strcmp(buffer, "VmRSS"))
            {
                fscanf(status_fp, "%skB%*s\n", buffer);
                p[current_pid].physical_memeory = atoll(buffer);
            }
            else if (!strcmp(buffer, "RssFile"))
            {
                fscanf(status_fp, "%skB%*s\n", buffer);
                p[current_pid].shared_memory = atoll(buffer);
            }
            else
            {
                fscanf(status_fp, "%[^\n]", buffer);
                fgetc(status_fp);
            }
        }
        p[current_pid].memory_utilization = (float)p[current_pid].physical_memeory / TOTAL_MEMORY_SIZE * 100;
        current_pid++;
        fclose(stat_fp);
        fclose(status_fp);
    }

    for (i = 0; i < file_count; i++)
        free(namelist[i]);
    free(namelist);

    /*************************************************************************/
    // old_cpu_amount 관리
    free(LAST_CPU_UPTIME);
    LAST_CPU_UPTIME = (uint64_t **)malloc(file_count * sizeof(uint64_t *));
    LAST_PROCESS_COUNT = file_count;

    for (i = 0; i < file_count; i++)
    {
        // old_cpu_amount table 매핑
        LAST_CPU_UPTIME[i] = (uint64_t *)malloc(2 * sizeof(uint64_t));
        LAST_CPU_UPTIME[i][0] = p[i].pid;
        LAST_CPU_UPTIME[i][1] = utime + stime;
    }
    /*************************************************************************/
    return p;
}

/**
 * @brief Get user name about process
 * @param uid Process user id
 * @param buffer Return buffer
 */
void get_user_name(int uid, char *buffer)
{
    uint16_t target_uid;
    char tmp[BUFFER_SIZE] = {0};
    FILE *fp;

    if ((fp = fopen("/etc/passwd", "r")) == NULL)
    {
        fprintf(stderr, "fopen error for /etc/passwd");
        exit(1);
    }

    while (true)
    {
        fscanf(fp, "%[^:]", buffer);
        fgetc(fp);
        fscanf(fp, "%[^:]", tmp);
        fgetc(fp);
        fscanf(fp, "%hd", &target_uid);
        fgetc(fp);

        if (target_uid == uid)
        {
            if (strlen(buffer) > 7)
            {
                buffer[7] = '+';
                buffer[8] = '\0';
            }
            break;
        }

        fscanf(fp, "%[^\n]", tmp);
        fgetc(fp);
    }
    fclose(fp);
}

/**
 * @brief Get time format from total time
 * @param total_time Process total time
 * @param buffer Buffer
 */
void get_time_format(uint64_t total_time, char *buffer)
{
    uint64_t hour, min, sec;

    sec = total_time % sysconf(_SC_CLK_TCK);
    min = total_time / sysconf(_SC_CLK_TCK);
    hour = min / 60;
    min %= 60;

    if (1000 <= hour)
        sprintf(buffer, "%ld:%02ld", hour, min);
    else
        sprintf(buffer, "%ld:%02ld.%02ld", hour, min, sec);
}

/**
 * @brief Print process list
 * @param h header struct pointer
 * @param p process struct pointer
 */
void print_processes(header *h, process *p)
{
    char buffer[MAX_BUFFER_SIZE] = {0};
    char tmp[BUFFER_SIZE] = {0};
    uint32_t i;

    // Header line
    attrset(A_STANDOUT);
    sprintf(buffer, "    PID USER      PR  NI     VIRT     RES     SHR S  %%CPU  %%MEM      TIME+ COMMAND                                                                                                                                            ");
    attron(A_STANDOUT);
    buffer[WINDOW_SIZE.ws_col - 1] = '\0';
    printw("%s\n", buffer);
    attroff(A_STANDOUT);
    // List line
    sort_by_pid(p, h->task[0]);
    sort_by_cpu(p, h->task[0]);
    for (i = VIEW_POSITION; i < WINDOW_SIZE.ws_row + VIEW_POSITION - 7; i++)
    {
        if (p[i].status == 'R')
            attron(A_BOLD);

        // PID
        sprintf(buffer, "%7d ", p[i].pid);
        // USER
        sprintf(tmp, "%-8s ", p[i].user_name);
        strcat(buffer, tmp);
        // PR
        if (p[i].priority == (int16_t)-100)
            sprintf(tmp, "%3s ", "rt");
        else
            sprintf(tmp, "%3d ", p[i].priority);
        strcat(buffer, tmp);
        // NI
        sprintf(tmp, "%3d ", p[i].nice);
        strcat(buffer, tmp);
        // VIRT
        sprintf(tmp, "%8ld ", p[i].virtual_memory);
        strcat(buffer, tmp);
        // RES
        sprintf(tmp, "%7ld ", p[i].physical_memeory);
        strcat(buffer, tmp);
        // SHR
        sprintf(tmp, "%7ld ", p[i].shared_memory);
        strcat(buffer, tmp);
        // S
        sprintf(tmp, "%c ", p[i].status);
        strcat(buffer, tmp);
        // %CPU
        sprintf(tmp, "%4.1f ", p[i].cpu_utilization);
        strcat(buffer, tmp);
        // %MEM
        sprintf(tmp, "%4.1f ", p[i].memory_utilization);
        strcat(buffer, tmp);
        // TIME+
        sprintf(tmp, "%11s ", p[i].time);
        strcat(buffer, tmp);
        // COMMAND
        sprintf(tmp, "%s ", p[i].command);
        strcat(buffer, tmp);

        buffer[WINDOW_SIZE.ws_col - 1] = '\0';
        printw("%s\n", buffer);
        attroff(A_BOLD);
    }

    free(p);
}

/**
 * @brief Sort process list ordered by process id
 * @param p process struct pointer
 * @param process_count Process count
 */
void sort_by_pid(process *p, uint32_t process_count)
{
    uint32_t i, j;
    process tmp;

    for (i = 0; i < process_count; i++)
        for (j = i; j < process_count; j++)
            if (p[i].pid > p[j].pid)
            {
                tmp = p[i];
                p[i] = p[j];
                p[j] = tmp;
            }
}

/**
 * @brief Sort process list ordered by cpu utilization
 * @param p process struct pointer
 * @param process_count Process count
 */
void sort_by_cpu(process *p, uint32_t process_count)
{
    uint32_t i, j;
    process tmp;

    for (i = 0; i < process_count; i++)
        for (j = i; j < process_count; j++)
            if (p[i].cpu_utilization < p[j].cpu_utilization)
            {
                tmp = p[i];
                p[i] = p[j];
                p[j] = tmp;
            }
}