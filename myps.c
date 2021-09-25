/**
 * @file myps.c
 * @author 김병준 (kbj9704@gmail.com)
 * @brief myps 명령어 기능 함수 구현
 */

#include "myps.h"

/**
 * @brief Main entry function 
 * @param argc Argument count
 * @param argv Argument verse
 * @return int Ok 0, Error 1
 */
int main(int argc, char *argv[])
{
    process **p;
    uint32_t i;

    // Option check
    parse_option(argc, argv);

    if (ioctl(0, TIOCGWINSZ, &WINDOW_SIZE) < 0)
    {
        fprintf(stderr, "ioctl error\n");
        exit(1);
    }

    g_cpu_total_time = get_cpu_total_time();
    g_memory_total_size = get_memory_total_size();
    p = get_processes();
    sort_by_pid(p);
    print_processes(p);

    for (i = 0; i < g_process_count; i++)
        free(p[i]);
    free(p);

    exit(0);
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
    uint16_t i, j;

    for (i = 1; i < argc; i++) {
        for (j = 0; j < strlen(argv[i]); j++)
            switch (argv[i][j])
            {
            case 'a':
                g_option_a = true;
                break;
            case 'u':
                g_option_u = true;
                break;
            case 'x':
                g_option_x = true;
                break;
            }
    }

    while ((opt = getopt(argc, argv, "h")) != -1)
    {
        switch (opt)
        {
        // Print help
        case 'h':
            print_help();
            exit(1);
        // Unknown or invalid option
        case '?':
            print_help();
            exit(1);
        }
    }
}

/**
 * @brief Print help text to screen
 */
void print_help(void)
{
    printf("Usage:\n");
    printf("  ps [-AaCcEefhjlMmrSTvwXx] [-O fmt | -o fmt] [-G gid[,gid...]]\n");
    printf("     [-g grp[,grp...]] [-u [uid,uid...]]\n");
    printf("     [-p pid[,pid...]] [-t tty[,tty...]] [-U user[,user...]]");
    printf("  ps [-L]");
}

/**
 * @brief Get system's usage CPU total time
 * @return uint64_t CPU total time
 */
uint64_t get_cpu_total_time(void)
{
    uint64_t cpu_time[8], total_time = 0;
    uint8_t i;
    FILE *fp;

    if ((fp = fopen("/proc/stat", "r")) == NULL)
    {
        fprintf(stderr, "fopen error for /proc/stat\n");
        exit(1);
    }

    fscanf(fp, "%*s %ld %ld %ld %ld %ld %ld %ld %ld",
           &cpu_time[0],
           &cpu_time[1],
           &cpu_time[2],
           &cpu_time[3],
           &cpu_time[4],
           &cpu_time[5],
           &cpu_time[6],
           &cpu_time[7]);

    for (i = 0; i < 8; i++)
        total_time += cpu_time[i];

    fclose(fp);

    return total_time;
}

/**
 * @brief Get system's memory total size
 * @return uint64_t Memory total size
 */
uint64_t get_memory_total_size(void)
{
    uint64_t total_size = 0;
    FILE *fp;

    if ((fp = fopen("/proc/meminfo", "r")) == NULL)
    {
        fprintf(stderr, "fopen error for /proc/meminfo\n");
        exit(1);
    }

    fscanf(fp, "%*s%ld%*s\n", &total_size);

    fclose(fp);

    return total_size;
}

