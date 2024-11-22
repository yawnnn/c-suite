#ifndef __TRACKING_ALLOCATOR_H__
#define __TRACKING_ALLOCATOR_H__

typedef struct TrackingNode {
   struct TrackingNode *next;
} TrackingNode;

typedef struct TrackingList {
   TrackingNode *head;
} TrackingList;

// keeps track of allocations and frees them all at once
typedef struct TrackingAllocator {
   TrackingList list;
} TrackingAllocator;

void tracking_allocator_init(TrackingAllocator *allocator);
void *tracking_allocator_alloc(TrackingAllocator *allocator, size_t size);
void *tracking_allocator_realloc(TrackingAllocator *allocator, size_t size, void *prev_allocation);
void tracking_allocator_free(TrackingAllocator *allocator);
void tracking_allocator_reset(TrackingAllocator *allocator);

#endif /* __TRACKING_ALLOCATOR_H__ */