// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Alocador básico de memória heap.

// implementação trivial, a ser substituída no projeto de alocação de heap.
#include <stdio.h>
#include "memory.h"

#define HEAP_SIZE 64 * 1024 * 1024
#define ERROR -1

static char heap[HEAP_SIZE];

int blocks_amount = 0;
int current_block_id = 0;

long get_struct_block_size()
{
    return ((sizeof(struct mem_block) - 1) | 0x00000F) + 1;
}

struct mem_block *create_new_mem_block(void *address, long int size)
{
    struct mem_block *new_block = address;

    new_block->initial_address = address + get_struct_block_size();
    new_block->block_size = size - get_struct_block_size();
    new_block->is_allocked = 0;
    new_block->left_block = new_block;
    new_block->right_block = new_block;
    new_block->id = current_block_id;

    blocks_amount++;
    current_block_id++;

    struct mem_block *first_block = (struct mem_block *)heap;

    if (first_block != NULL && first_block->left_block < new_block)
    {
        first_block->left_block = new_block;
    }

    return new_block;
}

int mem_block_coallescence(struct mem_block *block)
{
    if (block == NULL ||
        block->is_allocked ||
        block == block->right_block ||
        block->right_block->initial_address < block->initial_address ||
        block->right_block->is_allocked)
    {
        return 0;
    }
    struct mem_block *right_block = block->right_block;
    block->block_size += right_block->block_size + get_struct_block_size();
    block->right_block = right_block->right_block;
    block->right_block->left_block = block;

    right_block = NULL;

    return 1;
}

void mem_init()
{
    create_new_mem_block(
        &heap,
        HEAP_SIZE);
}

void *mem_alloc(int size)
{
    if (size <= 0 || size > mem_avail())
    {
        return NULL;
    }
    int adjusted_size = ((size - 1) | 0x00000F) + 1;

    struct mem_block *current_block = (struct mem_block *)heap;
    int blocks_accessed = 0;

    while (current_block != NULL && blocks_accessed < blocks_amount)
    {
        if (!current_block->is_allocked && current_block->block_size >= adjusted_size)
        {
            if (current_block->block_size != adjusted_size)
            {
                struct mem_block *new_block = create_new_mem_block(
                    current_block->initial_address + adjusted_size,
                    current_block->block_size - adjusted_size);

                new_block->right_block = current_block->right_block;
                new_block->left_block = current_block;
                current_block->right_block->left_block = new_block;
                current_block->right_block = new_block;
                current_block->block_size = adjusted_size;
            }

            current_block->is_allocked = 1;

            return current_block->initial_address;
        }
        blocks_accessed++;
        current_block = current_block->right_block;
    }

    return NULL;
}

int mem_free(void *ptr)
{
    if (ptr == NULL || ptr < (void *)&heap || ptr > (void *)(heap + HEAP_SIZE))
    {
        return ERROR;
    }

    struct mem_block *block = (struct mem_block *)(ptr - get_struct_block_size());
    if (!block->is_allocked)
    {
        return ERROR;
    }

    struct mem_block *left_block = block->left_block;

    block->is_allocked = 0;

    if (mem_block_coallescence(left_block))
    {
        block = left_block;
        blocks_amount--;
    }

    if (mem_block_coallescence(block))
    {
        blocks_amount--;
    }

    return (0);
}

void mem_report()
{
    struct mem_block *current_block = (struct mem_block *)heap;
    int free_mem = 0, free_blocks = 0;
    int total_mem = mem_size();
    char *alloc_state;

    while (current_block != NULL)
    {
        if(!current_block->is_allocked)
        {
            free_mem += current_block->block_size;
            free_blocks++;
        }
        if (current_block < current_block->right_block)
            current_block = current_block->right_block;
        else
            current_block = NULL;
    }

    printf("heap: %ld KB allocated (%d blocks), %d KB free (%d blocks) | %p - %p\n", 
        (total_mem - free_mem + (blocks_amount * get_struct_block_size())) / 1024,
        blocks_amount - free_blocks,
        free_mem / 1024,
        free_blocks,
        heap,
        heap + HEAP_SIZE
    );

    current_block = (struct mem_block *)heap;
    while (current_block != NULL)
    {
        if (current_block->is_allocked) 
        {
            alloc_state = "aloc";
        } else {
            alloc_state = "FREE";
        }
        printf("heap: block %d: %p | %p - %p %s prev %d next %d size %ld\n",
               current_block->id,
               current_block,
               current_block->initial_address,
               current_block->initial_address + current_block->block_size,
               alloc_state,
               current_block->left_block->id,
               current_block->right_block->id,
               current_block->block_size);
        if (current_block < current_block->right_block)
            current_block = current_block->right_block;
        else
            current_block = NULL;
    }
}

// informa a quantidade de memória total, em bytes
int mem_size() 
{
    return HEAP_SIZE - (blocks_amount * get_struct_block_size());
}

// informa a quantidade de memória disponível, em bytes
int mem_avail()
{
    int free_mem = 0;

    struct mem_block *current_block = (struct mem_block *)heap;
    while (current_block != NULL)
    {
        if(!current_block->is_allocked)
        {
            free_mem += current_block->block_size;
        }
        if (current_block < current_block->right_block)
            current_block = current_block->right_block;
        else
            current_block = NULL;
    }

    return free_mem / 8;
}
