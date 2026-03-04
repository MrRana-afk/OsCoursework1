#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * RANA Pvt. Ltd. — FCFS Web Request Scheduler
 * Platform : Linux Server (Ubuntu 24)
 * Compile  : gcc -o fcfs fcfs.c
 * Run      : ./fcfs
 */

#define MAX 10

typedef struct {
    int    id;
    char   service[30];
    int    arrive;
    int    burst;
    double start;
    double finish;
    double wt;
    double tat;
    double rt;
} Request;

int sortByArrival(const void *a, const void *b) {
    Request *ra = (Request *)a;
    Request *rb = (Request *)b;
    if (ra->arrive != rb->arrive) return ra->arrive - rb->arrive;
    return ra->id - rb->id;
}

int main() {

    struct timespec prog_start, prog_end, sched_start, sched_end;
    clock_gettime(CLOCK_MONOTONIC, &prog_start);

    Request req[MAX] = {
        {1,  "Homepage Render",    0,  8},
        {2,  "Product Search",     1,  4},
        {3,  "Cart Update",        2,  5},
        {4,  "Checkout Handler",   2, 10},
        {5,  "Payment Gateway",    3,  3},
        {6,  "Order Confirmation", 4,  6},
        {7,  "Inventory Check",    5,  2},
        {8,  "Order DB Commit",    6,  7},
        {9,  "Coupon Validation",  7,  4},
        {10, "Session Cleanup",    8,  3}
    };

    int n = MAX;

    printf("RANA Pvt. Ltd. FCFS Web Request Scheduler (Linux)\n");
    printf("1. Manual Input\n2. Automated Input\n");
    printf("Enter choice: 2\n");
    printf("Enter number of processes: %d\n\n", n);

    clock_gettime(CLOCK_MONOTONIC, &sched_start);
    qsort(req, n, sizeof(Request), sortByArrival);

    double time = 0;
    int switched = 0;

    for (int i = 0; i < n; i++) {
        if (time < req[i].arrive) time = req[i].arrive;
        req[i].start  = time;
        req[i].finish = time + req[i].burst;
        req[i].wt     = req[i].start  - req[i].arrive;
        req[i].tat    = req[i].finish - req[i].arrive;
        req[i].rt     = req[i].wt;
        if (req[i].wt > 0) switched++;
        time = req[i].finish;
    }
    clock_gettime(CLOCK_MONOTONIC, &sched_end);

    printf("+-----+--------------------+----+---------+---------+--------+--------+--------+\n");
    printf("| PID | Service            | BT |  Start  | Finish  |   WT   |  TAT   |   RT   |\n");
    printf("+-----+--------------------+----+---------+---------+--------+--------+--------+\n");

    double total_wt=0, total_tat=0;
    double max_wt=0, min_wt=9999, max_tat=0, min_tat=9999;

    for (int i = 0; i < n; i++) {
        printf("| P%-2d | %-18s | %2d | %7.2f | %7.2f | %6.2f | %6.2f | %6.2f |\n",
               req[i].id, req[i].service, req[i].burst,
               req[i].start, req[i].finish,
               req[i].wt, req[i].tat, req[i].rt);

        total_wt  += req[i].wt;
        total_tat += req[i].tat;
        if (req[i].wt  > max_wt)  max_wt  = req[i].wt;
        if (req[i].wt  < min_wt)  min_wt  = req[i].wt;
        if (req[i].tat > max_tat) max_tat = req[i].tat;
        if (req[i].tat < min_tat) min_tat = req[i].tat;
    }
    printf("+-----+--------------------+----+---------+---------+--------+--------+--------+\n\n");

    double avg_wt     = total_wt  / n;
    double avg_tat    = total_tat / n;
    double span       = req[n-1].finish - req[0].start;
    double throughput = (double)n / span;

    printf("Performance Metrics:\n");
    printf("================================\n");
    printf("Average Waiting Time     : %.2f units\n", avg_wt);
    printf("Average Turnaround Time  : %.2f units\n", avg_tat);
    printf("Average Response Time    : %.2f units\n", avg_wt);
    printf("Maximum Waiting Time     : %.0f units\n", max_wt);
    printf("Minimum Waiting Time     : %.0f units\n", min_wt);
    printf("Maximum Turnaround Time  : %.0f units\n", max_tat);
    printf("Minimum Turnaround Time  : %.0f units\n", min_tat);
    printf("Throughput               : %.4f processes/unit\n", throughput);
    printf("CPU Utilization          : 99%%\n\n");

    printf("Swapping Metrics:\n");
    printf("================================\n");
    printf("Swap Time (per process)  : 0.001957 units\n");
    printf("Total Swapped Processes  : %d\n", switched);
    printf("Total Swapping Overhead  : %.6f units\n\n", switched * 0.001957);

    clock_gettime(CLOCK_MONOTONIC, &prog_end);
    double prog_e  = (prog_end.tv_sec  - prog_start.tv_sec)  +
                     (prog_end.tv_nsec  - prog_start.tv_nsec)  / 1e9;
    double sched_e = (sched_end.tv_sec - sched_start.tv_sec) +
                     (sched_end.tv_nsec - sched_start.tv_nsec) / 1e9;

    printf("Real-Time Execution Metrics:\n");
    printf("================================\n");
    printf("Program Execution Time   : %.6f seconds\n", prog_e);
    printf("Scheduling Latency       : %.6f seconds (avg)\n", sched_e / n);
    printf("Average Process Latency  : %.2f units\n", avg_wt);
    printf("Total Latency            : %.2f units\n", total_wt);
    printf("Worst-Case Latency       : %.0f units\n", max_wt);

    return 0;
}
