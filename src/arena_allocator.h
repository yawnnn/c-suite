#ifndef __ARENA_ALLOCATOR_H__
#define __ARENA_ALLOCATOR_H__

typedef struct ArenaNode {
    struct ArenaNode *next;
} ArenaNode;

typedef struct ArenaAllocator {
    ArenaNode *head;
} ArenaAllocator;

#endif /* __ARENA_ALLOCATOR_H__ */