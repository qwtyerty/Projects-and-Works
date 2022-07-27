#include <assert.h>
#include <math.h>
#include "umexec.h"
#include "bitpack.h"


/**
 * All instructions
 * **/

static void condMove(unsigned* regs, unsigned A, unsigned B, unsigned C);
static void segLoad(unsigned* regs, unsigned A, unsigned B, unsigned C, Seg_T prog);
static void segStore(unsigned* regs, unsigned A, unsigned B, unsigned C, Seg_T prog);
static void add(unsigned* regs, unsigned A, unsigned B, unsigned C);
static void multiply(unsigned* regs, unsigned A, unsigned B, unsigned C); 
static void division(unsigned* regs, unsigned A, unsigned B, unsigned C);
static void bitNand(unsigned* regs, unsigned A, unsigned B, unsigned C);
static void halt(Seg_T prog);
static void mapSeg(unsigned* regs,  unsigned B, unsigned C, Seg_T prog);
static void unmapSeg(unsigned* regs, unsigned C, Seg_T prog);
static void output(unsigned* regs, unsigned C);
static void input(unsigned* regs, unsigned C);
static void loadProg(unsigned* regs, unsigned B, unsigned C, Seg_T prog, 
    unsigned* progCtr);
static void loadVal(unsigned* regs, unsigned A, uint32_t val);


/**
 * Instruction struct
 * **/
typedef struct Instruction
{
  unsigned opcode;
  unsigned regA;
  unsigned regB;
  unsigned regC;
} Instruction;


/**
 * Helper Functions
 * **/

static Instruction extractInstruction(uint32_t word, Instruction instr);
static void runInstruction(Instruction instr, unsigned* regs, Seg_T prog,
    unsigned* progCtr);



/**
 * Definitions
 * **/


extern void umExecProg(Seg_T prog, unsigned* regs)
{
  /**
   * Executes a program loaded into segmented memory,
   * executes all instructions until there are no more instructions
   * or a halt instruction is hit**/
  
  // Initialize the word, instructions and the program counter
  uint32_t word;
  Instruction instr;
  unsigned progCtr = 0;

  // Execution Loop
  while(true)
  {
    // Load an instruction and extract it
    word = Seg_load(prog, 0, progCtr);
    instr = extractInstruction(word, instr);

    // Run it and incrrement the counter
    runInstruction(instr, regs, prog, &progCtr);
    ++progCtr;
  }
}

static Instruction extractInstruction(uint32_t word, Instruction instr)
{
  /**
   * Unpack instruction and put them into and instruction struct and 
   * returns it for the execution loop
   * **/

  instr.opcode = Bitpack_getu(word, 4, 28);
  
  //Prep appropriate registers for the instruction
  if(instr.opcode == 13)
  {
    instr.regA = Bitpack_getu(word, 3, 25);
    instr.regB = Bitpack_getu(word, 25, 0);
  }
  else
  {
    instr.regA = Bitpack_getu(word, 3, 6);
    instr.regB = Bitpack_getu(word, 3, 3);
    instr.regC = Bitpack_getu(word, 3, 0);
  }

  // Send it for execution
  return instr;
}


