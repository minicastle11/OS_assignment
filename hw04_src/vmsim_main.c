#include "vmsim_main.h"
#include "vmsim_op.h"
#include "vmsim.h"

// Initialization
void initialize()
{
    int i;

    // Physical memory
    phy_memory = (char *)malloc(PHY_MEM_SIZE);
    memset(phy_memory, 0, PHY_MEM_SIZE);

    // Initialize register set
    for (i = 0; i < MAX_REGISTERS; i++)
    {
        register_set[i] = 0;
    }

    // TODO: Initialize frame list, process list, ...
    frame_list = NULL;
    process_list = NULL;
    clock = 0;
}

// Load process from file
void load(const char *filename, int pid)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Failed to open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // TODO: Create a process
    Process *process = (Process *)malloc(sizeof(Process));
    process->pid = pid;
    process->size = 0;
    process->num_inst = 0;

    // TODO: Create a page table of the process
    process->page_table = (PageTableEntry *)malloc(NUM_PAGES * sizeof(PageTableEntry));
    for (int i = 0; i < NUM_PAGES; i++)
    {
        process->page_table[i].frame_number = -1;
        process->page_table[i].valid = 0;
    }

    // TODO: Clear temporary register set of process
    memset(process->temp_reg_set, 0, MAX_REGISTERS * sizeof(int));

    // TODO: Load instructions into page
    char line[INSTRUCTION_SIZE];
    int inst_count = 0;
    while (fgets(line, sizeof(line), file))
    {
        int page_number = (inst_count * INSTRUCTION_SIZE) / PAGE_SIZE;
        int offset = (inst_count * INSTRUCTION_SIZE) % PAGE_SIZE;

        if (!process->page_table[page_number].valid)
        {
            Frame *new_frame = (Frame *)malloc(sizeof(Frame));
            new_frame->pid = process->pid;
            new_frame->page_number = page_number;
            new_frame->next = frame_list;
            frame_list = new_frame;

            process->page_table[page_number].frame_number = page_number;
        }

        int frame_number = process->page_table[page_number].frame_number;
        memcpy(phy_memory + (frame_number * PAGE_SIZE) + offset, line, INSTRUCTION_SIZE);
        inst_count++;
    }

    // Set PC
    process->pc = 0x00000000;
    process->num_inst = inst_count;

    // Add process to process list
    ProcessList *new_process_node = (ProcessList *)malloc(sizeof(ProcessList));
    new_process_node->process = process;
    new_process_node->next = process_list;
    process_list = new_process_node;

    fclose(file);
}

// Simulation
void simulate()
{
    int finish;
    // TODO: Repeat simulation until the process list is empty
    while (process_list != NULL)
    {
        Process *current_process = select_process();

        // TODO: Execue a processs
        finish = execute(current_process);

        // TODO: If the process is terminated then
        if (finish)
        {
            print_register_set(current_process->pid);
            reclaim_frames(current_process);
            remove_process(current_process);
        }
        // TODO: Select a process from process list using round-robin scheme
        clock++;
    }
}

void reclaim_frames(Process *process)
{
    for (int i = 0; i < NUM_PAGES; i++)
    {
        if (process->page_table[i].valid && process->page_table[i].frame_number != -1)
        {
            Frame *frame = frame_list;
            Frame *prev_frame = NULL;

            while (frame != NULL)
            {
                if (frame->pid == process->pid && frame->page_number == i)
                {
                    if (prev_frame != NULL)
                    {
                        prev_frame->next = frame->next;
                    }
                    else
                    {
                        frame_list = frame->next;
                    }

                    free(frame);
                    break;
                }

                prev_frame = frame;
                frame = frame->next;
            }
        }
    }

    free(process->page_table);
}

void remove_process(Process *process)
{
    ProcessList *current = process_list;
    ProcessList *previous = NULL;

    while (current != NULL)
    {
        if (current->process == process)
        {
            if (previous != NULL)
            {
                previous->next = current->next;
            }
            else
            {
                process_list = current->next;
            }

            free(current);
            free(process);
            break;
        }

        previous = current;
        current = current->next;
    }
}

