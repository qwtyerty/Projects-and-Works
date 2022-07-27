#include "umload.h"
#include "bitpack.h"

extern Seg_T umLoadProg(FILE* fp)
{
  /**
   * Load a um program into segmented memory 
   * returns the segmented memory for use in 
   * umexec
   * **/

  //Get the file size by jumping to the end of the file
  //then return back to the beginning
  int curPos = ftell(fp);
  fseek(fp, 0L, SEEK_END);
  int fileSize = ftell(fp);
  fseek(fp, curPos, SEEK_SET);

  //Get the number of instructions, init memory and set an address
  int numInstr = fileSize / 4;
  Seg_T mem = Seg_new();
  uint32_t address = Seg_map(mem, numInstr);

  uint64_t instr = 0;
  uint64_t byte;
  //Loop through the number of instructions getting each instruction
  for(int i = 0; i < numInstr; ++i)
  {
    //Set the word back to 0
    instr = 0;
    for(int j = 3; j >= 0; j--)
    {
      //Get the byte and pack it into an instruction
      byte = getc(fp);
      instr = Bitpack_newu(instr, 8, (j * 8), byte);
    }

    //Store the instruction into segmented memory
    Seg_store(mem, (uint32_t)instr, address, i);
  }

  //Return the segment for execution
  return mem;
}
