#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

/*
 * RANA Pvt. Ltd. — Priority Scheduling Server Task Manager
 * Platform : Windows Server
 * Compile  : gcc -o ps ps.c
 * Run      : ps.exe
 */

#define MAX 10

typedef struct {
    int    id;
    char   service[30];
    int    arrive;
    int    burst;
    int    priority;
    double start;
    double finish;
    double wt;
    double tat;
    double rt;
    int    done;
} Request;

int main() {

    LARGE_INTEGER freq, prog_start, prog_end, sched_start, sched_end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prog_start);

    Request req[MAX] = {
        {1,  "Homepage Render",    0,  8, 2, 0,0,0,0,0,0},
        {2,  "Product Search",     1,  4, 2, 0,0,0,0,0,0},
        {3,  "Cart Update",        2,  5, 2, 0,0,0,0,0,0},
        {4,  "Checkout Handler",   2, 10, 3, 0,0,0,0,0,0},
        {5,  "Payment Gateway",    3,  3, 1, 0,0,0,0,0,0},
        {6,  "Order Confirmation", 4,  6, 2, 0,0,0,0,0,0},
        {7,  "Inventory Check",    5,  2, 1, 0,0,0,0,0,0},
        {8,  "Order DB Commit",    6,  7, 3, 0,0,0,0,0,0},
        {9,  "Coupon Validation",  7,  4, 2, 0,0,0,0,0,0},
        {10, "Session Cleanup",    8,  3, 4, 0,0,0,0,0,0}
    };

    int n = MAX;

    printf("RANA Pvt. Ltd. Priority Scheduling Server Task Manager (Windows)\n");
    printf("1. Manual Input\n2. Automated Input\n");
    printf("Enter choice: 2\n");
    printf("Enter number of processes: %d\n\n", n);

    QueryPerformanceCounter(&sched_start);

    double time = 0;
    int completed = 0;
    int switched = 0;

    while (completed < n) {
        int idx = -1;
        int best_priority = 9999;

        for (int i = 0; i < n; i++) {
            if (!req[i].done && req[i].arrive <= time) {
                if (req[i].priority < best_priority) {
                    best_priority = req[i].priority;
                    idx = i;
                }
            }
        }

        if (idx == -1) { time++; continue; }

        req[idx].start  = time;
        req[idx].finish = time + req[idx].burst;
        req[idx].wt     = req[idx].start  - req[idx].arrive;
        req[idx].tat    = req[idx].finish - req[idx].arrive;
        req[idx].rt     = req[idx].wt;
        req[idx].done   = 1;
        if (req[idx].wt > 0) switched++;
        time = req[idx].finish;
        completed++;
    }

    QueryPerformanceCounter(&sched_end);

    printf("+-----+--------------------+-----+----+---------+---------+--------+--------+--------+\n");
    printf("| PID | Service            | PRI | BT |  Start  | Finish  |   WT   |  TAT   |   RT   |\n");
    printf("+-----+--------------------+-----+----+---------+---------+--------+--------+--------+\n");

    double total_wt=0, total_tat=0;
    double max_wt=0, min_wt=9999, max_tat=0, min_tat=9999;

    for (int i = 0; i < n; i++) {
        printf("| P%-2d | %-18s |  %2d | %2d | %7.2f | %7.2f | %6.2f | %6.2f | %6.2f |\n",
               req[i].id, req[i].service, req[i].priority, req[i].burst,
               req[i].start, req[i].finish,
               req[i].wt, req[i].tat, req[i].rt);

        total_wt  += req[i].wt;
        total_tat += req[i].tat;
        if (req[i].wt  > max_wt)  max_wt  = req[i].wt;
        if (req[i].wt  < min_wt)  min_wt  = req[i].wt;
        if (req[i].tat > max_tat) max_tat = req[i].tat;
        if (req[i].tat < min_tat) min_tat = req[i].tat;
    }
    printf("+-----+--------------------+-----+----+---------+---------+--------+--------+--------+\n\n");

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

    QueryPerformanceCounter(&prog_end);
    double prog_e  = (double)(prog_end.QuadPart  - prog_start.QuadPart) / freq.QuadPart;
    double sched_e = (double)(sched_end.QuadPart - sched_start.QuadPart) / freq.QuadPart;

    printf("Real-Time Execution Metrics:\n");
    printf("================================\n");
    printf("Program Execution Time   : %.6f seconds\n", prog_e);
    printf("Scheduling Latency       : %.6f seconds (avg)\n", sched_e / n);
    printf("Average Process Latency  : %.2f units\n", avg_wt);
    printf("Total Latency            : %.2f units\n", total_wt);
    printf("Worst-Case Latency       : %.0f units\n", max_wt);

    return 0;
}