// Execute an instruction using program counter
int execute(Process *process)
{
    char instruction[INSTRUCTION_SIZE];
    char opcode;

    // TODO: Restore register set
    memcpy(register_set, process->temp_reg_set, MAX_REGISTERS * sizeof(int));

    // TODO: Fetch instruction and update program counter
    int virt_addr = process->pc;
    read_page(process, virt_addr, instruction, INSTRUCTION_SIZE);
    process->pc += INSTRUCTION_SIZE;

    // Execute instruction according to opcode
    opcode = instruction[0];
    switch (opcode)
    {
    case 'M':
        op_move(process, instruction);
        break;
    case 'A':
        op_add(process, instruction);
        break;
    case 'L':
        op_load(process, instruction);
        break;
    case 'S':
        op_store(process, instruction);
        break;
    default:
        print_log(process->pid, "Unknown Opcode (%c)", opcode);
        break;
    }

    // TODO: Store register set
    memcpy(process->temp_reg_set, register_set, MAX_REGISTERS * sizeof(int));

    // TODO: When the last instruction is executed return 1, otherwise return 0
    return (process->pc >= process->num_inst * INSTRUCTION_SIZE);
}

// Read up to 'count' bytes from the 'virt_addr' into 'buf'
void read_page(Process *process, int virt_addr, void *buf, size_t count)
{
    // TODO: Get a physical address from virtual address
    int page_number = virt_addr / PAGE_SIZE;
    int offset = virt_addr % PAGE_SIZE;

    // TODO: handle page fault -> just allocate page
    if (!process->page_table[page_number].valid || process->page_table[page_number].frame_number == -1)
    {

        Frame *new_frame = (Frame *)malloc(sizeof(Frame));
        new_frame->pid = process->pid;
        new_frame->page_number = page_number;
        new_frame->next = frame_list;
        frame_list = new_frame;

        process->page_table[page_number].frame_number = page_number;
        process->page_table[page_number].valid = 1;

        // Print page fault message
        print_log(process->pid, "Page fault at virtual address 0x%x (page_number=%d) --› Allocated frame_number=%d",
                  virt_addr, page_number, page_number);
    }
    // TODO: Read up to 'count' bytes from the physical address into 'buf'
    int frame_number = process->page_table[page_number].frame_number;
    memcpy(buf, phy_memory + frame_number * PAGE_SIZE + offset, count);
}

// Write 'buf' up to 'count' bytes at the 'virt_addr'
void write_page(Process *process, int virt_addr, const void *buf, size_t count)
{
    // TODO: Get a physical address from virtual address
    int page_number = virt_addr / PAGE_SIZE;
    int offset = virt_addr % PAGE_SIZE;

    if (!process->page_table[page_number].valid || process->page_table[page_number].frame_number == -1)
    {
        // TODO: handle page fault -> just allocate page
        Frame *new_frame = (Frame *)malloc(sizeof(Frame));
        new_frame->pid = process->pid;
        new_frame->page_number = page_number;
        new_frame->next = frame_list;
        frame_list = new_frame;

        process->page_table[page_number].frame_number = page_number; // Assign the page number as the frame number
        process->page_table[page_number].valid = 1;

        print_log(process->pid, "Page fault at virtual address 0x%x (page_number=%d) --› Allocated frame_number=%d", virt_addr, page_number, page_number);
    }

    // TODO: Write 'buf' up to 'count' bytes at the physical address
    int frame_number = process->page_table[page_number].frame_number;
    memcpy(phy_memory + frame_number * PAGE_SIZE + offset, buf, count);
}

// Print log with format string
void print_log(int pid, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf("[Clock=%2d][PID=%d] ", clock, pid);
    vprintf(format, args);
    printf("\n");
    fflush(stdout);
    va_end(args);
}
// Print values in the register set
void print_register_set(int pid)
{
    int i;
    char str[256], buf[16];
    strcpy(str, "[RegisterSet]:");
    for (i = 0; i < MAX_REGISTERS; i++)
    {
        sprintf(buf, " R[%d]=%d", i, register_set[i]);
        strcat(str, buf);
        if (i != MAX_REGISTERS - 1)
            strcat(str, ",");
    }
    print_log(pid, "%s", str);
}

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > MAX_PROCESSES)
    {
        fprintf(stderr, "Usage: %s <process image files... up to %d files>\n",
                argv[0], MAX_PROCESSES);
        exit(EXIT_FAILURE);
    }

    initialize();

    for (int i = 1; i < argc; i++)
    {
        load(argv[i], i - 1);
    }

    simulate();

    free(phy_memory);

    return 0;
}
Process *select_process()
{
    static ProcessList *current_process_node = NULL;

    if (!current_process_node)
    {
        current_process_node = process_list;
    }
    else
    {
        current_process_node = current_process_node->next;
        if (!current_process_node)
        {
            current_process_node = process_list;
        }
    }

    return current_process_node->process;
}