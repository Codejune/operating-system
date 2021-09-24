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
    parse_option(argc, argv); // Option check
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
    while ((opt = getopt(argc, argv, "hd:p:")) != -1)
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