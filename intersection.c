/**
 * @file intersection.c
 * @brief 프로젝트에서 사용되는 함수 정의
 * @author 김병준 (kbj9704@gmail.com)
 */
#include "intersection.h"

/**
 * @brief Main entry function
 * @return int OK 0, ERROR 1
 */
int main(void)
{
    pthread_t way_tid[MAX_WAY_COUNT][2];
    pthread_t intrsect_tid;
    uint8_t i, ret;

    // Initialize intersection
    init_intrsect();

    // Initialize vehicle list
    init_vhcle_q();

    // Initialize way queue
    init_way_q();

    // Generate way thread
    for (i = 0; i < MAX_WAY_COUNT; i++)
    {
        way_tid[i][1] = i + 1;
        if ((pthread_create(&(way_tid[i][0]), NULL, &t_way, (void *)&(way_tid[i][1]))) != 0)
        {
            fprintf(stderr, "pthread_create() error\n");
            exit(EXIT_FAILURE);
        }
    }

    // Generate intersection thread
    if ((ret = pthread_create(&intrsect_tid, NULL, &t_intrsect, NULL)) != 0)
    {
        fprintf(stderr, "pthread_create() error\n");
        exit(EXIT_FAILURE);
    }

    // Wait for traffic is done
    if ((ret = pthread_join(intrsect_tid, NULL)) != 0)
    {
        fprintf(stderr, "pthread_join() error\n");
        exit(EXIT_FAILURE);
    }

    // Destroy all way thread
    for (i = 0; i < MAX_WAY_COUNT; i++)
        if ((ret = pthread_cancel(way_tid[i][0])) != 0)
        {
            fprintf(stderr, "pthread_cancel() error\n");
            exit(EXIT_FAILURE);
        }

    // De-allocation all way queue
    free(g_way_q);

    exit(EXIT_SUCCESS);
}

/**
 * @brief Initialize intersection
 */
void init_intrsect(void)
{
    g_intrsect.traffic_type = TRAFFIC_TYPE_NOT_RUNNING;
    memset(g_intrsect.is_running, false, sizeof(g_intrsect.is_running));
}

/**
 * @brief Initialize vehicle number and list
 */
