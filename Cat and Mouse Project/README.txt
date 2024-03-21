The source code for the Project is located in the "Cat and Mouse" folder
Additionally, a pre-built executible is within the following location:
Cat and Mouse > x64 > debug 

The program takes in one argument, which is the location of a maze csv file.

The program expects to receive a csv file as input in the following format:

width, height
MAZE_DATA, MAZE_DATA, MAZE_DATA, ...
MAZE_DATA, MAZE_DATA, MAZE_DATA, ...
MAZE_DATA, MAZE_DATA, MAZE_DATA, ...
etc

Additionally, data should be a number from 0-4 which represents the following:
0 = Empty Tile
1 = Wall Tile
2 = Mouse Tile
3 = Cat Tile
4 = Cheese Tile

Currently the program expects there to be only one cat and one mouse, however the program can work with multiple cheese location.
- Kevin Aldana