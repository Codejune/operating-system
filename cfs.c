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
	uint8_t i;
	uint32_t pid, pid_list[SLAVE_COUNT] = {0};
	struct timeval begin_t, end_t;

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
			printf("++ Child created, PID: %d\n", getpid());
			fibonacci(42);
			exit(EXIT_SUCCESS);
		}
		// Parent process (PID != 0)
		else
			pid_list[i] = pid;
	}

	for (i = 0; i < SLAVE_COUNT; i++)
        if ((pid = wait(&status)) > 1)
			printf("-- Child terminated, PID: %d\n", pid);
        
    
	gettimeofday(&end_t, NULL);
	print_runtime(&begin_t, &end_t);

	exit(EXIT_SUCCESS);
}

/**
 * @brief Fibonacci calculate function
 * @param n number
 * @return uint64_t result
 */
uint64_t fibonacci(uint64_t n)
{
	if (n <= 1)
		return n;
	else
		return fibonacci(n - 1) + fibonacci(n - 2);
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