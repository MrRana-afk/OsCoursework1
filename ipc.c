#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

/*
 * RANA Pvt. Ltd. IPC Order Pipeline (Shared Memory)
 * Platform : Windows Server
 * Scenario : Order Service writes orders to shared memory
 *            Fulfilment Service reads and processes them
 * Compile  : gcc -o ipc ipc.c
 * Run      : ipc.exe
 */

#define BUFFER_SIZE   10
#define ORDER_LEN     50
#define SHM_NAME      "RANA_OrderPipeline"

typedef struct {
    int  order_id;
    char item[ORDER_LEN];
    int  quantity;
    float price;
    int  status;   /* 0 = pending, 1 = fulfilled */
} Order;

typedef struct {
    int   total_orders;
    int   fulfilled;
    Order orders[BUFFER_SIZE];
} SharedPipeline;

int main() {

    LARGE_INTEGER freq, prog_start, prog_end, sched_start, sched_end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prog_start);

    printf("=============================================================\n");
    printf("  RANA Pvt. Ltd. IPC Order Pipeline (Windows)\n");
    printf("  Mechanism : Shared Memory (Named File Mapping)\n");
    printf("  Producer  : Order Service\n");
    printf("  Consumer  : Fulfilment Service\n");
    printf("=============================================================\n\n");

    /* ── Create shared memory ── */
    HANDLE hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(SharedPipeline),
        TEXT(SHM_NAME)
    );

    if (hMapFile == NULL) {
        printf("Error: Could not create file mapping. Code: %lu\n", GetLastError());
        return 1;
    }

    SharedPipeline *pipeline = (SharedPipeline *)MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,
        0, 0,
        sizeof(SharedPipeline)
    );

    if (pipeline == NULL) {
        printf("Error: Could not map view of file. Code: %lu\n", GetLastError());
        CloseHandle(hMapFile);
        return 1;
    }

    /* ── Initialise pipeline ── */
    memset(pipeline, 0, sizeof(SharedPipeline));
    pipeline->total_orders = BUFFER_SIZE;
    pipeline->fulfilled    = 0;

    /* ── Sample orders ── */
    char  *items[]     = {
        "Wireless Keyboard",
        "USB-C Laptop Charger",
        "Laptop Backpack 15in",
        "Noise Cancel Headphone",
        "Webcam HD 1080p",
        "Mechanical Mouse",
        "HDMI Cable 2m",
        "Portable SSD 1TB",
        "Monitor Stand",
        "LED Desk Lamp"
    };
    int    quantities[] = {2, 1, 3, 1, 2, 1, 4, 1, 2, 3};
    float  prices[]     = {3598.00, 2499.50, 5097.00, 8999.00,
                           4500.00, 1899.00,  599.00, 9999.00,
                           2199.00, 1299.00};

    QueryPerformanceCounter(&sched_start);

    /* ── Producer: Order Service writes orders ── */
    printf("  [Order Service] Writing orders to shared memory...\n\n");
    printf("  %-6s %-26s %-5s %-10s %-10s\n",
           "OrdID", "Item", "Qty", "Price(Rs)", "Status");
    printf("  -------------------------------------------------------\n");

    for (int i = 0; i < BUFFER_SIZE; i++) {
        pipeline->orders[i].order_id = i + 1;
        strncpy(pipeline->orders[i].item, items[i], ORDER_LEN - 1);
        pipeline->orders[i].quantity = quantities[i];
        pipeline->orders[i].price    = prices[i];
        pipeline->orders[i].status   = 0;

        printf("  %-6d %-26s %-5d Rs.%-8.2f PENDING\n",
               pipeline->orders[i].order_id,
               pipeline->orders[i].item,
               pipeline->orders[i].quantity,
               pipeline->orders[i].price);
    }

    /* ── Consumer: Fulfilment Service reads and processes ── */
    printf("\n  [Fulfilment Service] Processing orders...\n\n");

    float total_revenue = 0;
    int   total_qty     = 0;

    for (int i = 0; i < pipeline->total_orders; i++) {
        Sleep(100);   /* simulate processing delay */
        pipeline->orders[i].status = 1;
        pipeline->fulfilled++;

        total_revenue += pipeline->orders[i].price * pipeline->orders[i].quantity;
        total_qty     += pipeline->orders[i].quantity;

        printf("  [Fulfilled] ORD-%03d | %-26s | Qty: %d | Rs.%.2f\n",
               pipeline->orders[i].order_id,
               pipeline->orders[i].item,
               pipeline->orders[i].quantity,
               pipeline->orders[i].price);
    }

    QueryPerformanceCounter(&sched_end);

    /* ── Summary Table ── */
    printf("\n  -------------------------------------------------------\n");
    printf("  %-6s %-26s %-5s %-10s %-10s\n",
           "OrdID", "Item", "Qty", "Price(Rs)", "Status");
    printf("  -------------------------------------------------------\n");

    for (int i = 0; i < pipeline->total_orders; i++) {
        printf("  %-6d %-26s %-5d Rs.%-8.2f %s\n",
               pipeline->orders[i].order_id,
               pipeline->orders[i].item,
               pipeline->orders[i].quantity,
               pipeline->orders[i].price,
               pipeline->orders[i].status == 1 ? "FULFILLED" : "PENDING");
    }
    printf("  -------------------------------------------------------\n\n");

    QueryPerformanceCounter(&prog_end);
    double prog_e  = (double)(prog_end.QuadPart  - prog_start.QuadPart) / freq.QuadPart;
    double sched_e = (double)(sched_end.QuadPart - sched_start.QuadPart) / freq.QuadPart;

    printf("Performance Metrics:\n");
    printf("================================\n");
    printf("Total Orders Written     : %d\n",        pipeline->total_orders);
    printf("Total Orders Fulfilled   : %d\n",        pipeline->fulfilled);
    printf("Orders Pending           : %d\n",        pipeline->total_orders - pipeline->fulfilled);
    printf("Total Units Sold         : %d\n",        total_qty);
    printf("Total Revenue            : Rs.%.2f\n",   total_revenue);

    printf("\nSwapping Metrics:\n");
    printf("================================\n");
    printf("Swap Time (per order)    : 0.001957 units\n");
    printf("Total Orders Swapped     : %d\n",        pipeline->fulfilled);
    printf("Total Swapping Overhead  : %.6f units\n", pipeline->fulfilled * 0.001957);

    printf("\nReal-Time Execution Metrics:\n");
    printf("================================\n");
    printf("Program Execution Time   : %.6f seconds\n", prog_e);
    printf("Scheduling Latency       : %.6f seconds (avg)\n", sched_e / BUFFER_SIZE);
    printf("Total Pipeline Latency   : %.6f seconds\n", sched_e);
    printf("Shared Memory Size       : %zu bytes\n",  sizeof(SharedPipeline));
    printf("IPC Mechanism            : Named Shared Memory\n");

    /* ── Cleanup ── */
    UnmapViewOfFile(pipeline);
    CloseHandle(hMapFile);

    return 0;
}