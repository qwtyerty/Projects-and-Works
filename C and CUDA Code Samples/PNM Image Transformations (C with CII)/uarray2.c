#include <stdlib.h>
#include <string.h>
#include "assert.h"
#include "array.h"
#include "mem.h"
#include "uarray2.h"
#define T UArray2_T

//Unblocked Implementation of 2D Arrays

struct T{
	int height;
	int width;
	int size;
	Array_T array;
};

T UArray2_new(int width, int height, int size){
	assert(width > 0);
	assert(height > 0);
	assert(size > 0);
	T uarray;
	NEW(uarray);
	uarray -> array = Array_new(width*height, (sizeof(Array_T*)));
	uarray -> height = height;
	uarray -> width = width;
	uarray -> size = size;
	return uarray;
}

void UArray2_free(T *uarray){
	Array_free(&(*uarray) -> array);
  FREE(*uarray);
	return;
}

int UArray2_width(T uarray){
	return uarray -> width;
}

int UArray2_height(T uarray){
	return uarray -> height;
}

int UArray2_size(T uarray){
	return uarray -> size;
}

void *UArray2_at(T uarray, int x, int y){
	return Array_get(uarray->array, (y * uarray->width) + x);
}

void UArray2_map_col_major(T uarray, void apply(int x, int y, T uarray, void* elem, void *cl), void *cl){
	for(int i = 0; i < uarray -> width; i++){
		for(int j = 0; j < uarray -> height; j++){
			apply(i, j, uarray, UArray2_at(uarray, i,j), cl);
		}
	}
}

void UArray2_map_row_major(T uarray, void apply(int x, int y, T uarray, void* elem, void *cl), void *cl){
	for(int i = 0; i < uarray -> height; i++){
		for(int j = 0; j < uarray -> width; j++){
			apply(j, i, uarray, UArray2_at(uarray, j, i), cl);
		}
	}
}

