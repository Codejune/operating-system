/**
 * @file mytop.c
 * @author 김병준 (kbj9704@gmail.com)
 * @brief mytop 명령어 기능 함수 구현
 */
#include "mytop.h"

/**
 * @brief Main entry function 
 * @param argc Argument count
 * @param argv Argument verse
 * @return int Ok 0, Error 1
 */
int main(int argc, char *argv[])
{
    uint8_t key;

    parse_option(argc, argv);       // Option check
    initscr();                      // Screen initialization
    signal(SIGALRM, alarm_handler); // Signal regist
    while (true)
    {
        ioctl(2, TIOCGWINSZ, &WINDOW_SIZE); // 터미널크기 측정
        alarm(REFRESH_PERIOD);
        update();
        refresh();
        key = getch();
        switch (key)
        {
        // q
        case (int)'q':
            endwin();
            exit(0);
        // Key down
        case 65:
            if (VIEW_ROW_POSITION != 0)
                VIEW_ROW_POSITION--;
            break;
        // Key up
        case 66:
            if (VIEW_ROW_POSITION != g_header.task[0])
                VIEW_ROW_POSITION++;
            break;
        }
        clear();
    }
    endwin();
    exit(EXIT_SUCCESS);
}

/**
 * @brief Parse command argument option
 * @param argc Argument count
 * @param argv Argument verse
 * @return int Ok 0, Error -1
 */
void parse_option(int argc, char *argv[])
{
    char opt;
    while ((opt = getopt(argc, argv, "hd:p:")) != -1)
    {
        switch (opt)
        {
        // Print help
        case 'h':
            print_help();
            exit(EXIT_FAILURE);
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
            exit(EXIT_FAILURE);
        }
    }
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
    process *new_processes;
    update_header();
    print_header();
    new_processes = get_processes();
    print_processes(new_processes);
    printw("											\n");
    free(g_last_processes);
    g_last_processes = new_processes;
}

/**
 * @brief Update header
 */
void update_header(void)
{
    get_current_time();
    get_uptime();
    get_user_count();
    get_load_average();
    get_tasks_status();
    get_cpu_status();
    get_memmory_status();
}

/**
 * @brief Get current time 
 * @param h header struct pointer
 */
void get_current_time()
{
    struct tm now_tm;
    time_t now_t;
    time(&now_t);
    now_tm = *localtime(&now_t);
    sprintf(g_header.current_time, "%02d:%02d:%02d", now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec);
}

/**
 * @brief Get uptime from /proc/uptime
 */
