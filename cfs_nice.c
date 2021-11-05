/**
 * @file schedule.c
 * @author 김병준 (kbj9704@gmail.com)
 * @brief schedule 명령어 기능 함수 구현
 */
#include "common.h"

/**
 * @brief Main entry function
 * @param argc Argument count
 * @param argv Argument verse
 * @return int Ok 0, Error 1
 */
int main(int argc, char *argv[])
{
    int status;
    uint8_t i, j;
    uint32_t pid, pid_list[SLAVE_COUNT] = {0};
    struct timeval begin_t, end_t;
    char buf[512] = {0};

    printf("Parent running,  PID: %d\n", getpid());
    gettimeofday(&begin_t, NULL);
    for (i = 0; i < SLAVE_COUNT; i++)
    {
        // Process fork
        if ((pid = fork()) == (uint32_t)-1)
        {
            // Occured error: EAGAIN, ENOMEM
            perror("Failed to fork");
            exit(EXIT_FAILURE);
        }

        // Child process (PID = 0)
        if (pid == 0)
        {
            if (i < 7)
            {
                nice(NICE_HIGH);
                printf("++ NICE(%3d) Child created, PID: %d\n", NICE_HIGH, getpid());
                product(500);
            }
            else if (i < 14)
            {
                nice(NICE_MID);
                printf("++ NICE(%3d) Child created, PID: %d\n", NICE_MID, getpid());
                product(600);
            }
            else
            {
                nice(NICE_LOW);
                printf("++ NICE(%3d) Child created, PID: %d\n", NICE_LOW, getpid());
                product(700);
            }
            // sprintf(buf, "chrt -p %d", getpid());
            // system(buf);
            exit(EXIT_SUCCESS);
        }
        // Parent process (PID != 0)
        else
            pid_list[i] = pid;
    }

    for (i = 0; i < SLAVE_COUNT; i++)
    {
        if ((pid = wait(&status)) > 1)
        {
            for (j = 0; j < SLAVE_COUNT; j++)
            {
                if (pid_list[j] == pid)
                {
                    if (j < 7)
                        printf("-- NICE(%3d) Child terminated, PID: %d\n", NICE_HIGH, pid_list[j]);
                    else if (j < 14)
                        printf("-- NICE(%3d) Child terminated, PID: %d\n", NICE_MID, pid_list[j]);
                    else
                        printf("-- NICE(%3d) Child terminated, PID: %d\n", NICE_LOW, pid_list[j]);
                }
            }
        }
    }
    gettimeofday(&end_t, NULL);
    print_runtime(&begin_t, &end_t);

    exit(EXIT_SUCCESS);
}

void product(uint64_t n)
{
    uint32_t i, j, k;
    uint64_t sum = 0;

    for(i = 1; i <= n; i++)
        for(j = 1; j <= n; j++)
            for(k = 1; k <= n; k++)
                sum+= i * j * k;
}
/**
 * @brief Print running time
 * @param begin_t start time
 * @param end_t end time
 */
void print_runtime(struct timeval *begin_t, struct timeval *end_t)
{
    end_t->tv_sec -= begin_t->tv_sec;

    if (end_t->tv_usec < begin_t->tv_usec)
    {
        end_t->tv_sec--;
        end_t->tv_usec += SECOND_TO_MICRO;
    }

    end_t->tv_usec -= begin_t->tv_usec;
    printf("Runtime: %ld:%06ld(sec:usec)\n", end_t->tv_sec, end_t->tv_usec);
}
