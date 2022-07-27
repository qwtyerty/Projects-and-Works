#ifndef SEG_INCLUDED
#define SEG_INCLUDED
#include <stdint.h>

#define T Seg_T
typedef struct T* T;

extern T Seg_new();
extern void Seg_free(T mem);
extern uint32_t Seg_map(T mem, int size);
extern void Seg_unmap(T mem, unsigned segId);
extern void Seg_store(T mem, uint32_t val, unsigned segId,
    unsigned offset);
extern uint32_t Seg_load(T mem, unsigned segId, unsigned offset);
extern int Seg_length(T mem, unsigned segId);
extern void Seg_load_prog(T mem, unsigned segId);

#undef T
#endif 