void get_uptime()
{
    FILE *fp;
    if ((fp = fopen("/proc/uptime", "r")) == NULL)
    {
        fprintf(stderr, "fopen error for /proc/uptime\n");
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%f", &g_header.uptime);
    fclose(fp);
}

/**
 * @brief Get user count from /etc/passwd
 */
void get_user_count()
{
    struct utmp *user;

    g_header.user_count = 0;
    setutent();
    while ((user = getutent()) != NULL)
        if (user->ut_type == USER_PROCESS)
            g_header.user_count++;
    endutent();
}

/**
 * @brief Get load average from /proc/loadavg
 */
void get_load_average()
{
    FILE *fp;

    if ((fp = fopen("/proc/loadavg", "r")) == NULL)
    {
        fprintf(stderr, "fopen error for /proc/loadavg");
        exit(EXIT_FAILURE);
    }

    fscanf(fp, "%f %f %f", &g_header.load_average[0], &g_header.load_average[1], &g_header.load_average[2]);
    fclose(fp);
}

/**
 * @brief Get status count of tasks from /proc
 */
void get_tasks_status()
{
    FILE *fp;
    uint32_t i, file_count, pid = 1;
    char file_name[MAX_BUFFER_SIZE] = {0}, buffer;
    struct dirent **namelist;

    memset(g_header.task, 0, sizeof(g_header.task));
    file_count = scandir("/proc", &namelist, NULL, alphasort);
    for (i = 0; i < file_count; i++)
    {
        if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..") || !atoi(namelist[i]->d_name))
            continue;

        sprintf(file_name, "/proc/%s/stat", namelist[i]->d_name);

        if ((fp = fopen(file_name, "r")) == NULL)
            continue;

        fscanf(fp, "%*s %*s %c %*s", &buffer);

        // Total
        g_header.task[0]++;
        switch (buffer)
        {
        // Running
        case 'R':
            g_header.task[1]++;
            break;
        // Sleeping
        case 'S':
        case 'I':
            g_header.task[2]++;
            break;
        // Stopping
        case 'T':
        case 't':
            g_header.task[3]++;
            break;
        // Zombie
        case 'Z':
            g_header.task[4]++;
            break;
        }
        pid++;

        fclose(fp);
    }

    for (i = 0; i < file_count; i++)
        free(namelist[i]);
    free(namelist);
}

/**
 * @brief Get cpu status for utilization
 */
void get_cpu_status()
{
    uint64_t current_cpu_status[8];
    uint8_t i;
    FILE *fp;

    if ((fp = fopen("/proc/stat", "r")) == NULL)
    {
        fprintf(stderr, "fopen error for /proc/stat\n");
        exit(EXIT_FAILURE);
    }

    fscanf(fp, "%*s %ld %ld %ld %ld %ld %ld %ld %ld",
           &current_cpu_status[0],
           &current_cpu_status[1],
           &current_cpu_status[2],
           &current_cpu_status[3],
           &current_cpu_status[4],
           &current_cpu_status[5],
           &current_cpu_status[6],
           &current_cpu_status[7]);

    g_cpu_total_time = 0;
    for (i = 0; i < 8; i++)
    {
        switch (i)
        {
        // system
        case 1:
            g_header.cpu[1] = current_cpu_status[2] - g_last_cpu_status[2];
            break;
        // nice
        case 2:
            g_header.cpu[2] = current_cpu_status[1] - g_last_cpu_status[1];
            break;
        // total, idle, IO-wait, h/w interrupts, s/w interrupts, stolen
        default:
            g_header.cpu[i] = current_cpu_status[i] - g_last_cpu_status[i];
            break;
        }
        g_cpu_total_time += g_header.cpu[i];
    }
    memcpy(g_last_cpu_status, current_cpu_status, sizeof(current_cpu_status));
    fclose(fp);
}

/**
 * @brief Get memmory status from /proc/meminfo
 */
void get_memmory_status()
{
    uint8_t i;
    uint64_t size, buffer_cache = 0;
    FILE *fp;

    if ((fp = fopen("/proc/meminfo", "r")) == NULL)
    {
        fprintf(stderr, "fopen error for /proc/meminfo\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < 22; i++)
    {
        fscanf(fp, "%*s%ld%*s\n", &size);
        switch (i)
        {
        case 0: // MemTotal
            g_header.physical_memory[0] = (double)size / 1024;
            g_memory_total_size = size;
            break;
        case 1: // MemFree
            g_header.physical_memory[1] = (double)size / 1024;
            break;
        case 2: // MemAvailable
            g_header.virtual_memory[3] = (double)size / 1024;
            break;
        case 3: // Buffers
        case 4: // Cached
            buffer_cache += size;
            break;
        case 14: // SwapTotal
            g_header.virtual_memory[0] = (double)size / 1024;
            break;
        case 15: // SwapFree
            g_header.virtual_memory[1] = (double)size / 1024;
            // SwapUsed = SwapTotal - SwapFree
            g_header.virtual_memory[2] = g_header.virtual_memory[0] - g_header.virtual_memory[1];
            break;
        case 21: // Reclaim
            buffer_cache += size;
            g_header.physical_memory[3] = (double)buffer_cache / 1024;
            // MemUsed = MemTotal - MemFree - Buff/Cache
            g_header.physical_memory[2] = g_header.physical_memory[0] - g_header.physical_memory[1] - g_header.physical_memory[3];
            break;
        }
    }
    fclose(fp);
}

/**
 * @brief Print header information
 */
void print_header()
{
    char buffer[MAX_BUFFER_SIZE] = {0};
    uint8_t hour, min;

    min = (int)g_header.uptime / 60 % 60;
    hour = g_header.uptime / (60 * 60);

    // 1 Line
    if (hour == 0)
        sprintf(buffer, "mytop - %s up %02d minutes,  %d users,  load average: %.2f, %.2f, %.2f",
                g_header.current_time, min, g_header.user_count, g_header.load_average[0], g_header.load_average[1], g_header.load_average[2]);
    else if (hour > 23)
        sprintf(buffer, "mytop - %s up %d days, %02d:%02d,  %d users,  load average: %.2f, %.2f, %.2f",
                g_header.current_time, hour / 24, hour % 24, min, g_header.user_count, g_header.load_average[0], g_header.load_average[1], g_header.load_average[2]);
    else
        sprintf(buffer, "mytop - %s up %02d:%02d,  %d users,  load average: %.2f, %.2f, %.2f",
                g_header.current_time, hour, min, g_header.user_count, g_header.load_average[0], g_header.load_average[1], g_header.load_average[2]);
    buffer[WINDOW_SIZE.ws_col - 1] = '\0';
    printw("%s\n", buffer);
    // 2 Line
    sprintf(buffer, "Tasks: %3d total, %3d running, %3d sleeping, %3d stopped, %3d zombie",
            g_header.task[0], g_header.task[1], g_header.task[2], g_header.task[3], g_header.task[4]);
    buffer[WINDOW_SIZE.ws_col - 1] = '\0';
    printw("%s\n", buffer);
    // 3 Line
    sprintf(buffer, "%%Cpu(s): %.1f us, %.1f sy, %.1f ni, %.1f id, %.1f wa, %.1f hi, %.1f si, %.1f st",
            (double)g_header.cpu[0] / g_cpu_total_time * 100,
            (double)g_header.cpu[1] / g_cpu_total_time * 100,
            (double)g_header.cpu[2] / g_cpu_total_time * 100,
            (double)g_header.cpu[3] / g_cpu_total_time * 100,
            (double)g_header.cpu[4] / g_cpu_total_time * 100,
            (double)g_header.cpu[5] / g_cpu_total_time * 100,
            (double)g_header.cpu[6] / g_cpu_total_time * 100,
            (double)g_header.cpu[7] / g_cpu_total_time * 100);
    buffer[WINDOW_SIZE.ws_col - 1] = '\0';
    printw("%s\n", buffer);
    // 4 Line
    sprintf(buffer, "MiB Mem : %9.1f total, %9.1f free, %9.1f used, %9.1f buff/cache",
            g_header.physical_memory[0],
            g_header.physical_memory[1],
            g_header.physical_memory[2],
            g_header.physical_memory[3]);
    buffer[WINDOW_SIZE.ws_col - 1] = '\0';
    printw("%s\n", buffer);
    // 5 Line
    sprintf(buffer, "MiB Swap: %9.1f total, %9.1f free, %9.1f used, %9.1f avail Mem",
            g_header.virtual_memory[0],
            g_header.virtual_memory[1],
            g_header.virtual_memory[2],
            g_header.virtual_memory[3]);
    buffer[WINDOW_SIZE.ws_col - 1] = '\0';
    printw("%s\n\n", buffer);
}

/**
 * @brief Get processes list
 * @return process* process struct pointer
 */
process *get_processes()
{
    uint32_t file_count, current_pid = 0, i, j;
    double start_time;
    char file_name[MAX_BUFFER_SIZE] = {0}, buffer[BUFFER_SIZE] = {0};
    FILE *stat_fp, *status_fp;
    struct dirent **namelist;
    struct stat statbuf;
    process *p = (process *)calloc(g_header.task[0], sizeof(process));

    file_count = scandir("/proc", &namelist, NULL, alphasort);
    for (i = 0; i < file_count; i++)
    {
        if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..") || !atoi(namelist[i]->d_name))
            continue;

        sprintf(file_name, "/proc/%s", namelist[i]->d_name);
        stat(file_name, &statbuf);
        // USER
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
        for (j = 0; j < 23; j++)
        {
            fscanf(stat_fp, "%[^ ]", buffer);
            fgetc(stat_fp);

            switch (j)
            {
            // PID
            case 0:
                p[current_pid].pid = atoi(buffer);
                break;
            // COMMAND
            case 1:
                sprintf(p[current_pid].command, "%.*s", (int)strlen(buffer) - 2, buffer + 1);
                break;
            // S
            case 2:
                p[current_pid].status = buffer[0];
                break;
            // utime
            case 13:
                p[current_pid].cpu_utilization = atoll(buffer);
                break;
            // TIME+, stime
            case 14:
                p[current_pid].cpu_utilization += atoll(buffer);
                get_time_format(p[current_pid].cpu_utilization, p[current_pid].time);
                break;
            // PR
            case 17:
                p[current_pid].priority = atoi(buffer);
                break;
            // NI
            case 18:
                p[current_pid].nice = atoi(buffer);
                break;
            // starttime
            case 21:
                start_time = atoll(buffer);
                p[current_pid].cpu_utilization = 100 * ((p[current_pid].cpu_utilization / HERTZ) / (g_header.uptime - (start_time / HERTZ)));
                break;
            }
        }

        p[current_pid].virtual_memory = 0;
        p[current_pid].physical_memeory = 0;
        p[current_pid].shared_memory = 0;
        while (true)
        {
            if (fscanf(status_fp, "%[^:]", buffer) == EOF)
                break;

            fgetc(status_fp);

            // VIRT
            if (!strcmp(buffer, "VmSize"))
            {
                fscanf(status_fp, "%skB%*s\n", buffer);
                p[current_pid].virtual_memory = atoll(buffer);
            }
            // RES
            else if (!strcmp(buffer, "VmRSS"))
            {
                fscanf(status_fp, "%skB%*s\n", buffer);
                p[current_pid].physical_memeory = atoll(buffer);
            }
            // SHR
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
        // %MEM
        p[current_pid].memory_utilization = (float)p[current_pid].physical_memeory / g_memory_total_size * 100;
        current_pid++;
        fclose(stat_fp);
        fclose(status_fp);
    }

    // for (i = 0; i < file_count; i++)
    //     free(namelist[i]);
    // free(namelist);

    return p;
}

/**
 * @brief Get user name about process
 * @param uid Process user id
 * @param buffer Return buffer
 */
void get_user_name(uint32_t uid, char *buffer)
{
    struct passwd *upasswd = getpwuid(uid);
    strncpy(buffer, upasswd->pw_name, 16);
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
 * @param p process struct pointer
 */
void print_processes(process *p)
{
    char buffer[MAX_BUFFER_SIZE] = {0};
    char tmp[BUFFER_SIZE] = {0};
    uint32_t i;

    // Header line
    attrset(A_STANDOUT);
    sprintf(buffer, "    PID USER      PR  NI     VIRT     RES     SHR S  %%CPU  %%MEM       TIME+ COMMAND                                                                                                                                            ");
    attron(A_STANDOUT);
    buffer[WINDOW_SIZE.ws_col - 1] = '\0';
    printw("%s\n", buffer);
    attroff(A_STANDOUT);
    // List line
    sort_by_pid(p);
    sort_by_cpu(p);
    for (i = VIEW_ROW_POSITION; i < WINDOW_SIZE.ws_row + VIEW_ROW_POSITION - 7; i++)
    {
        if (p[i].status == 'R')
            attron(A_BOLD);
        // PID
        sprintf(buffer, "%7d ", p[i].pid);
        // USER
        if (strlen(p[i].user_name) > 7)
            sprintf(tmp, "%-.*s+ ", 7, p[i].user_name);
        else
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
        sprintf(tmp, "%5.1f ", p[i].cpu_utilization);
        strcat(buffer, tmp);
        // %MEM
        sprintf(tmp, "%5.1f ", p[i].memory_utilization);
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
}

/**
 * @brief Sort process list ordered by process id
 * @param p process struct pointer
 */
void sort_by_pid(process *p)
{
    uint32_t i, j;
    process tmp;

    for (i = 0; i < g_header.task[0]; i++)
        for (j = i; j < g_header.task[0]; j++)
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
 */
void sort_by_cpu(process *p)
{
    uint32_t i, j;
    process tmp;

    for (i = 0; i < g_header.task[0]; i++)
        for (j = i; j < g_header.task[0]; j++)
            if (p[i].cpu_utilization < p[j].cpu_utilization)
            {
                tmp = p[i];
                p[i] = p[j];
                p[j] = tmp;
            }
}