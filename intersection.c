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

    printf("Number of vehicles passed from each start point\n");
    for (i = 0; i < MAX_WAY_COUNT; i++)
        printf("P%d: %d times\n", i + 1, g_passed_vhcle[i]);
    printf("Total time: %hhd ticks\n", g_total_ticks);

    // Destroy all way thread
    for (i = 0; i < MAX_WAY_COUNT; i++)
        if ((ret = pthread_cancel(way_tid[i][0])) != 0)
        {
            fprintf(stderr, "pthread_cancel() error\n");
            exit(EXIT_FAILURE);
        }

    // De-allocation
    free(g_way_q);
    pthread_mutex_destroy(&g_mutex);
    pthread_cond_destroy(&g_tf_cond);

    exit(EXIT_SUCCESS);
}

/**
 * @brief Initialize intersection
 */
void init_intrsect(void)
{
    g_intrsect.traffic_type = TRAFFIC_NO_RUNNING;
    memset(g_intrsect.is_running, false, 2 * sizeof(bool));
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
        q_enq(&g_vhcle_q, test_data[i]);
#else
        way = rand() % MAX_WAY_COUNT + 1;
        printf("%d ", way);
        q_enq(&g_vhcle_q, way);
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
        g_way_q[i].front = g_way_q[i].rear = 0;
}

/**
 * @brief Get boolean of queue is empty
 * @param q Target queue
 * @return true Queue is empty, false Queue is not empty
 */
bool q_is_empty(queue_t *q)
{
    return (q->front == q->rear) ? true : false;
}

/**
 * @brief Get boolean of queue is full
 * @param q Target queue
 * @return true Queue is full, false Queue is not full
 */
bool q_is_full(queue_t *q)
{
    return ((q->rear + 1) % MAX_QUEUE_SIZE == q->front) ? true : false;
}

/**
 * @brief Put data into queue
 * @param q Target queue
 * @param data Way number
 */
void q_enq(queue_t *q, uint8_t data)
{
    if (q_is_full(q))
    {
        fprintf(stderr, "Queue is full\n");
        exit(EXIT_FAILURE);
    }
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    q->data[q->rear] = data;
}

/**
 * @brief Get data from queue
 * @param q Target queue
 * @return uint8_t Way number
 */
uint8_t q_deq(queue_t *q)
{
    if (q_is_empty(q))
    {
        fprintf(stderr, "Queue is empty\n");
        exit(EXIT_FAILURE);
    }
    q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    return q->data[q->front];
}

/**
 * @brief Print queue
 */
void q_print(void)
{
    uint8_t i, j;

    for (i = 0; i < MAX_WAY_COUNT; i++)
    {
        printf("Way queue[%hhd]: ", i + 1);
        if (!q_is_empty(&g_way_q[i]))
        {
            j = g_way_q[i].front;
            do
            {
                j = (j + 1) % MAX_QUEUE_SIZE;
                printf("%hhd ", g_way_q[i].data[j]);
            } while (j == g_way_q[i].rear - 1);
        }
        printf("\n");
    }
}

/**
 * @brief Intersection thread function which is managing traffic
 * @return void* NULL
 */
