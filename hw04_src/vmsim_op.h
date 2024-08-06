#ifndef VMSIM_OP_H
#define VMSIM_OP_H

#include "vmsim.h"

void op_move(Process *process, char *instruction);
void op_add(Process *process, char *instruction);
void op_load(Process *process, char *instruction);
void op_store(Process *process, char *instruction);

extern void read_page(Process *process, int virt_addr, void *buf, size_t count);
extern void write_page(Process *process, int virt_addr, const void *buf, size_t count);
extern void print_log(int pid, const char *format, ...);

#endif
