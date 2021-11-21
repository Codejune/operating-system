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
#define MAX_VHCLE_COUNT 16             // Maximal vehicle count
#define MAX_WAY_COUNT 4                // Maximal way count
#define MAX_QUEUE_SIZE MAX_VHCLE_COUNT // Maximal queue size
#define DEBUG 1                        // Debug mode
// Structure
/**
 * @brief Traffic type enumeration
 */
typedef enum traffic_t
{
    TRAFFIC_HORIZONTAL = 0, // P2, P4
    TRAFFIC_VERTICAL = 1,   // P1, P3
    TRAFFIC_NO_RUNNING = 2, // Not running
} traffic_t;

/**
 * @brief Cycle queue structure
 */
typedef struct queue_t
{
    uint8_t data[MAX_VHCLE_COUNT]; // Queue data
    uint8_t front;                 // Head index
    uint8_t rear;                  // Tail index
} queue_t;

/**
 * @brief Intersection structure
 */
typedef struct intrsect_t
{
    uint8_t passing[2][2];              // Passing road, left time of passing
    enum traffic_t traffic_type;        // Traffic type
    bool is_running[2];                 // Way status
    bool is_way_checked[MAX_WAY_COUNT]; // Way checked status
} intrsect_t;

// Global Variable
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex lock variable
pthread_cond_t g_tf_cond = PTHREAD_COND_INITIALIZER; // Traffic Signal condition variable
uint8_t g_vhcle_cnt;                                 // Vehicle count
queue_t g_vhcle_q;                                   // Vehicle queue
queue_t *g_way_q;                                    // Way queue list
intrsect_t g_intrsect;                               // Intersection structure
uint8_t g_passed_vhcle[MAX_WAY_COUNT] = {0};         // Passed vehicle
uint8_t g_total_ticks = 0;

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
void *t_way(void *arg);
bool is_finished(void);
void print_intrsect(uint8_t passed);
#endif