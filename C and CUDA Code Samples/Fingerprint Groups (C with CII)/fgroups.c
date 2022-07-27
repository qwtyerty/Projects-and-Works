#include <stdio.h>
#include <stdlib.h>
#include <table.h>
#include <list.h>
#include <ctype.h>
#include <str.h>
#include <string.h>
#include <atom.h>

// Searches for and finds all examples of repeated "fingerprints" using
// Hansens CII Library

// Set to a milliion to prepare for several hundred thousand lines
const int MAX_SIZE = 1000000;

void printNames(const void* key, void** value, void* cl);
void printList(void** x, void* cl);
void* addToTable(Table_T* table, char* fp, char* name);
void readFile(FILE* file, Table_T table);
void findFGroups(FILE* file);

struct NewLineState {
  int state;
};

int main(int argc, char* argv[])
{
  //Check for Valid input
  if(argc == 1)
    findFGroups(stdin);
  else
  {
    fprintf(stderr, "%s: No file given via standard input, Halting...", argv[0]);
      exit(1);
  }

  fclose(stdin);
  return 0;
}

void findFGroups(FILE* file)
{
  if(file == NULL)
  {
    fprintf(stderr, "Could not open file for reading, Halting...");
    exit(2);
  }
  Table_T fPrintTable = Table_new(0, NULL, NULL);
  readFile(file, fPrintTable);


}

void readFile(FILE* file, Table_T table)
{
  // Initialize strings for looping (Assuming both FP and name are <= 512 bytes)
  char str[1024];
  char* fp; 
  char* name;

  // Although char is one byte, I feel sizeof(char) makes it clearer
  while((fgets(str, 1024 * sizeof(char), file)) != NULL)
  {
    // Get first token which is the fingerprint
    fp = strtok(str, " ");

    // Loop over any possible white space (compare char to make sure it is not
    //white space)
    while((name = strtok(NULL, " "))[0] == ' ')
      continue;
    // Parts found add it to tables
    table = *((Table_T*) addToTable(&table, fp, name));
  }

  struct NewLineState  state = {.state = 0};
  Table_map(table, printNames, &state);

  //Now that we're done with the table free it
  Table_free(&table);
  return;
}

void* addToTable(Table_T* fPrintTable, char* fp, char* name)
{
  const char* atomFP = Atom_string(fp);
  const char* atomName = Atom_string(name);
  // Check if there is a list at the given key
  List_T tableValue = Table_get(*fPrintTable, atomFP);
  if(tableValue == NULL)
  {
    // No key, add a new entry
    List_T printGroup = List_list((char *) atomName, NULL);
    // Add to the table
    Table_put(*fPrintTable, (char*) atomFP, printGroup);
  }
  else
  {
    // Key exists just append to the list and update the table
    List_T tail = List_list((char *) atomName, NULL);
    List_T printGroup = List_append(tableValue, tail);
    Table_put(*fPrintTable, (char *) atomFP, printGroup);
  }
  return fPrintTable;
}

void printNames(const void* key, void** value, void* cl)
{
  const char* fp = (char*) key;
  struct NewLineState* close = cl;
  (void) fp;
  //Test if there are more than two names then print them and then and endl
  if(List_length(*value) > 1)
  {
    if(close->state == 0)
      close->state += 1;
    else
    {
      printf("\n");
    }
    
    List_map(*value, printList, cl);

    //Free after use
    List_free(*value);
  }
  else
    List_free(*value);

  return;
}

void printList(void** x, void* cl)
{
  char* nameStr = *x;
  printf("%s", nameStr);
  (void) cl;
  return;
}