process **get_processes(void)
{
    char file_name[MAX_BUFFER_SIZE] = {0};
    uint16_t tty, ireny_tty;
    uint32_t file_count, p_count, irent_pid, pid, i, j = 0;
    FILE *fp;
    struct dirent **namelist;
    struct stat statbuf;
    process **p;
    uid_t uid = getuid();
    bool is_target = true;

    file_count = scandir("/proc", &namelist, NULL, alphasort);
    irent_pid = getpid();

    // Count process count
    for (i = 0; i < file_count; i++)
    {
        if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..") || !(pid = atoi(namelist[i]->d_name)))
            continue;

        p_count++;

        // Get irent tty number
        if (irent_pid == pid)
            ireny_tty = get_tty(irent_pid);
    }

    // Initialization process list
    p = (process **)malloc(p_count * sizeof(process *));
    for (i = 0; i < p_count; i++)
        p[i] = (process *)malloc(sizeof(process));

    for (i = 0; i < file_count; i++)
    {
        if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..") || !(pid = (uint32_t)atoi(namelist[i]->d_name)))
            continue;

        is_target = true;
        sprintf(file_name, "/proc/%d", pid);
        stat(file_name, &statbuf);
        strcat(file_name, "/stat");

        if ((fp = fopen(file_name, "r")) == NULL)
        {
            fprintf(stderr, "fopen error for %s\n", file_name);
            exit(1);
        }

        tty = get_tty(pid);

        // Check print target
        if (!g_option_a && uid != statbuf.st_uid)
            is_target = false;
        if (!g_option_x && tty == 0)
            is_target = false;
        if (!g_option_a && !g_option_x && !g_option_u && ireny_tty != tty)
            is_target = false;
        if (!g_option_a && !g_option_x && g_option_u && uid != statbuf.st_uid)
            is_target = false;

        if (is_target)
            p[j++] = get_process_status(pid);

        fclose(fp);
    }

    for (i = 0; i < file_count; i++)
        free(namelist[i]);
    free(namelist);

    g_process_count = j;
    return p;
}

/**
 * @brief Get tty number
 * @param pid Process id
 * @return uint16_t TTY number
 */
