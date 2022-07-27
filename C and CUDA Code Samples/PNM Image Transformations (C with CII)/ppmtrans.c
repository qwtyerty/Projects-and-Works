#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"

// Defines transformations for pnm files using A2Methods which implements a
// 2D Array

//Helper functions to pass to more specific functions
Pnm_ppm transformImage(int rot, A2Methods_T methods, Pnm_ppm pixelMap,
    A2Methods_mapfun* map);

//Handles either transforms with the a same or different dimensions
Pnm_ppm transSameDim(int rot, A2Methods_T methods, Pnm_ppm pixelMap,
    A2Methods_mapfun* map, A2Methods_Array2 orig);
Pnm_ppm transDiffDim(int rot, A2Methods_T methods, Pnm_ppm pixelMap,
    A2Methods_mapfun* map, A2Methods_Array2 orig);


//Functions to actually apply the transformations (mapping funcs)
void rot90(int i, int j, A2Methods_Array2 array2,
    A2Methods_Object* elem, void* cl);
void rot180(int i, int j, A2Methods_Array2 array2,
    A2Methods_Object* elem, void* cl);
void rot270(int i, int j, A2Methods_Array2 array2,
    A2Methods_Object* elem, void* cl);
void flipV(int i, int j, A2Methods_Array2 array2,
    A2Methods_Object* elem, void* cl);
void flipH(int i, int j, A2Methods_Array2 array2,
    A2Methods_Object* elem, void* cl);
void trans(int i, int j, A2Methods_Array2 array2,
    A2Methods_Object* elem, void* cl);

//Constants for other "roations"
const int VERTICAL = 1;
const int HORIZONTAL = 2;
const int TRANSPOSE = 3;


int main(int argc, char *argv[]) {
  int rotation = 0;
  A2Methods_T methods = array2_methods_plain; // default to UArray2 methods
  assert(methods);
  A2Methods_mapfun *map = methods->map_default; // default to best map
  assert(map);
#define SET_METHODS(METHODS, MAP, WHAT) do { \
      methods = (METHODS); \
      assert(methods); \
      map = methods->MAP; \
      if (!map) { \
        fprintf(stderr, "%s does not support " WHAT "mapping\n", argv[0]); \
        exit(1); \
      } \
    } while(0)

  int i;
  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-row-major")) {
      SET_METHODS(array2_methods_plain, map_row_major, "row-major");
    } else if (!strcmp(argv[i], "-col-major")) {
      SET_METHODS(array2_methods_plain, map_col_major, "column-major");
    } else if (!strcmp(argv[i], "-block-major")) {
      SET_METHODS(array2_methods_blocked, map_block_major, "block-major");
    } else if (!strcmp(argv[i], "-rotate")) {
      assert(i + 1 < argc);
      char *endptr;
      rotation = strtol(argv[++i], &endptr, 10);
      assert(*endptr == '\0'); // parsed all correctly
      assert(rotation == 0   || rotation == 90
          || rotation == 180 || rotation == 270);
    }
    else if (!strcmp(argv[i], "-flip"))
    {
      assert(i + 1 < argc);
      if(!strcmp(argv[++i], "vertical"))
        rotation = VERTICAL;
      else if(!strcmp(argv[i], "horizontal"))
        rotation = HORIZONTAL;
      else
        assert(0);
    }
    else if (!strcmp(argv[i], "-transpose"))
    {
      rotation = TRANSPOSE;
    }
    else if (*argv[i] == '-') {
      fprintf(stderr, "%s: unknown option '%s'\n", argv[0], argv[i]);
      exit(1);
    } else if (argc - i > 2) {
      fprintf(stderr, "Usage: %s [-rotate <angle>] "
              "[-{row,col,block}-major] [filename]\n", argv[0]);
      exit(1);
    } else {
      break;
    }
  }
  //Base line assume it is in stdin
  FILE* file = stdin;
  
  //Remainder in argc, there is a file
  if(argc > i)
  {
    file = fopen(argv[i], "r");
    assert(file);
  }

  //Read, transform, then write the file
  Pnm_ppm pixelMap = Pnm_ppmread(file, methods);
  pixelMap = transformImage(rotation, methods, pixelMap, map);
  Pnm_ppmwrite(stdout, pixelMap);

  //Close the file if it is from commandline instead of stdin
  if(argc > i)
  {
    fclose(file);
  }
  Pnm_ppmfree(&pixelMap);
  exit(0);
}

Pnm_ppm transformImage(int rot, A2Methods_T methods, Pnm_ppm pixelMap,
    A2Methods_mapfun *map)
{
  /**
   * Handle Transformations and handle passing to functions
   * **/

  //No rotation we can just return
  if(rot == 0)
  {
    return pixelMap;
  }
  
  //Since we need to modify we need a copy
  A2Methods_Array2 orig = pixelMap->pixels;

  //Dims change pass to function
  if(rot == 90 || rot == 270)
  {
    return transDiffDim(rot, methods, pixelMap, map, orig);
  }
  else
  {
    return transSameDim(rot, methods, pixelMap, map, orig);
  }

}

