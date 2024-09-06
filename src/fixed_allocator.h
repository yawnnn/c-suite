#ifndef __FIXED_ALLOCATOR_H__
#define __FIXED_ALLOCATOR_H__

typedef struct FixedAllocator {
   char *start;
   char *end;
   char *head;
} FixedAllocator;

#endif /* __FIXED_ALLOCATOR_H__ */