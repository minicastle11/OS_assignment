#ifndef VMSIM_MAIN_H
#define VMSIM_MAIN_H

#include "vmsim.h"

// Functions
void initialize();
void load(const char *filename, int pid);
void simulate();
int execute(Process *process);
void read_page(Process *process, int virt_addr, void *buf, size_t count);
void write_page(Process *process, int virt_addr, const void *buf, size_t count);
void print_log(int pid, const char *format, ...);
void print_register_set(int pid);

// Extern declarations
extern Process *select_process();
extern void reclaim_frames(Process *process);
extern void remove_process(Process *process);

// Global variables
char *phy_memory;
int register_set[MAX_REGISTERS];
int clock;
Frame *frame_list;
ProcessList *process_list;

// TODO: You can add more functions and variables

#endif