static void runInstruction(Instruction instr, unsigned* regs, Seg_T prog,
    unsigned* progCtr)
{
  /**
   * Helper function that directs an instruction call to the proper function
   * using the instruction's opcode
   * **/

  //Get the opcode and ensure that it is valid
  unsigned opcode = instr.opcode;
  
  //Switch based on opcodes
  switch(opcode)
  {
    case 0:
      condMove(regs, instr.regA, instr.regB, instr.regC);
      break;

    case 1:
      segLoad(regs, instr.regA, instr.regB, instr.regC,
          prog);
      break;

    case 2:
      segStore(regs, instr.regA, instr.regB, instr.regC,
          prog);
      break;

    case 3:
      add(regs, instr.regA, instr.regB, instr.regC);
      break;

    case 4: 
      multiply(regs, instr.regA, instr.regB, instr.regC);
      break;

    case 5: 
      division(regs, instr.regA, instr.regB, instr.regC);
      break;

    case 6:
      bitNand(regs, instr.regA, instr.regB, instr.regC);
      break;

    case 7:
      halt(prog);
      break;

    case 8:
      mapSeg(regs, instr.regB, instr.regC, prog);
      break;

    case 9:
      unmapSeg(regs, instr.regC, prog);
      break;

    case 10:
      output(regs, instr.regC);
      break;

    case 11:
      input(regs, instr.regC);
      break;

    case 12:
      loadProg(regs, instr.regB, instr.regC, prog, progCtr);
      break;

    case 13:
      loadVal(regs, instr.regA, (uint32_t)instr.regB);
      break;

    default:
      exit(1);

  }

}

static void condMove(uint32_t* regs, unsigned A, unsigned B, unsigned C)
{
  /* 
   * Apply a condition move based on register C, then moves 
   * register B into register A
   *  */

  if(regs[C] != 0)
    regs[A] = regs[B];
}

static void segLoad(uint32_t* regs, unsigned A, unsigned B, unsigned C, Seg_T prog)
{
  /**
   * Load a segment into register A
   * **/

  regs[A] = Seg_load(prog, regs[B], regs[C]);
}

static void segStore(uint32_t* regs, unsigned A, unsigned B, unsigned C, Seg_T prog)
{
  /**
   * Store a segment using Seg_store
   * **/

  Seg_store(prog, regs[C], regs[A], regs[B]);
}

static void add(uint32_t* regs, unsigned A, unsigned B, unsigned C)
{
  /**
   * Add register B and C, then store the result in register A
   * **/

  regs[A] = regs[B] + regs[C];
}

static void multiply(uint32_t* regs, unsigned A, unsigned B, unsigned C)
{
  /**
   * Multiply register B and C and store the result in register A
   * **/

  regs[A] = regs[B] * regs[C];
}

static void division(uint32_t* regs, unsigned A, unsigned B, unsigned C)
{
  /**
   * Divide register B by register C and store the result in register A
   * **/
  regs[A] = regs[B] / regs[C];
}

static void bitNand(uint32_t* regs, unsigned A, unsigned B, unsigned C)
{
  /**
   * Take the bitwise NAND of registers B and C then store the results in 
   * register A
   * **/
  regs[A] = ~(regs[B] & regs[C]);
}

static void halt(Seg_T prog)
{
  /**
   * Terminate the program, by freeing the segmented memory and then 
   * terminating the program with exit code 0
   * **/
  Seg_free(prog);
  exit(0);
}

static void mapSeg(uint32_t* regs, unsigned B, unsigned C, Seg_T prog)
{
  /**
   * Makes a new segment with a number of words equal to register C,
   * and maps it into register B
   * **/
  regs[B] = Seg_map(prog, regs[C]);
}

static void unmapSeg(uint32_t* regs, unsigned C, Seg_T prog)
{
  /**
   * Unmap segmented memory**/
  Seg_unmap(prog, regs[C]);
}

static void output(uint32_t* regs, unsigned C)
{
  /**
   * Output a single character to stdout
   * **/
  printf("%c", regs[C]);
}

static void input(uint32_t* regs, unsigned C)
{
  /**
   * Get a single character from stdin and put it into 
   * register C
   * **/
  regs[C] = fgetc(stdin);
}

static void loadProg(uint32_t* regs, unsigned B, unsigned C, Seg_T prog,
    unsigned* progCtr)
{
  /**
   * Loads a program, which effectively acts as a jump routine
   * **/
  if(regs[B] != 0)
    Seg_load_prog(prog, regs[B]);

  *progCtr = regs[C] -1;
}

static void loadVal(uint32_t * regs, unsigned A, uint32_t val)
{
  /**
   * Loads a value into register A
   * **/
  regs[A] = val;
}
