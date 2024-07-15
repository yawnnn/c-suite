#ifndef __FIXED_BUFFER_H__
#define __FIXED_BUFFER_H__

typedef struct FixedBuffer {
    char *start;
    char *end;
    char *head;
} FixedBuffer;

#endif /* __FIXED_BUFFER_H__ */