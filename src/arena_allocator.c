#include <stdlib.h>
#include "arena_allocator.h"

void ArenaAllocator_init(ArenaAllocator *arena) {
    arena->head = NULL;
}

void *ArenaAllocator_alloc(ArenaAllocator *arena, size_t bytes) {
    ArenaNode *next;

    next = (ArenaNode *)malloc(sizeof(ArenaNode) + bytes);
    next->next = arena->head;
    arena->head = next;

    /* normally you should worry about alignment here, but from what i understand
     * the result of malloc is already aligned
     * and sizeof(ArenaNode) == sizeof(ArenaNode *) == sizeof(void *), which is always a good alignment */
    return ((char *)next) + sizeof(ArenaNode);
}

void *ArenaAllocator_realloc(
    ArenaAllocator *arena,
    size_t bytes,
    void *prev_allocation
) {
    if (arena->head) {
        ArenaNode *curr, *prev, *needle;

        needle = (ArenaNode *)(((char *)prev_allocation) - sizeof(ArenaNode));

        prev = NULL;
        curr = arena->head;
        while (curr && curr != needle) {
            prev = curr;
            curr = curr->next;
        }

        if (curr) {
            ArenaNode *next;

            next = curr->next;
            needle = (ArenaNode *)realloc(needle, sizeof(ArenaNode) + bytes);
            needle->next = next;

            if (prev)
                prev->next = needle;
            else
                arena->head = needle;

            return ((char *)next) + sizeof(ArenaNode);
        }
    }

    return NULL;
}

void ArenaAllocator_free(ArenaAllocator *arena) {
    if (arena->head) {
        ArenaNode *curr, *next;
        
        curr = arena->head;
        next = curr->next;
        while (next) {
            curr = next;
            next = next->next;
            free(curr);
        }

        free(arena->head);
        arena->head = NULL;
    }
}