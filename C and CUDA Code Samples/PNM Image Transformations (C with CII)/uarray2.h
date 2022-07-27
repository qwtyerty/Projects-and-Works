#ifndef UARRAY2_INCLUDED
#define UARRAY2_INCLUDED
#define T UArray2_T

// Defines the struct for 2D Arrays
typedef struct T *T;

extern T UArray2_new(int width, int height, int size);
extern void UArray2_free(T *uarray);
extern int UArray2_width(T uarray);
extern int UArray2_height(T uarray);
extern int UArray2_size(T uarray);
extern void *UArray2_at(T uarray, int x, int y);
extern void UArray2_map_row_major(T uarray, void apply(int x, int y, T uarray, void *elem, void *cl), void *cl);
extern void UArray2_map_col_major(T uarray, void apply(int x, int y, T uarray, void *elem, void *cl), void *cl);

#undef T
#endif
