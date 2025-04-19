#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BLOCKS 1000

typedef struct {
    int start;
    int end;
    char process[20]; // empty string means unused
} Block;

Block memory[MAX_BLOCKS];
int block_count = 0;
int total_memory;

void init_memory(int size) {
    total_memory = size;
    block_count = 1;
    memory[0].start = 0;
    memory[0].end = size - 1;
    strcpy(memory[0].process, ""); // unused
}

int block_size(Block *b) {
    return b->end - b->start + 1;
}

void print_memory() {
    for (int i = 0; i < block_count; i++) {
        if (strlen(memory[i].process) == 0)
            printf("Addresses [%d:%d] Unused\n", memory[i].start, memory[i].end);
        else
            printf("Addresses [%d:%d] Process %s\n", memory[i].start, memory[i].end, memory[i].process);
    }
}

void compact_memory() {
    Block new_memory[MAX_BLOCKS];
    int new_count = 0;
    int current = 0;
    for (int i = 0; i < block_count; i++) {
        if (strlen(memory[i].process) > 0) {
            int size = block_size(&memory[i]);
            new_memory[new_count].start = current;
            new_memory[new_count].end = current + size - 1;
            strcpy(new_memory[new_count].process, memory[i].process);
            current += size;
            new_count++;
        }
    }
    if (current < total_memory) {
        new_memory[new_count].start = current;
        new_memory[new_count].end = total_memory - 1;
        strcpy(new_memory[new_count].process, "");
        new_count++;
    }
    memcpy(memory, new_memory, sizeof(new_memory));
    block_count = new_count;
}

void release_process(char *proc) {
    for (int i = 0; i < block_count; i++) {
        if (strcmp(memory[i].process, proc) == 0) {
            strcpy(memory[i].process, "");
            // merge with previous
            if (i > 0 && strlen(memory[i-1].process) == 0) {
                memory[i-1].end = memory[i].end;
                for (int j = i; j < block_count - 1; j++)
                    memory[j] = memory[j+1];
                block_count--;
                i--;
            }
            // merge with next
            if (i + 1 < block_count && strlen(memory[i+1].process) == 0) {
                memory[i].end = memory[i+1].end;
                for (int j = i + 1; j < block_count - 1; j++)
                    memory[j] = memory[j+1];
                block_count--;
            }
            return;
        }
    }
    printf("Error: Process not found\n");
}

void request_memory(char *proc, int size, char strategy) {
    int best_index = -1;
    int best_size = (strategy == 'W') ? -1 : total_memory + 1;

    for (int i = 0; i < block_count; i++) {
        if (strlen(memory[i].process) == 0 && block_size(&memory[i]) >= size) {
            int current_size = block_size(&memory[i]);
            if ((strategy == 'F' && best_index == -1) ||
                (strategy == 'B' && current_size < best_size) ||
                (strategy == 'W' && current_size > best_size)) {
                best_index = i;
                best_size = current_size;
            }
        }
    }

    if (best_index == -1) {
        printf("Error: Not enough memory\n");
        return;
    }

    Block *hole = &memory[best_index];
    int original_end = hole->end;
    int new_end = hole->start + size - 1;

    // shift blocks if needed
    for (int i = block_count; i > best_index; i--)
        memory[i] = memory[i - 1];
    block_count++;

    memory[best_index].end = new_end;
    strcpy(memory[best_index].process, proc);

    memory[best_index + 1].start = new_end + 1;
    memory[best_index + 1].end = original_end;
    strcpy(memory[best_index + 1].process, "");

    if (memory[best_index + 1].start > memory[best_index + 1].end)
        block_count--;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./allocator <size>\n");
        return 1;
    }

    init_memory(atoi(argv[1]));

    char line[100];
    while (1) {
        printf("allocator> ");
        if (!fgets(line, sizeof(line), stdin)) break;

        char cmd[10], proc[20], strategy;
        int size;

        if (sscanf(line, "%s %s %d %c", cmd, proc, &size, &strategy) == 4 && strcmp(cmd, "RQ") == 0) {
            request_memory(proc, size, strategy);
        } else if (sscanf(line, "%s %s", cmd, proc) == 2 && strcmp(cmd, "RL") == 0) {
            release_process(proc);
        } else if (strncmp(line, "STAT", 4) == 0) {
            print_memory();
        } else if (strncmp(line, "C", 1) == 0) {
            compact_memory();
        } else if (strncmp(line, "X", 1) == 0) {
            break;
        } else {
            printf("Invalid command\n");
        }
    }

    return 0;
}