void init_vhcle_q(void)
{
    uint8_t i;
#ifdef DEBUG
    uint8_t test_data[10] = {4, 4, 3, 4, 1, 2, 2, 3, 3, 3};
    g_vhcle_cnt = 10;
#else
    uint8_t way;

    // Get vehicle count
    printf("Total number of vehicles: ");
    scanf("%hhd", &g_vhcle_cnt);

    if (g_vhcle_cnt > MAX_VHCLE_COUNT)
    {
        fprintf(stderr, "Out of maximum vehicle count\n");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
#endif

    // Generate vehicle list and print
    printf("Start point: ");
    for (i = 0; i < g_vhcle_cnt; i++)
    {
#ifdef DEBUG
        printf("%d ", test_data[i]);
        q_put(&g_vhcle_q, test_data[i]);
#else
        way = rand() % MAX_WAY_COUNT + 1;
        printf("%d ", way);
        q_put(&g_vhcle_q, way);
#endif
    }
    printf("\n");
}

/**
 * @brief Initialize way queue
 */
void init_way_q(void)
{
    uint8_t i;

    g_way_q = (queue_t *)malloc(sizeof(queue_t) * g_vhcle_cnt);

    for (i = 0; i < g_vhcle_cnt; i++)
    {
        g_way_q[i].cnt = 0;
        g_way_q[i].front = 0;
        g_way_q[i].rear = 0;
    }
}

/**
 * @brief Get boolean of queue is empty
 * @param q Target queue
 * @return true Queue is empty, false Queue is not empty
 */
bool q_is_empty(queue_t *q)
{
    return q->cnt == 0 ? true : false;
}

/**
 * @brief Get boolean of queue is full
 * @param q Target queue
 * @return true Queue is full, false Queue is not full
 */
bool q_is_full(queue_t *q)
{
    return q->cnt == MAX_QUEUE_SIZE ? true : false;
}

/**
 * @brief Put data into queue
 * @param q Target queue
 * @param data Way number
 */
void q_put(queue_t *q, uint8_t data)
{
    q->data[q->front++] = data;
    q->front %= MAX_QUEUE_SIZE;
    q->cnt++;
}

/**
 * @brief Get data from queue
 * @param q Target queue
 * @return uint8_t Way number
 */
uint8_t q_get(queue_t *q)
{
    uint8_t data;
    data = q->data[q->rear++];
    q->rear %= MAX_QUEUE_SIZE;
    q->cnt--;
    return data;
}

void *t_intrsect(void *arg)
{
    uint8_t tick, way, passed, i;

    tick = 1;

    // Traffic signal routine
    while (true)
    {
        passed = 0;
        printf("tick: %hhd\n", tick);

        // Get vehicle from queue and put it on the way
        if (!q_is_empty(&g_vhcle_q))
        {
            way = q_get(&g_vhcle_q);
            q_put(&g_way_q[way - 1], way);
        }

        // Send signal to way which is send vehicle to the intersection
        pthread_cond_broadcast(&g_tf_cond);

        // Wait for signal from way which is receive vehicle from the intersection
        pthread_cond_wait(&g_w_cond, &g_mutex);

        // Check vehicle is passed
        for (i = 0; i < 2; i++)
        {
            if (g_intrsect.is_running[i] && --g_intrsect.passing[i][1] == 0)
            {
                passed = g_intrsect.passing[i][0];
                g_intrsect.is_running[i] = false;
            }
        }

        if (!g_intrsect.is_running[0] && g_intrsect.is_running[1])
            g_intrsect.traffic_type = TRAFFIC_TYPE_NOT_RUNNING;

        printf("==========================\n");
        printf("Passed Vehicle\n");
        printf("Car ");
        if (passed > 0)
            printf("%hhd\n", passed);
        else
            printf("\n");
        printf("Waiting Vehicle\n");
        printf("Car ");
        printf("\n==========================\n");

        pthread_mutex_unlock(&g_mutex);
        tick++;
    }
}

/**
 * @brief Way thread function, which is send vehicle to the intersection
 * @param arg Way number
 * @return void* NULL
 */
void *t_way(void *arg)
{
    uint8_t way;

    way = *((int *)arg);

    while (true)
    {
        pthread_cond_wait(&g_tf_cond, &g_mutex);

        if (q_is_empty(&g_way_q[way]))
        {
            pthread_mutex_unlock(&g_mutex);
            continue;
        }

        // Set vehicle to passing status
        // Traffic type is not running
        if (g_intrsect.traffic_type == TRAFFIC_TYPE_NOT_RUNNING)
        {
            g_intrsect.traffic_type = way % 2;
            g_intrsect.passing[0][0] = q_get(&g_way_q[way]);
            g_intrsect.passing[0][1] = 2;
            g_intrsect.is_running[0] = true;
        }
        // Traffic type is matched
        else if (g_intrsect.traffic_type == way % 2)
        {
            // Road is not full and there is no same start way
            if (g_intrsect.is_running[0] && g_intrsect.passing[0][0] != way && !g_intrsect.is_running[1])
            {
                g_intrsect.passing[1][0] = q_get(&g_way_q[way]);
                g_intrsect.passing[1][1] = 2;
                g_intrsect.is_running[1] = true;
            }
            else if (g_intrsect.is_running[1] && g_intrsect.passing[1][0] != way && !g_intrsect.is_running[0])
            {
                g_intrsect.passing[0][0] = q_get(&g_way_q[way]);
                g_intrsect.passing[0][1] = 2;
                g_intrsect.is_running[0] = true;
            }
        }

        // Raise signal to intersection that the vehicle is passing
        pthread_cond_signal(&g_w_cond);

        // Unlock pthread mutex
        pthread_mutex_unlock(&g_mutex);
    }
}