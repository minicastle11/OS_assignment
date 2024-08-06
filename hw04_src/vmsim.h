#ifndef VMSIM_H
#define VMSIM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define PAGE_SIZE 4096                  // 4KB
#define PHY_MEM_SIZE (16 * 1024 * 1024) // 16MB
#define NUM_PAGES (PHY_MEM_SIZE / PAGE_SIZE)
#define MAX_PROCESSES 5
#define MAX_REGISTERS 8
#define INSTRUCTION_SIZE 32 // 32 bytes

typedef struct
{
    int frame_number; // frame number
    int valid;        // valid is 1 if page is in physical memory
} PageTableEntry;

typedef struct
{
    int pid;                         // process ID
    int size;                        // total size of process
    int num_inst;                    // number of instruction in file
    int pc;                          // program counter which holds address of next instruction
    int temp_reg_set[MAX_REGISTERS]; // Register set for context switching
    PageTableEntry *page_table;      // Page table
} Process;

typedef struct Frame
{
    int pid;
    int page_number;
    struct Frame *next;
} Frame;

typedef struct ProcessList
{
    Process *process;
    struct ProcessList *next;
} ProcessList;

// Global variables
extern char *phy_memory;
extern int register_set[MAX_REGISTERS];
extern int clock;
extern Frame *frame_list;
extern ProcessList *process_list;

// Function prototypes
Process *select_process();
void reclaim_frames(Process *process);
void remove_process(Process *process);

// Function prototypes from vmsim_main.c
void initialize();
void load(const char *filename, int pid);
void simulate();
int execute(Process *process);
void read_page(Process *process, int virt_addr, void *buf, size_t count);
void write_page(Process *process, int virt_addr, const void *buf, size_t count);
void print_log(int pid, const char *format, ...);
void print_register_set(int pid);

// TODO: You can add more definitions

#endif