Pnm_ppm transDiffDim(int rot, A2Methods_T methods, Pnm_ppm pixelMap, 
    A2Methods_mapfun *map, A2Methods_Array2 orig)
{
  /**
   * Handles 90 and 270 Rotations which change dimensions
   * **/
  
  //Fill the new map
  pixelMap->pixels = methods->new_with_blocksize(methods->height(orig),
      methods->width(orig), methods->size(orig), methods->blocksize(orig));

  //Apply proper rotation
  if(rot == 90)
    map(orig, rot90, pixelMap);
  else
    map(orig, rot270, pixelMap);

  //Swap dims
  int width = pixelMap->width;
  pixelMap->width = pixelMap->height;
  pixelMap->height = width;

  //Free copy and return
  methods->free(&orig);
  return pixelMap;
}

Pnm_ppm transSameDim(int rot, A2Methods_T methods, Pnm_ppm pixelMap,
    A2Methods_mapfun *map, A2Methods_Array2 orig)
{
  /**
   * Handles all other trasnsformations that keep dims
   * **/

  //Fill the new map
  pixelMap->pixels = methods->new_with_blocksize(methods->width(orig),
      methods->height(orig), methods->size(orig), methods->blocksize(orig));
  
  //Apply the Transformation
  if(rot == 180)
    map(orig, rot180, pixelMap);
  else if(rot == VERTICAL)
    map(orig, flipV, pixelMap);
  else if(rot == HORIZONTAL)
    map(orig, flipH, pixelMap);
  else
    map(orig, trans, pixelMap);
  
  //Fre and return
  methods->free(&orig);
  return pixelMap;
  
}
void rot90(int i, int j, A2Methods_Array2 array2,
    A2Methods_Object* elem, void* cl)
{
  /**
   * Rotate 90 degrees
   * **/

  //warning supression
  (void) array2;

  //Get the map
  Pnm_ppm pixelMap = (Pnm_ppm) cl;
  
  //Use geometric property
  int newColPos = pixelMap->height - j - 1;
  int newRowPos = i;
  
  //Copy into new possition
  *(Pnm_rgb)(pixelMap->methods->
      at(pixelMap->pixels, newColPos, newRowPos)) = *(Pnm_rgb) elem;
}

void rot180(int i, int j, A2Methods_Array2 array2,
    A2Methods_Object* elem, void* cl)
{
  /**
   * rotate 190 degrees
   * **/

  //Warning supress
  (void) array2;

  //Get the map
  Pnm_ppm pixelMap = (Pnm_ppm) cl;
  
  //Get new positions
  int newColPos = pixelMap->width - i - 1;
  int newRowPos = pixelMap->height - j - 1;

  //Copy it in
  *(Pnm_rgb)(pixelMap->methods->
      at(pixelMap->pixels, newColPos, newRowPos)) = *(Pnm_rgb) elem;
}

void rot270(int i, int j, A2Methods_Array2 array2,
    A2Methods_Object* elem, void* cl)
{
  /**
   * rotate 270 degrees
   * **/

  //Supress
  (void) array2;
  
  Pnm_ppm pixelMap = (Pnm_ppm) cl;
  
  //get new pos
  int newColPos = j;
  int newRowPos = pixelMap-> width - i - 1;

  //Copy it in
  *(Pnm_rgb)(pixelMap->methods->
      at(pixelMap->pixels, newColPos, newRowPos)) = *(Pnm_rgb) elem;
}

void flipV(int i, int j, A2Methods_Array2 array2,
    A2Methods_Object* elem, void* cl)
{
  /**
   * Flips Vertically
   * **/

  //Supress
  (void) array2;

  //Copy in and assign new positions
  Pnm_ppm pixelMap = (Pnm_ppm) cl;
  int newColPos = i;
  int newRowPos = pixelMap->height - j - 1;

  //Copy into it
  *(Pnm_rgb)(pixelMap->methods->
      at(pixelMap->pixels, newColPos, newRowPos)) = *(Pnm_rgb) elem;

}

void flipH(int i, int j, A2Methods_Array2 array2,
    A2Methods_Object* elem, void* cl)
{
  /**
   *  Flips Horizontally
   * **/

  //Supress and get data
  (void) array2;
  Pnm_ppm pixelMap = (Pnm_ppm) cl;

  //New poses
  int newColPos = pixelMap->width - i - 1;
  int newRowPos = j;

  //Assign them
  *(Pnm_rgb)(pixelMap->methods->
      at(pixelMap->pixels, newColPos, newRowPos)) = *(Pnm_rgb) elem;
}

void trans(int i, int j, A2Methods_Array2 array2,
    A2Methods_Object* elem, void* cl)
{
  /**
   * Transposes
   * **/

  //Supress and get map
  (void) array2;
  Pnm_ppm pixelMap = (Pnm_ppm) cl;

  //Get Pos
  int newColPos = j;
  int newRowPos = i;

  //Assign it
  *(Pnm_rgb)(pixelMap->methods->
      at(pixelMap->pixels, newColPos, newRowPos)) = *(Pnm_rgb) elem;
}

