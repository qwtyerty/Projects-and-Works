#include <stdio.h>
#include <stdlib.h>
#include "seg.h"

/**
 * Executes a program that is loaded into segemnted memory and 
 * executes all instructions until either there are no more instructions
 * or a halt instruction is called
 * **/
extern void umExecProg(Seg_T prog, unsigned* regs);
