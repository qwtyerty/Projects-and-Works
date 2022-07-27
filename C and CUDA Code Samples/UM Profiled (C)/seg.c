#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <seq.h>
#include <array.h>
#include <stack.h>
#include "seg.h"

#define T Seg_T

struct T 
{
  Seq_T segs;
  Stack_T tracker;
};

extern T Seg_new()
{
  /**
   * Initializes a new instance of segmented memory, and 
   * returns the initialized struct
   * **/

  // Allocate memory and assert that it exists
  T mem = malloc(sizeof(*mem));
  assert(mem);

  // Make a sequence, then store the stack for the tracker  
  mem->segs = Seq_new(5);
  assert(mem->segs);
  mem->tracker = Stack_new();

  //Return the struct
  return mem;
}

extern void Seg_free(T mem)
{
  /**
   * Frees the segmented memory
   * **/

  // Initialize storage for an temp segment and length
  Array_T seg;
  int length;

  length = Seq_length(mem->segs);

  //Loop and an remove the lowest in the squence
  //Free it if it isn't already NULL
  for(int i = 0; i < length; ++i)
  {
    seg = Seq_remlo(mem->segs);
    if(seg != NULL)
      Array_free(&seg);
  }

  //Free everything else
  Seq_free(&(mem->segs));
  Stack_free(&(mem->tracker));
  free(mem);
}

extern uint32_t Seg_map(T mem, int size)
{
  /**
   * Maps a segment of memory, and returns its index
   * taking into account the current state of the stack
   * **/

  //Crreate the new seg
  unsigned idx;
  Array_T newSeg = Array_new(size, sizeof(uint32_t));
  assert(newSeg);
  
  //If the stack is empty don't pop
  if(Stack_empty(mem->tracker))
  {
    Seq_addhi(mem->segs, (void*) newSeg);
    idx = Seq_length(mem->segs) - 1;
  }

  //Stack isn't empty we can pop of the top
  else
  {
    idx = (unsigned)(uintptr_t)Stack_pop(mem->tracker);
    Seq_put(mem->segs, idx, (void*) newSeg);
  }

  return (uint32_t)idx;
}

extern void Seg_unmap(T mem, unsigned segId)
{
  /**
   * Unmaps a segment of memory
   * **/
  Array_T seg = Seq_put(mem->segs,segId, NULL);
  Array_free(&seg);
  Stack_push(mem->tracker, (void *)(uintptr_t)segId);

}

extern void Seg_store(T mem, uint32_t val, unsigned segId, 
    unsigned offset)
{
  /**
   * Store a value at a location based on its segmennt id and offset
   * into the array located there
   * **/
  assert(sizeof(val) <= sizeof(uintptr_t));
  Array_T seg = Seq_get(mem->segs, segId);
  *(uint32_t*)Array_get(seg, offset) = val;
}

extern uint32_t Seg_load(T mem, unsigned segId, unsigned offset)
{
  /**
   * Load a value stored at a given segment id and offset
   * and return its value
   * **/
  Array_T seg = Seq_get(mem->segs, segId);
  return *(uint32_t*)Array_get(seg, offset);
}

extern int Seg_length(T mem, unsigned segId)
{
  /**
   * Return the length of the segment
   * **/
  Array_T seg = Seq_get(mem->segs, segId);
  return Array_length(seg);
}

extern void Seg_load_prog(T mem, unsigned segId)
{
  /**
   * Load a program into a segment based on its segment
   * ID**/
  Array_T seg = Seq_get(mem->segs, segId);
  int len = Array_length(seg);
  Array_T newProg = Array_copy(seg, len);
  Array_T seg0 = Seq_put(mem->segs, 0, newProg);
  Array_free(&seg0);

}
