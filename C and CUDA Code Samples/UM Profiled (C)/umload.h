#include <stdio.h>
#include <stdlib.h>
#include "seg.h"

/**
 * Load a program into segmented memory and store it into
 * a Seg_T to be used by umexec
 * **/
extern Seg_T umLoadProg(FILE* fp);