uint16_t get_tty(uint32_t pid)
{
    char file_name[MAX_BUFFER_SIZE] = {0};
    FILE *fp;
    uint16_t tty;

    sprintf(file_name, "/proc/%d/stat", pid);

    if ((fp = fopen(file_name, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", file_name);
        exit(1);
    }

    fscanf(fp, "%*d %*s %*c %*d %*d %*d %hd", &tty);
    fclose(fp);

    return tty;
}

process *get_process_status(uint32_t pid)
{
    char file_name[MAX_BUFFER_SIZE] = {0}, buffer[BUFFER_SIZE] = {0};
    process *p = (process *)calloc(1, sizeof(process));
    struct stat statbuf;
    FILE *fp;
    uint8_t i, j;
    double start_time;

    // PID
    p->pid = pid;
    sprintf(file_name, "/proc/%d", pid);
    stat(file_name, &statbuf);
    // USER
    get_user_name(statbuf.st_uid, p->user_name);
    strcat(file_name, "/stat");
    // Check /proc/{pid}/stat
    if ((fp = fopen(file_name, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", file_name);
        exit(1);
    }
    for (i = 0; i < 22; i++)
    {
        fscanf(fp, "%[^ ]", buffer);
        fgetc(fp);

        switch (i)
        {
        // COMMAND
        case 1:
            sprintf(p->command, "[%.*s]", (int)strlen(buffer) - 2, buffer + 1);
            break;
        // STATE
        case 2:
            p->stat[0] = buffer[0];
            break;
        // session
        case 5:
            if ((uint32_t)atoi(buffer) == pid)
                p->stat[3] = 's';
            break;
        // TTY
        case 6:
            get_terminal(atoi(buffer), p->tty);
            break;
        // tpgid
        case 7:
            if (atoi(buffer) == getpgid(pid))
                p->stat[5] = '+';
            break;
        // utime
        case 13:
            p->cpu = atof(buffer);
            break;
        // stime
        case 14:
            p->cpu += atof(buffer);
            get_time_format(p->cpu, p->time);
            break;
        // nice
        case 18:
            if (atoi(buffer) < 0)
                p->stat[1] = '<';
            else if (atoi(buffer) > 0)
                p->stat[1] = 'N';
            break;
        // thread number
        case 19:
            if (atoi(buffer) > 1)
                p->stat[4] = 'l';
            break;
        // starttime
        case 21:
            start_time = atof(buffer);
            p->cpu = 100 * ((p->cpu / HERTZ) / (get_up_time() - (start_time / HERTZ)));
            get_start_time(start_time, p->start);
            break;
        }
    }
    fclose(fp);
    strcat(file_name, "us");
    // Check /proc/{pid}/status
    if ((fp = fopen(file_name, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", file_name);
        exit(1);
    }
    while (true)
    {
        if (fscanf(fp, "%[^:]", buffer) == EOF)
            break;
        fgetc(fp);

        if (!strcmp(buffer, "VmSize"))
        {
            fscanf(fp, "%skB%*s\n", buffer);
            p->vsz = atoll(buffer);
        }
        // RES
        else if (!strcmp(buffer, "VmRSS"))
        {
            fscanf(fp, "%skB%*s\n", buffer);
            p->rss = atoll(buffer);
            p->memory = (double)p->rss / g_memory_total_size * 100;
        }
        // SHR
        else if (!strcmp(buffer, "VmLck"))
        {
            fscanf(fp, "%skB%*s\n", buffer);
            if (atoll(buffer) > 0)
                p->stat[2] = 'L';
        }
        else
        {
            fscanf(fp, "%[^\n]", buffer);
            fgetc(fp);
        }
    }
    fclose(fp);
    get_command(pid, p->command);

    for (i = 1, j = 1; i < 6; i++)
        if (p->stat[i] != (char)0)
            p->stat[j++] = p->stat[i];
    p->stat[j] = '\0';

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
    if (strlen(upasswd->pw_name) > 7)
        sprintf(buffer, "%.*s+", 7, upasswd->pw_name);
    else
        sprintf(buffer, "%s", upasswd->pw_name);
}

/**
 * @brief Get terminal name
 * @param tty TTY number
 * @param buffer Buffer
 */
void get_terminal(uint16_t tty, char *buffer)
{
    char file_name[MAX_BUFFER_SIZE] = {0};
    struct stat statbuf;
    struct dirent **namelist;
    uint32_t file_count, i;

    // Terminal not exist
    if (tty == 0)
        sprintf(buffer, "?");

    // Search /dev/tty*
    file_count = scandir("/dev", &namelist, NULL, alphasort);
    for (i = 0; i < file_count; i++)
    {
        if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..") || strncmp(namelist[i]->d_name, "tty", 3))
            continue;

        sprintf(file_name, "/dev/%s", namelist[i]->d_name);
        stat(file_name, &statbuf);

        if (tty == statbuf.st_rdev)
        {
            sprintf(buffer, "%s", namelist[i]->d_name);
            for (i = 0; i < file_count; i++)
                free(namelist[i]);
            free(namelist[i]);
            return;
        }
    }

    // Search /dev/pts/*
    file_count = scandir("/dev/pts", &namelist, NULL, alphasort);
    for (i = 0; i < file_count; i++)
    { // "/dev/pts/*" 탐색
        if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, ".."))
            continue;

        sprintf(file_name, "/dev/pts/%s", namelist[i]->d_name);
        stat(file_name, &statbuf);

        if (tty == statbuf.st_rdev)
        {
            sprintf(buffer, "pts/%s", namelist[i]->d_name);
            for (i = 0; i < file_count; i++)
                free(namelist[i]);
            free(namelist[i]);
            return;
        }
    }

    for (i = 0; i < file_count; i++)
        free(namelist[i]);
    free(namelist[i]);
}

/**
 * @brief Get CPU usage to time format
 * @param time CPU time
 * @param buffer Buffer
 */
void get_time_format(uint64_t time, char *buffer)
{
    uint64_t sec;
    uint32_t hour, min;

    time /= sysconf(_SC_CLK_TCK);
    min = time / 60;
    hour = min / 60;
    sec = time % 60;

    if (min > 999)
        sprintf(buffer, "undefined");
    else if (!g_option_a && !g_option_u && !g_option_x)
        sprintf(buffer, "%02d:%02d:%02ld", hour, min % 60, sec);
    else
        sprintf(buffer, "%3d:%02ld", min, sec);
}

/**
 * @brief Get process start time to time format
 * @param start_time Start time
 * @param buffer Buffer
 */
void get_start_time(uint64_t start_time, char *buffer)
{
    time_t uptime, now;
    struct tm tm;

    uptime = get_up_time();
    time(&now);
    start_time /= sysconf(_SC_CLK_TCK);
    uptime -= start_time;
    uptime = now - uptime;
    tm = *localtime(&uptime);
    sprintf(buffer, "%02d:%02d", tm.tm_hour, tm.tm_min);
}

/**
 * @brief Get system up time
 * @return time_t Up time
 */
time_t get_up_time(void)
{
    FILE *fp;
    float time;

    if ((fp = fopen("/proc/uptime", "r")) == NULL)
    {
        fprintf(stderr, "fopen error for /proc/uptime\n");
        exit(1);
    }
    fscanf(fp, "%f", &time);
    fclose(fp);

    return time;
}

/**
 * @brief Get command string
 * @param pid Process id
 * @param buffer Buffer
 */
void get_command(uint32_t pid, char *buffer)
{
    char file_name[BUFFER_SIZE], tmp[BUFFER_SIZE];
    FILE *fp;
    uint16_t length, i;

    sprintf(file_name, "/proc/%d/cmdline", pid);

    if ((fp = fopen(file_name, "r")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", file_name);
        exit(1);
    }
    fp = fopen(file_name, "r");
    strcpy(tmp, buffer);
    fread(buffer, BUFFER_SIZE, 1, fp);
    length = ftell(fp);
    fclose(fp);

    // Empty
    if (length == 0)
        strcpy(buffer, tmp);
    else if (g_option_a || g_option_u || g_option_x)
        for (i = 0; i < length - 1; i++)
            if (buffer[i] == '\0')
                buffer[i] = ' ';
}

/**
 * @brief Sort process list ordered by pid
 * @param p Process list
 */
void sort_by_pid(process **p)
{
    uint32_t i, j;
    process *tmp;

    for (i = 0; i < g_process_count; i++)
        for (j = i; j < g_process_count; j++)
            if (p[i]->pid > p[j]->pid)
            {
                tmp = p[i];
                p[i] = p[j];
                p[j] = tmp;
            }
}

/**
 * @brief Print process list
 * @param p Process list
 */
void print_processes(process **p)
{
    uint32_t i = 0;
    char buffer[MAX_BUFFER_SIZE] = {0};

    if (g_option_u) // u옵션 시 출력
    {
        sprintf(buffer, "%-12s%4s %s %s%7s%6s %s      %s %s   %s %s",
                "USER", "PID", "\%CPU", "%MEM", "VSZ", "RSS", "TTY", "STAT", "START", "TIME", "COMMAND");
        printf("%s\n", buffer);

        while (i < g_process_count)
        {
            sprintf(buffer, "%-10s%6d %4.1f %4.1f %6d %5d %-8s %-4s %s %6s %s",
                    p[i]->user_name,
                    p[i]->pid,
                    p[i]->cpu,
                    p[i]->memory,
                    p[i]->vsz,
                    p[i]->rss,
                    p[i]->tty,
                    p[i]->stat,
                    p[i]->start,
                    p[i]->time,
                    p[i]->command);
            buffer[WINDOW_SIZE.ws_col] = '\0';
            printf("%s\n", buffer);
            i++;
        }
    }
    else
    {
        if (!g_option_a && !g_option_u && !g_option_x)
            sprintf(buffer, "%7s %-8s %8s %s", "PID", "TTY", "TIME", "CMD");
        else
            sprintf(buffer, "%7s %-8s %s %6s %s", "PID", "TTY", "STAT", "TIME", "COMMAND");
        printf("%s\n", buffer);

        while (i < g_process_count)
        {
            if (g_option_a || g_option_x)

                sprintf(buffer, "%7d %-8s %-4s %6s %s",
                        p[i]->pid,
                        p[i]->tty,
                        p[i]->stat,
                        p[i]->time,
                        p[i]->command);
            else
                sprintf(buffer, "%7d %-8s %6s %s",
                        p[i]->pid,
                        p[i]->tty,
                        p[i]->time,
                        p[i]->command);

            buffer[WINDOW_SIZE.ws_col] = '\0';
            printf("%s\n", buffer);
            i++;
        }
    }
}