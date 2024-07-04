#ifndef __FIXED_BUFFER_ALLOCATOR_H__
#define __FIXED_BUFFER_ALLOCATOR_H__

typedef struct FixedBufferAllocator {
    char *start;
    char *end;
    char *head;
} FixedBufferAllocator;

#endif /* __FIXED_BUFFER_ALLOCATOR_H__ */