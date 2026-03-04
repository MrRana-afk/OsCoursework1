#include <stdio.h>
#include <windows.h>

/*
 * RANA Pvt. Ltd. — Inventory Race Condition (PROBLEM)
 * Platform : Windows Server
 * Scenario : Multiple customers buying last stock simultaneously
 *            WITHOUT any synchronization — shows race condition
 * Compile  : gcc -o sync_problem sync_problem.c -lpthread
 * Run      : sync_problem.exe
 */

#define TOTAL_CUSTOMERS  10
#define INITIAL_STOCK     5

int stock = INITIAL_STOCK;

typedef struct {
    int  id;
    char name[20];
    int  wants;
} Customer;

DWORD WINAPI unsafePurchase(LPVOID arg) {
    Customer *c = (Customer *)arg;

    /* No lock — multiple threads read same stock value */
    int seen = stock;

    /* Simulate processing delay — widens the race window */
    Sleep(10);

    if (seen >= c->wants) {
        stock -= c->wants;
        printf("[UNSAFE] P%-2d %-12s wants %d  bought  %d | Stock left: %d\n",
               c->id, c->name, c->wants, c->wants, stock);
    } else {
        printf("[UNSAFE] P%-2d %-12s wants %d  FAILED  | Stock left: %d\n",
               c->id, c->name, c->wants, stock);
    }

    return 0;
}

int main() {

    LARGE_INTEGER freq, prog_start, prog_end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prog_start);

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

    HANDLE threads[TOTAL_CUSTOMERS];

    printf("=============================================================\n");
    printf("  RANA Pvt. Ltd.  Synchronization PROBLEM Demo (Windows)\n");
    printf("  Initial Stock : %d units\n", INITIAL_STOCK);
    printf("  Customers     : %d concurrent buyers\n", TOTAL_CUSTOMERS);
    printf("  Issue         : No lock race condition possible\n");
    printf("=============================================================\n\n");

    for (int i = 0; i < TOTAL_CUSTOMERS; i++)
        threads[i] = CreateThread(NULL, 0, unsafePurchase, &customers[i], 0, NULL);

    WaitForMultipleObjects(TOTAL_CUSTOMERS, threads, TRUE, INFINITE);

    for (int i = 0; i < TOTAL_CUSTOMERS; i++)
        CloseHandle(threads[i]);

    QueryPerformanceCounter(&prog_end);
    double prog_e = (double)(prog_end.QuadPart - prog_start.QuadPart) / freq.QuadPart;

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
    printf("Stock Corruption         : %s\n", stock < 0 ? "YES oversold!" : "Possible");

    return 0;
}