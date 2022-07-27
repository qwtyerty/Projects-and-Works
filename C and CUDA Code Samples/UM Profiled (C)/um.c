#include <assert.h>
#include "umload.h"
#include "umexec.h"

int main(int argc, char* argv[])
{
  //Check number of arguments, output to stderr if there was less
  if(argc != 2)
  {
    if(argc < 2)
      fprintf(stderr, "Too few arguments.\n Usage: %s <um file> \n", argv[0]);
    else
      fprintf(stderr, "Too few arguments.\n Usage: %s <um file> \n", argv[0]);

    exit(1);
  }

  //Prep the program storage and register storage
  Seg_T prog;
  uint32_t regs[8] = {0};

  //Open the file and ensure it exists, open to read a binary file
  FILE* infile = fopen(argv[1], "rb");
  if(infile == NULL)
  {
    fprintf(stderr, "Could not open file %s for readding.", argv[1]);
    exit(2);
  }

  //Load the program, assert it exists, then close the file
  prog = umLoadProg(infile);
  assert(prog);
  fclose(infile);

  //Execute the file then free the segmented memory
  umExecProg(prog, regs);
  Seg_free(prog);

  return 0;
}


/**Invariant documentations
 *
 *  SEG
 *    Invariant: Each Array_T stored with the sequence for each segment
 *    represents memory for the storage for the programs to be loaded into and
 *    read from
 *
 *  UMLOAD
 *    Initialization: The stores of segment are empty and contain no
 *      information.
 *    Maintenance: we know we are making progress as we are loading more and
 *      more segments and are nearing the end of the the um file.
 *    Termination: We know we are done storing the program into the segments
 *      when we have reached the end of the UM file
 *
 *  UMEXEC
 *    Initialization: We have a loaded program stored into segments.
 *    Maintenance: We know know tthat we are making progress when more and more
 *      instructions have been ran, and the program counter continues to
 *      increment.
 *    Termination: We know when to terminate when there are no more
 *      instructions to be ran or when a halt instruction has been called.
 *
 *  UM
 *    Initialization: The stores of segments are empty and contain no
 *      information
 *    Maintenance: We know we are making progress when segments have a program
 *      stored inside them and we are running instructions along with the
 *      program count continuing to increment
 *    Termination: We know when to terminate when there are no more
 *      instructions to be called or when we reach a halt instruction.
 *
 * **/
