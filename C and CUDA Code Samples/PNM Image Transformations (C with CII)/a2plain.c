#include <stdlib.h>
#include <a2methods.h>
#include <a2plain.h>
#include <uarray2.h>

// define a private version of each function in A2Methods_T that we implement

//Defines functions for use with unblocked 2D Arrays

typedef A2Methods_Array2 T;

static T new(int width, int height, int size) {
  return UArray2_new(width, height, size);
}

static T new_with_blocksize(int width, int height, int size,
                                        int blocksize)
{
  (void) blocksize;
  return UArray2_new(width, height, size);
}

static void a2free(T* array2)
{
  UArray2_free((UArray2_T*) array2);
}

static int width(T array2)
{
  return UArray2_width(array2);
}

static int height(T array2)
{
  return UArray2_height(array2);
}

static int size(T array2)
{
  return UArray2_size(array2);
}

static int blocksize(T array2)
{
  (void) array2;
  return 1;
}

static A2Methods_Object* at(T array2, int i, int j)
{
  return UArray2_at(array2, i, j);
}

typedef void applyfun(int i, int j, UArray2_T array2, void* elem, void *cl); 

static void map_row_major(T array2, A2Methods_applyfun apply, void *cl)
{
  UArray2_map_row_major(array2, (applyfun*) apply, cl);
}

static void map_col_major(T array2, A2Methods_applyfun apply, void *cl)
{
  UArray2_map_col_major(array2, (applyfun*) apply, cl);
}

// now create the private struct containing pointers to the functions

static struct A2Methods_T array2_methods_plain_struct = {
  new,
  new_with_blocksize,
  a2free,
  width,
  height,
  size,
  blocksize,
  at,
  map_row_major,
  map_col_major,
  NULL,
  map_col_major, // map_defeault calls col since col provides better locality
};

// finally the payoff: here is the exported pointer to the struct

A2Methods_T array2_methods_plain = &array2_methods_plain_struct;
