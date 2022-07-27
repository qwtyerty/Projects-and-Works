#include "uarray2.h"
#include "uarray2b.h"
#include <assert.h>
#include <array.h>
#include <stdlib.h>
#include <stdio.h>
#include <mem.h>
#include <math.h>

#define T UArray2b_T

// The blocked array is a uarray2 where each index stores
// an Array_T which can hold data that is stored locally to
// each other

struct T
{
  int width;
  int height;
  int size;
  int blocksize;
  int blockWidth;
  int blockHeight;
  UArray2_T blocks;
};


//Helper functions
void applyBlock(int x, int y, T array2b, Array_T block, 
      void apply(int c, int r, T array2b, void* elem, void* cl),
      void *cl);

int getNumBlocks(int dim, int blocksize);



T UArray2b_new(int width, int height, int size, int blocksize)
{
  /**
   * Creates a Uarray2b with a given block size
   * **/
  assert(width > 0 && height > 0 && blocksize > 0 && size > 0);

  T array2b = NEW(array2b);
  
  array2b->width = width;
  array2b->height = height;
  array2b->size = size;
  array2b->blocksize = blocksize;
  array2b->blockWidth = getNumBlocks(width, blocksize);
  array2b->blockHeight = getNumBlocks(height, blocksize);

  array2b->blocks = UArray2_new(array2b->blockWidth, array2b->blockHeight,
      sizeof(Array_T));


  //This loop hit all blocksize^2 blocks by passing over each block 
  //and then iterating through the whole array of size blocksize^2
  int arrayLen = blocksize * blocksize;
  for(int j = 0; j < array2b->blockHeight; ++j)
  {
    for(int i = 0; i < array2b->blockWidth; ++i)
    {
      Array_T* blockArray = UArray2_at(array2b->blocks, i, j);
      *blockArray = Array_new(arrayLen, size);
    }
  }
  return array2b;
}


T UArray2b_new_16K_block(int width, int height, int size)
{
  /**
   * Creates A 16k Uarray2b
   * **/
  assert(width > 0 && height > 0 && size > 0);
  int blocksize = sqrt(16/size);
  if(blocksize < 1)
  {
    blocksize = 1;
  }
  return UArray2b_new(width, height, size, blocksize);
}


void UArray2b_free(T* array2b)
{
  /**
   * Frees the UArray2b from memory
   * **/

  assert(array2b && (*array2b));
  
  //This loop hits each block and frees the entire Array_T before
  //moving to the next block then freeing the whole UArray2b
  for(int j = 0; j < (*array2b)->blockHeight; ++j)
  {
    for(int i = 0; i < (*array2b)->blockWidth; ++i)
    {
      Array_T* garbage = UArray2_at((*array2b)->blocks, i, j);
      Array_free(garbage);
    }
  }

  UArray2_free(&((*array2b)->blocks));
  free(*array2b);
}

int UArray2b_width(T array2b)
{
  //Returns the width
  assert(array2b);
  return array2b->width;
}

int UArray2b_height(T array2b)
{
  //Returns the height
  assert(array2b);
  return array2b->height;
}

int UArray2b_size(T array2b)
{
  //Returns the size
  assert(array2b);
  return array2b->size;
}

int UArray2b_blocksize(T array2b)
{
  //Returns the blocksize
  assert(array2b);
  return array2b->blocksize;
}

void* UArray2b_at(T array2b, int c, int r)
{
  /**
   * Gets a value at a given column and row
   * **/

  assert(array2b);
  assert(c >= 0 && r >= 0);
  int blocksize = array2b->blocksize;

  //Get the target index
  Array_T* target = UArray2_at(array2b->blocks, c/blocksize, r/blocksize);

  //Treats the array as a 2day array for the block
  int targetIdx = blocksize * (c % blocksize) + (r % blocksize);
  return Array_get(*target, targetIdx);
} 

void UArray2b_map(T array2b, void apply(int c, int r, T array2b, void *elem,
      void *cl), void *cl)
{
  /**
   * Maps over the whole UArray2b and calls the apply on each index
   * **/
  assert(array2b);
  

  // Each block gets visited then calls apply block which then
  // Acts as a 2D array which we traverse cell-by-cell, then block-by-block
  // hitting each index
  
  //Invariant: Each cell is located within a block where other cells are all
  //stored locally to each other

  //Init: Map has not traversed any cell nor block and has not been given any
  //element to call the apply function on

  //Maintainence: We are making progress as we are moving further in (c,r)
  //space as we slowly call the apply function for each given cell
  
  //Termination: Map has ran out of blocks and cells to traverse over and hits
  //the limit of both the height and width of the UArray2B
  for(int j = 0; j < array2b->blockHeight; ++j)
  {
    for(int i = 0; i < array2b->blockWidth; ++i)
    {
      Array_T block = *(Array_T*) UArray2_at(array2b->blocks, i, j);
      applyBlock(i, j, array2b, block, apply, cl);
    }
  }
}

/*
  Helper Functions
*/


int getNumBlocks(int dim, int blocksize)
{
  //Returns the number of blocks
  int numBlocks = dim / blocksize;
  if(dim % blocksize == 0)
    return numBlocks;
  else
    return numBlocks + 1;
}

void applyBlock(int x, int y, T array2b, Array_T block,
      void apply(int c, int r, T array2b, void* elem, void *cl),
      void* cl)
{
  //Does the second part of the map function
  //
  // See invariant documentation above
  int bs = array2b->blocksize;
  int row = y * bs;
  int col = x * bs;
  for(int j =  row; j < row + bs; ++j)
  {
    for(int i = col; i < col + bs; ++i)
    {
      if((j < array2b->height) && (i < array2b->width))
      {
        int idx = bs * (j % bs) + (i % bs);
        apply(i, j, array2b, Array_get(block, idx), cl);
      }
    }
  }
}
