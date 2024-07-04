#include <stdlib.h>
#include "fixed_buffer_allocator.h"

void FixedBufferAllocator_init(
    FixedBufferAllocator *fixed_buffer,
    void *buffer,
    size_t size
) {
    size_t aligned_head;

    // aligning the addres to a multiple of sizeof(void *)
    aligned_head = (size_t)buffer;
    aligned_head = (aligned_head + sizeof(void *) - 1) & ~(sizeof(void *) - 1);

    fixed_buffer->start = (char *)(aligned_head);
    fixed_buffer->head = (char *)(aligned_head);
    fixed_buffer->end = fixed_buffer->start + size;
}

void *
FixedBufferAllocator_alloc(FixedBufferAllocator *fixed_buffer, size_t size) {
    void *next_head;

    next_head = fixed_buffer->head + size;

    if (next_head > fixed_buffer->end)
        return NULL;

    void *allocation;
    size_t aligned_head;

    allocation = fixed_buffer->head;
    fixed_buffer->head = next_head;

    // aligning the addres to a multiple of sizeof(void *)
    aligned_head = (size_t)fixed_buffer->head;
    aligned_head = (aligned_head + sizeof(void *) - 1) & ~(sizeof(void *) - 1);
    fixed_buffer->head = (char *)(aligned_head);

    return allocation;
}

void *FixedBufferAllocator_realloc(
    FixedBufferAllocator *fixed_buffer,
    void *ptr,
    size_t new_size
) {
    return FixedBufferAllocator_alloc(fixed_buffer, new_size);
}

void FixedBufferAllocator_clear(FixedBufferAllocator *fixed_buffer) {
    fixed_buffer->head = fixed_buffer->start;
}