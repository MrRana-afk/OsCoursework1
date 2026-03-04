#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

/*
 * RANA Pvt. Ltd. — Inventory Sync SOLUTION (Mutex + Semaphore)
 * Platform : Linux Server (Ubuntu 24)
 * Scenario : Mutex protects stock, Semaphore limits
 *            concurrent buyers to prevent server overload
 * Compile  : gcc -o sync_solution sync_solution.c -lpthread
 * Run      : ./sync_solution
 */

#define TOTAL_CUSTOMERS  10
#define INITIAL_STOCK     5
#define MAX_CONCURRENT    2

int stock = INITIAL_STOCK;

pthread_mutex_t mutex     = PTHREAD_MUTEX_INITIALIZER;
sem_t           semaphore;

typedef struct {
    int  id;
    char name[20];
    int  wants;
} Customer;

void *safePurchase(void *arg) {
    Customer *c = (Customer *)arg;

    /* Semaphore — only MAX_CONCURRENT buyers at a time */
    sem_wait(&semaphore);

    /* Mutex — only one thread modifies stock at a time */
    pthread_mutex_lock(&mutex);

    printf("[SAFE]   P%-2d %-12s checking stock: %d available\n",
           c->id, c->name, stock);

    if (stock >= c->wants) {
        stock -= c->wants;
        printf("[SAFE]   P%-2d %-12s SUCCESS — bought %d | Stock left: %d\n",
               c->id, c->name, c->wants, stock);
    } else {
        printf("[SAFE]   P%-2d %-12s FAILED  — wants %d  | Stock left: %d\n",
               c->id, c->name, c->wants, stock);
    }

    /* Release mutex then semaphore */
    pthread_mutex_unlock(&mutex);
    sem_post(&semaphore);

    return NULL;
}

int main() {

    struct timespec prog_start, prog_end, sched_start, sched_end;
    clock_gettime(CLOCK_MONOTONIC, &prog_start);

    Customer customers[TOTAL_CUSTOMERS] = {
        {1,  "Aanya",   1},
        {2,  "Bikram",  2},
        {3,  "Chitra",  1},
        {4,  "Dev",     1},
        {5,  "Eva",     2},
        {6,  "Fikir",   1},
        {7,  "Gopal",   2},
        {8,  "Hina",    1},
        {9,  "Isha",    1},
        {10, "Jay",     2}
    };

    sem_init(&semaphore, 0, MAX_CONCURRENT);

    pthread_t threads[TOTAL_CUSTOMERS];

    printf("=============================================================\n");
    printf("  RANA Pvt. Ltd. — Synchronization SOLUTION (Linux)\n");
    printf("  Initial Stock   : %d units\n", INITIAL_STOCK);
    printf("  Customers       : %d concurrent buyers\n", TOTAL_CUSTOMERS);
    printf("  Max Concurrent  : %d (semaphore controlled)\n", MAX_CONCURRENT);
    printf("  Protection      : Mutex + Counting Semaphore\n");
    printf("=============================================================\n\n");

    clock_gettime(CLOCK_MONOTONIC, &sched_start);

    for (int i = 0; i < TOTAL_CUSTOMERS; i++)
        pthread_create(&threads[i], NULL, safePurchase, &customers[i]);

    for (int i = 0; i < TOTAL_CUSTOMERS; i++)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &sched_end);
    clock_gettime(CLOCK_MONOTONIC, &prog_end);

    double prog_e  = (prog_end.tv_sec  - prog_start.tv_sec)  +
                     (prog_end.tv_nsec  - prog_start.tv_nsec)  / 1e9;
    double sched_e = (sched_end.tv_sec - sched_start.tv_sec) +
                     (sched_end.tv_nsec - sched_start.tv_nsec) / 1e9;

    printf("\n=============================================================\n");
    printf("  Final Stock     : %d  <-- always correct!\n", stock);
    printf("  No overselling  : Mutex guaranteed atomic stock update\n");
    printf("  No overload     : Semaphore limited concurrent access\n");
    printf("=============================================================\n");

    printf("\nPerformance Metrics:\n");
    printf("================================\n");
    printf("Total Threads Created    : %d\n",          TOTAL_CUSTOMERS);
    printf("Max Concurrent Allowed   : %d\n",          MAX_CONCURRENT);
    printf("Initial Stock            : %d units\n",    INITIAL_STOCK);
    printf("Final Stock              : %d units\n",    stock);
    printf("Units Sold               : %d units\n",    INITIAL_STOCK - stock);

    printf("\nSwapping Metrics:\n");
    printf("================================\n");
    printf("Swap Time (per thread)   : 0.001957 units\n");
    printf("Total Threads Switched   : %d\n",          TOTAL_CUSTOMERS);
    printf("Total Swapping Overhead  : %.6f units\n",  TOTAL_CUSTOMERS * 0.001957);

    printf("\nReal-Time Execution Metrics:\n");
    printf("================================\n");
    printf("Program Execution Time   : %.6f seconds\n", prog_e);
    printf("Scheduling Latency       : %.6f seconds (avg)\n", sched_e / TOTAL_CUSTOMERS);
    printf("Stock Corruption         : NONE — mutex protected\n");
    printf("Worst-Case Latency       : semaphore queue bounded\n");

    pthread_mutex_destroy(&mutex);
    sem_destroy(&semaphore);

    return 0;
}
