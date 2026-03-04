#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

/*
 * RANA Pvt. Ltd. — Inventory Race Condition (PROBLEM)
 * Platform : Linux Server (Ubuntu 24)
 * Scenario : Multiple customers buying last stock simultaneously
 *            WITHOUT any synchronization — shows race condition
 * Compile  : gcc -o sync_problem sync_problem.c -lpthread
 * Run      : ./sync_problem
 */

#define TOTAL_CUSTOMERS  10
#define INITIAL_STOCK     5

int stock = INITIAL_STOCK;

typedef struct {
    int  id;
    char name[20];
    int  wants;
} Customer;

void *unsafePurchase(void *arg) {
    Customer *c = (Customer *)arg;

    /* No lock — multiple threads read same stock value */
    int seen = stock;

    /* Simulate processing delay — widens the race window */
    usleep(10000);

    if (seen >= c->wants) {
        stock -= c->wants;
        printf("[UNSAFE] P%-2d %-12s wants %d — bought  %d | Stock left: %d\n",
               c->id, c->name, c->wants, c->wants, stock);
    } else {
        printf("[UNSAFE] P%-2d %-12s wants %d — FAILED  | Stock left: %d\n",
               c->id, c->name, c->wants, stock);
    }

    return NULL;
}

int main() {

    struct timespec prog_start, prog_end;
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

    pthread_t threads[TOTAL_CUSTOMERS];

    printf("=============================================================\n");
    printf("  RANA Pvt. Ltd. — Synchronization PROBLEM Demo (Linux)\n");
    printf("  Initial Stock : %d units\n", INITIAL_STOCK);
    printf("  Customers     : %d concurrent buyers\n", TOTAL_CUSTOMERS);
    printf("  Issue         : No lock — race condition possible\n");
    printf("=============================================================\n\n");

    for (int i = 0; i < TOTAL_CUSTOMERS; i++)
        pthread_create(&threads[i], NULL, unsafePurchase, &customers[i]);

    for (int i = 0; i < TOTAL_CUSTOMERS; i++)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &prog_end);
    double prog_e = (prog_end.tv_sec  - prog_start.tv_sec)  +
                    (prog_end.tv_nsec  - prog_start.tv_nsec)  / 1e9;

    printf("\n=============================================================\n");
    printf("  Final Stock   : %d  <-- may be NEGATIVE or WRONG!\n", stock);
    printf("  Race Condition: Two threads read same stock simultaneously\n");
    printf("                  Both think stock is available and both buy\n");
    printf("                  This causes overselling at RANA's platform\n");
    printf("=============================================================\n");
    printf("\nExecution Metrics:\n");
    printf("================================\n");
    printf("Program Execution Time   : %.6f seconds\n", prog_e);
    printf("Total Threads Created    : %d\n", TOTAL_CUSTOMERS);
    printf("Initial Stock            : %d units\n", INITIAL_STOCK);
    printf("Final Stock              : %d units\n", stock);
    printf("Stock Corruption         : %s\n", stock < 0 ? "YES — oversold!" : "Possible");

    return 0;
}
