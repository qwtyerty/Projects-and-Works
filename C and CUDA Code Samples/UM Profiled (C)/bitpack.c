#include "bitpack.h"
#include "assert.h"
#include <stdio.h>
#include "inttypes.h"

Except_T Bitpack_Overflow = { "Overflow packing bits" };


/**
 *  Helper Functions 
 * ***/

static inline uint64_t leftShift(uint64_t word, unsigned offset)
{
  if(offset <= 63)
    return (word << offset);
  
  else
    return 0;
}

static inline int64_t arithRightShift(int64_t word, unsigned offset)
{
  assert(offset <= 64);

  if(offset <= 63)
    return (word >> offset);

  else if(word >= 0)
    return 0;

  else 
      return ~0;
}

static inline uint64_t logicalRightShift(uint64_t word, unsigned offset)
{

  if(offset <= 63)
    return (word >> offset);

  else
    return 0;
}

static inline uint64_t bitpackReplace(uint64_t word, unsigned width, unsigned
    lsb, int64_t value)
{
  int64_t mask = ~0;
  mask = leftShift(mask, (64 - width));
  mask = logicalRightShift(mask, (64 - width - lsb));

  value = leftShift(value, lsb);
  value = (value & mask);
  word = (word & ~mask);
  
  return (word | value);
}


/***
 *  Actual functions
 * ***/

bool Bitpack_fitsu(uint64_t n, unsigned width)
{
  assert(width <= 64);

  uint64_t max = ~0;
  max = leftShift(max, width);
  max = ~max;

  if(n <= max) 
    return true;

  else
    return false;
}

bool Bitpack_fitss(int64_t n, unsigned width)
{
  assert(width <= 64);
  
  int64_t max;
  int64_t min = ~0;

  min = (int64_t) (leftShift(min, width -1));
  max = ~min;

  if(min <= n && n <= max)
    return true;
  else
    return false;
}

uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
  assert(width + lsb <= 64);

  uint64_t mask = ~0;
  mask = leftShift(mask, (64 - width));
  mask = logicalRightShift(mask, (64 - width - lsb));

  word = (word & mask);
  return logicalRightShift(word, lsb);
}

int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
  assert(width <= 64);
  assert(width + lsb <= 64);

  uint64_t mask = ~0;
  
  mask = leftShift(mask, (64 - width));
  mask = logicalRightShift(mask, (64 - width - lsb));
  
  word = (word & mask);
  word = leftShift(word, (64 - width - lsb));
  word = arithRightShift(word, (64 - width));

  return (int64_t)word;
}

uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
    uint64_t value)
{
  assert(width <= 64);
  assert(width + lsb <= 64);

  if(Bitpack_fitsu(value, width))
    word = bitpackReplace(word, width, lsb, value);
  
  else 
    RAISE(Bitpack_Overflow);
  
  return word;
}

uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,
    int64_t value)
{
  assert(width <= 64);
  assert(width + lsb <= 64);

  if(Bitpack_fitss(value, width))
    word = bitpackReplace(word, width, lsb, value);

  else
    RAISE(Bitpack_Overflow);

  return word;
}
