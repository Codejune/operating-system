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
#include <time.h>
#include <pthread.h>

// Macro
#define MAX_VHCLE_COUNT 15                 // Maximal vehicle count
#define MAX_WAY_COUNT 4                    // Maximal way count
#define MAX_QUEUE_SIZE MAX_VHCLE_COUNT - 1 // Maximal queue size
#define DEBUG 1                            // Debug mode
// Structure
/**
 * @brief Traffic type enumeration
 */
typedef enum traffic_t
{
    TRAFFIC_TYPE_HORIZONTAL = 0, // P1, P3
    TRAFFIC_TYPE_VERTICAL,       // P2, P4
    TRAFFIC_TYPE_NOT_RUNNING,    // Not running
} traffic_t;

/**
 * @brief Cycle queue structure
 */
typedef struct queue_t
{
    uint8_t data[MAX_VHCLE_COUNT]; // Queue data
    uint8_t items;                 // Number of items
    uint8_t head;                  // Head index
    uint8_t tail;                  // Tail index
} queue_t;

/**
 * @brief Intersection structure
 */
typedef struct intrsect_t
{
    uint8_t passing[2][2];       // Passing road, left time of passing
    enum traffic_t traffic_type; // Traffic type
    bool is_running[2];          // Way status
} intrsect_t;

// Global Variable
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex lock variable
pthread_cond_t g_tf_cond = PTHREAD_COND_INITIALIZER; // Traffic Signal condition variable
pthread_cond_t g_w_cond = PTHREAD_COND_INITIALIZER;  // Way condition variable
uint8_t g_vhcle_cnt;                                 // Vehicle count
queue_t g_vhcle_q;                                   // Vehicle queue
queue_t *g_way_q;                                    // Way queue list
intrsect_t g_intrsect;                               // Intersection structure
uint8_t g_passed_vhcle[MAX_WAY_COUNT] = {0};         // Passed vehicle

// Function prototype
void init_intrsect(void);
void init_vhcle_q(void);
void init_way_q(void);
bool q_is_empty(queue_t q);
bool q_is_full(queue_t q);
void q_put(queue_t q, uint8_t data);
uint8_t q_get(queue_t q);
void *t_intrsect(void *arg);
void *t_way(void *arg);
#endif