void *t_intrsect(void *arg)
{
    uint8_t way, passed, i, j;
    bool is_not_checked, is_all_q_empty;

    // Traffic signal routine
    while (true)
    {
        // Initialize status variables
        is_all_q_empty = true;
        passed = 0;
        memset(g_intrsect.is_way_checked, false, MAX_WAY_COUNT * sizeof(bool));

        // Get vehicle from queue and put it on the way
        pthread_mutex_lock(&g_mutex);
        if (!q_is_empty(&g_vhcle_q))
        {
            way = q_deq(&g_vhcle_q);
            q_enq(&g_way_q[way - 1], way);
        }
        pthread_mutex_unlock(&g_mutex);

        // Send signal to way which is send vehicle to the intersection
        pthread_cond_broadcast(&g_tf_cond);

        // Wait for all way thread is checked
        is_not_checked = false;
        while (!is_not_checked)
        {
            for (i = 0; i < MAX_WAY_COUNT; i++)
                if (g_intrsect.is_way_checked[i] == false)
                {
                    is_not_checked = true;
                    break;
                }
            usleep(1 * SECOND_TO_MICRO);
        }

        // Check vehicle is passed
        for (i = 0; i < 2; i++)
        {
            if (g_intrsect.is_running[i])
            {
                g_intrsect.passing[i][1]--;
                if (g_intrsect.passing[i][1] == 0)
                {
                    passed = g_intrsect.passing[i][0];
                    g_intrsect.passing[i][0] = 0;
                    g_intrsect.is_running[i] = false;
                    g_passed_vhcle[passed - 1]++;
                }
            }
        }

        if (!g_intrsect.is_running[0] && !g_intrsect.is_running[1])
            g_intrsect.traffic_type = TRAFFIC_NO_RUNNING;

        // Print tick information
        printf("tick: %hhd\n==========================\nPassed Vehicle\nCar ", ++g_total_ticks);
        if (passed > 0)
            printf("%hhd", passed);
        printf("\nWaiting Vehicle\nCar ");
        for (i = 0; i < MAX_WAY_COUNT; i++)
            if (!q_is_empty(&g_way_q[i]))
            {
                j = g_way_q[i].front;
                do
                {
                    j = (j + 1) % MAX_QUEUE_SIZE;
                    printf("%hhd ", g_way_q[i].data[j]);
                } while (j == g_way_q[i].rear - 1);
            }
        printf("\n==========================\n");

        // Loop exit trigger
        if (is_finished())
            break;

        // Last check vehicle is passed
        // for (i = 0; i < 2; i++)
        // {
        //     if (g_intrsect.is_running[i])
        //     {
        //         g_intrsect.passing[i][1]--;
        //         if (g_intrsect.passing[i][1] == 0)
        //         {
        //             passed = g_intrsect.passing[i][0];
        //             g_intrsect.passing[i][0] = 0;
        //             g_intrsect.is_running[i] = false;
        //             g_passed_vhcle[passed - 1]++;
        //         }
        //     }
        // }
        // break;
    }
    return NULL;
}

/**
 * @brief Way thread function, which is send vehicle to the intersection
 * @param arg Way number
 * @return void* NULL
 */
void *t_way(void *arg)
{
    uint8_t way = *((int *)arg);

    while (true)
    {
        pthread_cond_wait(&g_tf_cond, &g_mutex);

        // First, Check is vehicle is waiting
        if (!q_is_empty(&g_way_q[way - 1]))
        {
            // Second, Check is vehicle is already running on this way
            if (g_intrsect.passing[0][0] == way || g_intrsect.passing[1][0] == way)
            {
                g_intrsect.is_way_checked[way - 1] = true;
                pthread_mutex_unlock(&g_mutex);
                continue;
            }

            // Set vehicle to passing status
            // Traffic type is not running
            if (g_intrsect.traffic_type == TRAFFIC_NO_RUNNING)
            {
                g_intrsect.traffic_type = way % 2;
                g_intrsect.passing[0][0] = q_deq(&g_way_q[way - 1]);
                g_intrsect.passing[0][1] = 2;
                g_intrsect.is_running[0] = true;
            }
            // Traffic type is matched
            else if (g_intrsect.traffic_type == (way % 2))
            {
                // Road is not full and there is no same start way
                if (!g_intrsect.is_running[0])
                {
                    g_intrsect.traffic_type = way % 2;
                    g_intrsect.passing[0][0] = q_deq(&g_way_q[way - 1]);
                    g_intrsect.passing[0][1] = 2;
                    g_intrsect.is_running[0] = true;
                }
                else
                {
                    g_intrsect.traffic_type = way % 2;
                    g_intrsect.passing[1][0] = q_deq(&g_way_q[way - 1]);
                    g_intrsect.passing[1][1] = 2;
                    g_intrsect.is_running[1] = true;
                }
            }
        }

        g_intrsect.is_way_checked[way - 1] = true;

        // Unlock pthread mutex
        pthread_mutex_unlock(&g_mutex);
    }
    return NULL;
}

/**
 * @brief Check is process finished
 * @return true Finished
 * @return false Not finished
 */
bool is_finished(void)
{
    uint8_t i;

    // Check intersection is clear
    if (g_intrsect.traffic_type != TRAFFIC_NO_RUNNING)
        return false;

    // Check all vehicle is ready
    if (!q_is_empty(&g_vhcle_q))
        return false;

    // Check all vehicle is passed
    for (i = 0; i < MAX_WAY_COUNT; i++)
        if (!q_is_empty(&g_way_q[i]))
            return false;

    return true;
}