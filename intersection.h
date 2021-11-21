/**
 * @file intersection.h
 * @author 김병준 (kbj9704@gmail.com)
 * @brief 프로그램 구현에 필요한 공통 헤더, 함수 및 매크로 정의
 */
#ifndef INTERSECTION_H
#define INTERSECTION_H

// header
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

// Macro
#define SECOND_TO_MICRO 1000000
#define MAX_VHCLE_COUNT 50             // Maximal vehicle count
#define MAX_WAY_COUNT 4                // Maximal way count
#define MAX_QUEUE_SIZE MAX_VHCLE_COUNT // Maximal queue size
#define DEBUG 1                        // Debug mode

// Structure
/**
 * @brief Traffic type enumeration
 */
typedef enum directon_t
{
    DIRECTION_HORIZONTAL = 0, // P2, P4
    DIRECTION_VERTICAL = 1,   // P1, P3
    DIRECTION_EMPTY = 2,      // Not running
} directon_t;

/**
 * @brief Cycle queue structure
 */
typedef struct queue_t
{
    uint8_t data[MAX_QUEUE_SIZE]; // Queue data
    uint8_t front;                 // Head index
    uint8_t rear;                  // Tail index
} queue_t;

/**
 * @brief Intersection structure
 */
typedef struct intrsect_t
{
    uint8_t passing[2][2];             // [Passing road][Left time]
    enum directon_t direction;         // Traffic direction
    bool is_direct_changed;            // Direction changed
    bool is_way_running[2];            // Way status
    bool is_way_finish[MAX_WAY_COUNT]; // Way thread checked status
} intrsect_t;

// Global Variable
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex lock variable
pthread_cond_t g_tf_cond = PTHREAD_COND_INITIALIZER; // Traffic Signal condition variable
uint8_t g_vhcle_cnt;                                 // Vehicle count
queue_t g_vhcle_q;                                   // Vehicle queue
queue_t *g_way_q;                                    // Way queue list
intrsect_t g_intrsect;                               // Intersection structure
uint8_t g_total_ticks = 0;                           // Tick count

// Function prototype
void init_intrsect(void);
void init_vhcle_q(void);
void init_way_q(void);
bool q_is_empty(queue_t *q);
bool q_is_full(queue_t *q);
void q_enq(queue_t *q, uint8_t data);
uint8_t q_deq(queue_t *q);
void q_print(void);
void *t_intrsect(void *arg);
void set_vhcle_ready(void);
void wait_ways_finish(void);
void *t_way(void *arg);
bool is_intrsect_finish(void);
bool is_vhcle_running(uint8_t way);
void print_intrsect(uint8_t passed_vhcle);
#endif