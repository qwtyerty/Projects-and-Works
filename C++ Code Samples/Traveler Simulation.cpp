
#include <unistd.h>
#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <pthread.h>
#include "gl_frontEnd.h"
//
#include "gl_frontEnd.h"

using namespace std;


/**
	Backend for a snake-esque simulation of "travelers" who move on a grid based system,
		with p_threads handling the storage of information and moves applied. Synchronizes
		"generations" by checking the completion of each thread once they are all completed
		signal to start the next generation. Store the needed information an pass it to the open_gl
		front end.
**/

//==================================================================================
//	Function prototypes
//==================================================================================
void initializeApplication(void);
GridPosition getNewFreePosition(void);
Direction newDirection(Direction forbiddenDir = NUM_DIRECTIONS);
TravelerSegment newTravelerSegment(const TravelerSegment& currentSeg, bool& canAdd);
void generateWalls(void);
void generatePartitions(void);
void applyMove(Traveler* traveler,Direction dir);
void moveSegments(Traveler* traveler, int index, Direction dir);
Direction getMove(const Traveler* traveler);
void* threadFunc(void*);
//==================================================================================
//	Application-level global variables
//==================================================================================

//	Don't rename any of these variables
//-------------------------------------
//	The state grid and its dimensions (arguments to the program)
SquareType** grid;
unsigned int numRows = 0;	//	height of the grid
unsigned int numCols = 0;	//	width
unsigned int numTravelers = 0;	//	initial number
unsigned int numTravelersDone = 0;
unsigned int numLiveThreads = 0;		//	the number of live traveler threads
unsigned int numMoves = 0;
vector<Traveler> travelerList;
vector<SlidingPartition> partitionList;
GridPosition	exitPos;	//	location of the exit

//	travelers' sleep time between moves (in microseconds)
const int MIN_SLEEP_TIME = 1000;
int travelerSleepTime = 100000;

//	Dont change the dimensions as this may break the front end
const int MAX_NUM_MESSAGES = 8;
const int MAX_LENGTH_MESSAGE = 32;
char** message;
time_t launchTime;

//	Random generators:  For uniform distributions
const unsigned int MAX_NUM_INITIAL_SEGMENTS = 6;
random_device randDev;
default_random_engine engine(randDev());
uniform_int_distribution<unsigned int> unsignedNumberGenerator(0, numeric_limits<unsigned int>::max());
uniform_int_distribution<unsigned int> segmentNumberGenerator(0, MAX_NUM_INITIAL_SEGMENTS);
uniform_int_distribution<unsigned int> segmentDirectionGenerator(0, NUM_DIRECTIONS-1);
uniform_int_distribution<unsigned int> headsOrTails(0, 1);
uniform_int_distribution<unsigned int> rowGenerator;
uniform_int_distribution<unsigned int> colGenerator;

//Struct Definition
typedef struct ThreadInfo
{
	//	you probably want these
	pthread_t threadID;
	int threadIndex;
	Traveler threadTraveler;
	bool doneGen;
	bool complete;

} ThreadInfo;

//Other global declarations
unsigned int finishedThreads;
ThreadInfo* thread_array;
pthread_mutex_t onlyLock;


//Renedering function for travelers, also acts as a way to check if a level is done 
void drawTravelers(void)
{
	//-----------------------------
	//	You may have to sychronize things here
	//-----------------------------
	// Global Lock
	pthread_mutex_lock(&onlyLock);
	//Check to see if every thread is done travelling
	if (finishedThreads == numTravelers)
	{
		//Join the threads
		for(unsigned int i = 0; i < numTravelers; i++)
		{
			pthread_join(thread_array[i].threadID, NULL);
			numLiveThreads--;
		}
		//Reset global tracking variables, Create new level
		finishedThreads = 0;
		cout << "Starting new level..." << endl;
		initializeApplication();
	}
	//If the level isn't done continue rendering as normal
	else
	{
		//Draw each thread if it's not done already, and if it's finished it's work for the generation
		for (int i = 0; i < numTravelers; i++){
			if (thread_array[i].doneGen == true && thread_array[i].complete == false)
			{
				drawTraveler(thread_array[i].threadTraveler);
				thread_array[i].doneGen = false;
				usleep(10000);
			}
		}
	}
	//Global Unlock
	pthread_mutex_unlock(&onlyLock);
}

//Thread Function, does all the work of a traveler
void* threadFunc(void* arg)
{			
	ThreadInfo* sthread = (ThreadInfo*) arg;
	//Only work when not done with the level
	while (sthread->complete == false)
	{
		usleep(1000);
		//Lock
		pthread_mutex_lock(&onlyLock);
		//Only work when not done with the generation
		while (sthread->doneGen == false)
		{
			Direction move;
			//Only move if traveler isn't at an exit
			if (sthread->threadTraveler.exitStatus == false)
			{
				//Get proper move if not exiting
				move = getMove(&sthread->threadTraveler);
				//Traveler only moves if it gets a direction
				if (move != NO_DIRECTION)
				{
					//Apply the move
					applyMove(&sthread->threadTraveler, move);
				}
		    }
			//Test to see if the head is on an exit, when exiting this code should be ran everytime it moves
			if(grid[sthread->threadTraveler.segmentList[0].row][sthread->threadTraveler.segmentList[0].col] == EXIT)
			{
				if (sthread->threadTraveler.segmentList.size() == 1)
				{
					//Delete the head and declare thread as done with the level
					sthread->threadTraveler.segmentList.erase(sthread->threadTraveler.segmentList.begin() + sthread->threadTraveler.segmentList.size() - 1);
					finishedThreads++;
					sthread->complete = true;
				}
				else
				{
					//Delete the last segment on the tail
					sthread->threadTraveler.segmentList.erase(sthread->threadTraveler.segmentList.begin() + sthread->threadTraveler.segmentList.size() - 1);
					sthread->threadTraveler.exitStatus = true;
				}
			}
			//Work done for this generation
			sthread->doneGen = true;
		}	
		//Unlock
		pthread_mutex_unlock(&onlyLock);
	}
	//Unlock condition if work is finished
	cout << "Thread " << sthread->threadIndex << " finished" << endl;
	pthread_mutex_unlock(&onlyLock);
	return NULL;
}

//Gets the travelers next move by checking legal moves, returns it as a Direction
Direction getMove(const Traveler* traveler)
{
	unsigned int x = traveler->segmentList[0].col;
	unsigned int y = traveler->segmentList[0].row;
	Direction ori = traveler->segmentList[0].dir;

	vector <Direction> possibleMoves;

	if (ori == NORTH)
	{
		//Check West Tile
		if ((x-1 > 0) && (grid[y][x - 1] == FREE_SQUARE || grid[y][x - 1] == EXIT))
		{
			if (grid[y][x - 1] == EXIT)
				return WEST;
			else
				possibleMoves.push_back(WEST);
		}

		//Check East Tile
		if ((x+1 < numCols - 1) && (grid[y][x + 1] == FREE_SQUARE || grid[y][x + 1] == EXIT))
		{
			if (grid[y][x + 1] == EXIT)
				return EAST;
			else
				possibleMoves.push_back(EAST);
		}

		//Check North
		if ((y-1 > 0) && (grid[y-1][x] == FREE_SQUARE || grid[y-1][x] == EXIT))
		{
			if (grid[y-1][x] == EXIT)
				return NORTH;
			else
				possibleMoves.push_back(NORTH);
		}
	}

	if (ori == SOUTH)
	{
        	//Check West Tile
		if ((x-1 > 0) && (grid[y][x - 1] == FREE_SQUARE || grid[y][x - 1] == EXIT))
		{
			if (grid[y][x - 1] == EXIT)
				return WEST;
			else
				possibleMoves.push_back(WEST);
		}
		//Check East Tile
		if ((x+1 < numCols) && (grid[y][x + 1] == FREE_SQUARE || grid[y][x + 1] == EXIT))
		{
			if (grid[y][x + 1] == EXIT)
				return EAST;
		
			else
				possibleMoves.push_back(EAST);
		}

		//Check South
		if ((y+1 < numRows) && (grid[y+1][x] == FREE_SQUARE || grid[y+1][x] == EXIT))
		{
			if (grid[y+1][x] == EXIT)
				return SOUTH;
			else
				possibleMoves.push_back(SOUTH);
		}
        }
	
	if (ori == EAST)
	{
		//Check East Tile
		if ((x+1 < numCols - 1) && (grid[y][x + 1] == FREE_SQUARE || grid[y][x + 1] == EXIT))
		{
			if (grid[y][x + 1] == EXIT)
				return EAST;
			else
				possibleMoves.push_back(EAST);
		}

		//Check North
		if ((y-1 > 0) && (grid[y-1][x] == FREE_SQUARE || grid[y-1][x] == EXIT))
		{
			if (grid[y-1][x] == EXIT)
				return NORTH;
			else
				possibleMoves.push_back(NORTH);
		}

		//Check South
		if ((y+1 < numRows) && (grid[y+1][x] == FREE_SQUARE || grid[y+1][x] == EXIT))
		{
			if (grid[y+1][x] == EXIT)
				return SOUTH;
			else
				possibleMoves.push_back(SOUTH);
		}
	}

	if (ori == WEST)
	{
		//Check West Tile
		if ((x-1 > 0) && (grid[y][x - 1] == FREE_SQUARE || grid[y][x - 1] == EXIT))
		{
			if (grid[y][x - 1] == EXIT)
				return WEST;
			else
				possibleMoves.push_back(WEST);    
		}

		//Check North
		if ((y-1 > 0) && (grid[y-1][x] == FREE_SQUARE || grid[y-1][x] == EXIT))
		{
			if (grid[y-1][x] == EXIT)
				return NORTH;
			else
				possibleMoves.push_back(NORTH);
	}

		//Check South
		if ((y+1 < numRows) && (grid[y+1][x] == FREE_SQUARE || grid[y+1][x] == EXIT))
		{
			if (grid[y+1][x] == EXIT)
				return SOUTH;
			else
				possibleMoves.push_back(SOUTH);
		}
	}

	//Randomly pick a move
	int len = possibleMoves.size();
	if (len == 0)
		return NO_DIRECTION;
	else
	{
		switch (len)
		{
			case 1:
				return possibleMoves[0];
				break;
			case 2:
				return possibleMoves[headsOrTails(engine)];
				break;
			case 3:
				uniform_int_distribution<unsigned int> randoo(0, 2);
				return possibleMoves[randoo(engine)];
				break;
		}
	}
	
	//If something goes wrong then return NO_DIRECTION
	return NO_DIRECTION;
}

void updateMessages(void)
{
	//	Here I hard-code a few messages that I want to see displayed
	//	in my state pane.  The number of live robot threads will
	//	always get displayed.  No need to pass a message about it.
	unsigned int numMessages = 3;
	sprintf(message[0], "We created %d travelers", numTravelers);
	sprintf(message[1], "%d travelers solved the maze", numTravelersDone);
	sprintf(message[2], "Simulation run time is %ld", time(NULL)-launchTime);
	drawMessages(numMessages, message);
}

void handleKeyboardEvent(unsigned char c, int x, int y)
{
	int ok = 0;

	switch (c)
	{
		//	'esc' to quit
		case 27:
			exit(0);
			break;

		//	slowdown
		case ',':
			slowdownTravelers();
			ok = 1;
			break;

		//	speedup
		case '.':
			speedupTravelers();
			ok = 1;
			break;

		default:
			ok = 1;
			break;
	}
	if (!ok)
	{
		//	do something?
	}
}



void speedupTravelers(void)
{
	//	decrease sleep time by 20%, but don't get too small
	int newSleepTime = (8 * travelerSleepTime) / 10;
	
	if (newSleepTime > MIN_SLEEP_TIME)
	{
		travelerSleepTime = newSleepTime;
	}
}

void slowdownTravelers(void)
{
	//	increase sleep time by 20%
	travelerSleepTime = (12 * travelerSleepTime) / 10;
}





int main(int argc, char** argv)
{
	//Basic command line argument checking
	if(argc < 4 || argc > 5)
	{
		cout << "Usage: <width> <height> <number of travelers> <Optional: number of moves after which a traveler should grow a new segment>" << endl;
		exit(1);
	}
	stringstream ss;
	if(argc == 4)
	{
		ss << argv[1] << " " << argv[2] << " " << argv[3];
		ss >> numCols >> numRows >> numTravelers;
		cout << numRows << " " << numCols << " " <<numTravelers <<" "<< endl;
		numMoves = INT_MAX;
	}
	else
	{
		ss << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4];
		ss >> numCols >> numRows >> numTravelers >> numMoves;
		cout << numRows << " " << numCols << " " <<numTravelers <<" "<< numMoves << endl;
	}


	//	We know that the arguments  of the program  are going
	//	to be the width (number of columns) and height (number of rows) of the
	//	grid, the number of travelers, etc.
	//	So far, I hard code-some values
	numLiveThreads = 0;
	numTravelersDone = 0;
	finishedThreads = 0;
	//	Even though we extracted the relevant information from the argument
	//	list, I still need to pass argc and argv to the front-end init
	//	function because that function passes them to glutInit, the required call
	//	to the initialization of the glut library.
	initializeFrontEnd(argc, argv);
	
	//	Now we can do application-level initialization
	initializeApplication();

	launchTime = time(NULL);

	
	//	Now we enter the main loop of the program and to a large extend
	//	"lose control" over its execution.  The callback functions that 
	//	we set up earlier will be called when the corresponding event
	//	occurs
	glutMainLoop();
	
	//	Free allocated resource before leaving (not absolutely needed, but
	//	just nicer.  Also, if you crash there, you know something is wrong
	for (unsigned int i=0; i< numRows; i++)
		free(grid[i]);
	free(grid);
	for (int k=0; k<MAX_NUM_MESSAGES; k++)
		free(message[k]);
	free(message);
	
	//	This will probably never be executed (the exit point will be in one of the
	//	call back functions).
	return 0;
}


void initializeApplication(void)
{
	//	Initialize some random generators
	rowGenerator = uniform_int_distribution<unsigned int>(0, numRows-1);
	colGenerator = uniform_int_distribution<unsigned int>(0, numCols-1);

	//	Allocate the grid
	grid = new SquareType*[numRows];
	for (unsigned int i=0; i<numRows; i++)
	{
		grid[i] = new SquareType[numCols];
		for (unsigned int j=0; j< numCols; j++)
			grid[i][j] = FREE_SQUARE;
		
	}

	message = new char*[MAX_NUM_MESSAGES];
	for (unsigned int k=0; k<MAX_NUM_MESSAGES; k++)
		message[k] = new char[MAX_LENGTH_MESSAGE+1];
		
	srand((unsigned int) time(NULL));

	//	generate a random exit (change back to 1 in final version)
	for (int i = 0; i < 1; i++)
	{
		exitPos = getNewFreePosition();
		grid[exitPos.row][exitPos.col] = EXIT;
	}
	//	Generate walls and partitions
	generateWalls();
	generatePartitions();
	
	//	Initialize traveler info structs
	//	You will probably need to replace/complete this as you add thread-related data
	float** travelerColor = createTravelerColors(numTravelers);
	for (unsigned int k=0; k<numTravelers; k++) {
		GridPosition pos = getNewFreePosition();
		
		Direction dir = static_cast<Direction>(segmentDirectionGenerator(engine));

		TravelerSegment seg = {pos.row, pos.col, dir};
		Traveler traveler;
		traveler.segmentList.push_back(seg);
		grid[pos.row][pos.col] = TRAVELER;

		//	I add 0-n segments to my travelers
		unsigned int numAddSegments = segmentNumberGenerator(engine);
		TravelerSegment currSeg = traveler.segmentList[0];
		bool canAddSegment = true;
cout << "Traveler " << k << " at (row=" << pos.row << ", col=" <<
		pos.col << "), direction: " << dirStr(dir) << ", with up to " << numAddSegments << " additional segments" << endl;
cout << "\t";

		for (unsigned int s=0; s<numAddSegments && canAddSegment; s++)
		{
			TravelerSegment newSeg = newTravelerSegment(currSeg, canAddSegment);
			if (canAddSegment)
			{
				traveler.segmentList.push_back(newSeg);
				currSeg = newSeg;
cout << dirStr(newSeg.dir) << "  ";
			}
		}
cout << endl;

		for (unsigned int c=0; c<4; c++)
			traveler.rgba[c] = travelerColor[k][c];
		
		travelerList.push_back(traveler);
	}
	
	//	free array of colors
	for (unsigned int k=0; k<numTravelers; k++)
		delete []travelerColor[k];
	delete []travelerColor;

	//Initialize the thread array and populate it
	thread_array = new ThreadInfo[numTravelers];
	for (int i = 0; i < numTravelers; i++)
	{
		//Initialize thread variables
		cout << "Added to traveler thread" << endl;
		thread_array[i].threadTraveler = travelerList[i];
		thread_array[i].doneGen = false;
		thread_array[i].complete = false;
		thread_array[i].threadTraveler.exitStatus = false;
	}
	for (int i = 0; i < numTravelers; i++)
	{
	//Create the mutex lock before starting threading	
	if (pthread_mutex_init(&onlyLock, NULL) != 0) 
	{ 
	    printf("\n mutex init has failed\n"); 
        break;
    }
	//Create the threads
	int error_code = pthread_create(&thread_array[i].threadID, NULL, threadFunc, &thread_array[i]);
			if (error_code != 0)
			{
				printf("ERROR: Failed to create thread %d with error_code=%d\n", i, error_code);
			}
			else
			{
				numLiveThreads++;
				finishedThreads = 0;
			}
	}
}


//------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Generation Helper Functions
#endif
//------------------------------------------------------

GridPosition getNewFreePosition(void)
{
	GridPosition pos;

	bool noGoodPos = true;
	while (noGoodPos)
	{
		unsigned int row = rowGenerator(engine);
		unsigned int col = colGenerator(engine);
		if (grid[row][col] == FREE_SQUARE)
		{
			pos.row = row;
			pos.col = col;
			noGoodPos = false;
		}
	}
	return pos;
}

Direction newDirection(Direction forbiddenDir)
{
	bool noDir = true;

	Direction dir = NUM_DIRECTIONS;
	while (noDir)
	{
		dir = static_cast<Direction>(segmentDirectionGenerator(engine));
		noDir = (dir==forbiddenDir);
	}
	return dir;
}


TravelerSegment newTravelerSegment(const TravelerSegment& currentSeg, bool& canAdd)
{
	TravelerSegment newSeg;
	switch (currentSeg.dir)
	{
		case NORTH:
			if (	currentSeg.row < numRows-1 &&
					grid[currentSeg.row+1][currentSeg.col] == FREE_SQUARE)
			{
				newSeg.row = currentSeg.row+1;
				newSeg.col = currentSeg.col;
				newSeg.dir = newDirection(SOUTH);
				grid[newSeg.row][newSeg.col] = TRAVELER;
				canAdd = true;
			}
			//	no more segment
			else
				canAdd = false;
			break;

		case SOUTH:
			if (	currentSeg.row > 0 &&
					grid[currentSeg.row-1][currentSeg.col] == FREE_SQUARE)
			{
				newSeg.row = currentSeg.row-1;
				newSeg.col = currentSeg.col;
				newSeg.dir = newDirection(NORTH);
				grid[newSeg.row][newSeg.col] = TRAVELER;
				canAdd = true;
			}
			//	no more segment
			else
				canAdd = false;
			break;

		case WEST:
			if (	currentSeg.col < numCols-1 &&
					grid[currentSeg.row][currentSeg.col+1] == FREE_SQUARE)
			{
				newSeg.row = currentSeg.row;
				newSeg.col = currentSeg.col+1;
				newSeg.dir = newDirection(EAST);
				grid[newSeg.row][newSeg.col] = TRAVELER;
				canAdd = true;
			}
			//	no more segment
			else
				canAdd = false;
			break;

		case EAST:
			if (	currentSeg.col > 0 &&
					grid[currentSeg.row][currentSeg.col-1] == FREE_SQUARE)
			{
				newSeg.row = currentSeg.row;
				newSeg.col = currentSeg.col-1;
				newSeg.dir = newDirection(WEST);
				grid[newSeg.row][newSeg.col] = TRAVELER;
				canAdd = true;
			}
			//	no more segment
			else
				canAdd = false;
			break;
		
		default:
			canAdd = false;
	}
	
	return newSeg;
}

void generateWalls(void)
{
	const unsigned int NUM_WALLS = (numCols+numRows)/4;

	//	I decide that a wall length  cannot be less than 3  and not more than
	//	1/4 the grid dimension in its Direction
	const unsigned int MIN_WALL_LENGTH = 3;
	const unsigned int MAX_HORIZ_WALL_LENGTH = numCols / 3;
	const unsigned int MAX_VERT_WALL_LENGTH = numRows / 3;
	const unsigned int MAX_NUM_TRIES = 20;

	bool goodWall = true;
	
	//	Generate the vertical walls
	for (unsigned int w=0; w< NUM_WALLS; w++)
	{
		goodWall = false;
		
		//	Case of a vertical wall
		if (headsOrTails(engine))
		{
			//	I try a few times before giving up
			for (unsigned int k=0; k<MAX_NUM_TRIES && !goodWall; k++)
			{
				//	let's be hopeful
				goodWall = true;
				
				//	select a column index
				unsigned int HSP = numCols/(NUM_WALLS/2+1);
				unsigned int col = (1+ unsignedNumberGenerator(engine)%(NUM_WALLS/2-1))*HSP;
				unsigned int length = MIN_WALL_LENGTH + unsignedNumberGenerator(engine)%(MAX_VERT_WALL_LENGTH-MIN_WALL_LENGTH+1);
				
				//	now a random start row
				unsigned int startRow = unsignedNumberGenerator(engine)%(numRows-length);
				for (unsigned int row=startRow, i=0; i<length && goodWall; i++, row++)
				{
					if (grid[row][col] != FREE_SQUARE)
						goodWall = false;
				}
				
				//	if the wall first, add it to the grid
				if (goodWall)
				{
					for (unsigned int row=startRow, i=0; i<length && goodWall; i++, row++)
					{
						grid[row][col] = WALL;
					}
				}
			}
		}
		// case of a horizontal wall
		else
		{
			goodWall = false;
			
			//	I try a few times before giving up
			for (unsigned int k=0; k<MAX_NUM_TRIES && !goodWall; k++)
			{
				//	let's be hopeful
				goodWall = true;
				
				//	select a column index
				unsigned int VSP = numRows/(NUM_WALLS/2+1);
				unsigned int row = (1+ unsignedNumberGenerator(engine)%(NUM_WALLS/2-1))*VSP;
				unsigned int length = MIN_WALL_LENGTH + unsignedNumberGenerator(engine)%(MAX_HORIZ_WALL_LENGTH-MIN_WALL_LENGTH+1);
				
				//	now a random start row
				unsigned int startCol = unsignedNumberGenerator(engine)%(numCols-length);
				for (unsigned int col=startCol, i=0; i<length && goodWall; i++, col++)
				{
					if (grid[row][col] != FREE_SQUARE)
						goodWall = false;
				}
				
				//	if the wall first, add it to the grid
				if (goodWall)
				{
					for (unsigned int col=startCol, i=0; i<length && goodWall; i++, col++)
					{
						grid[row][col] = WALL;
					}
				}
			}
		}
	}
}


void generatePartitions(void)
{
	const unsigned int NUM_PARTS = (numCols+numRows)/4;

	//	I decide that a partition length  cannot be less than 3  and not more than
	//	1/4 the grid dimension in its Direction
	const unsigned int MIN_PARTITION_LENGTH = 3;
	const unsigned int MAX_HORIZ_PART_LENGTH = numCols / 3;
	const unsigned int MAX_VERT_PART_LENGTH = numRows / 3;
	const unsigned int MAX_NUM_TRIES = 20;

	bool goodPart = true;

	for (unsigned int w=0; w< NUM_PARTS; w++)
	{
		goodPart = false;
		
		//	Case of a vertical partition
		if (headsOrTails(engine))
		{
			//	I try a few times before giving up
			for (unsigned int k=0; k<MAX_NUM_TRIES && !goodPart; k++)
			{
				//	let's be hopeful
				goodPart = true;
				
				//	select a column index
				unsigned int HSP = numCols/(NUM_PARTS/2+1);
				unsigned int col = (1+ unsignedNumberGenerator(engine)%(NUM_PARTS/2-2))*HSP + HSP/2;
				unsigned int length = MIN_PARTITION_LENGTH + unsignedNumberGenerator(engine)%(MAX_VERT_PART_LENGTH-MIN_PARTITION_LENGTH+1);
				
				//	now a random start row
				unsigned int startRow = unsignedNumberGenerator(engine)%(numRows-length);
				for (unsigned int row=startRow, i=0; i<length && goodPart; i++, row++)
				{
					if (grid[row][col] != FREE_SQUARE)
						goodPart = false;
				}
				
				//	if the partition is possible,
				if (goodPart)
				{
					//	add it to the grid and to the partition list
					SlidingPartition part;
					part.isVertical = true;
					for (unsigned int row=startRow, i=0; i<length && goodPart; i++, row++)
					{
						grid[row][col] = VERTICAL_PARTITION;
						GridPosition pos = {row, col};
						part.blockList.push_back(pos);
					}
				}
			}
		}
		// case of a horizontal partition
		else
		{
			goodPart = false;
			
			//	I try a few times before giving up
			for (unsigned int k=0; k<MAX_NUM_TRIES && !goodPart; k++)
			{
				//	let's be hopeful
				goodPart = true;
				
				//	select a column index
				unsigned int VSP = numRows/(NUM_PARTS/2+1);
				unsigned int row = (1+ unsignedNumberGenerator(engine)%(NUM_PARTS/2-2))*VSP + VSP/2;
				unsigned int length = MIN_PARTITION_LENGTH + unsignedNumberGenerator(engine)%(MAX_HORIZ_PART_LENGTH-MIN_PARTITION_LENGTH+1);
				
				//	now a random start row
				unsigned int startCol = unsignedNumberGenerator(engine)%(numCols-length);
				for (unsigned int col=startCol, i=0; i<length && goodPart; i++, col++)
				{
					if (grid[row][col] != FREE_SQUARE)
						goodPart = false;
				}
				
				//	if the wall first, add it to the grid and build SlidingPartition object
				if (goodPart)
				{
					SlidingPartition part;
					part.isVertical = false;
					for (unsigned int col=startCol, i=0; i<length && goodPart; i++, col++)
					{
						grid[row][col] = HORIZONTAL_PARTITION;
						GridPosition pos = {row, col};
						part.blockList.push_back(pos);
					}
				}
			}
		}
	}
}


void moveSegments(Traveler* traveler, int index, Direction dir){

	//Store the current location of the segment
	unsigned int row = traveler->segmentList[index].row;
	unsigned int col = traveler->segmentList[index].col;

	//Free is current location
	grid[row][col] = FREE_SQUARE;

	//Move the segment and mark the new location as a segment so that it cannot be passed through
	switch(dir){
	
		case NORTH:
			traveler->segmentList[index].row = traveler->segmentList[index].row - 1;
			grid[row - 1][col] = SEGMENT;
			break;
		case SOUTH:
			traveler->segmentList[index].row = traveler->segmentList[index].row + 1;
			grid[row + 1][col] = SEGMENT;
			break;
		case EAST:
			traveler->segmentList[index].col = traveler->segmentList[index].col + 1;
			grid[row][col+1] = SEGMENT;
			break;
		case WEST: 
			traveler->segmentList[index].col = traveler->segmentList[index].col - 1;
			grid[row][col-1] = SEGMENT;
			break;
		case NO_DIRECTION:
			grid[row][col] = SEGMENT;
			break;
	}
}

//Takes in the traveler's move as arugment and applies it
void applyMove(Traveler* traveler, Direction dir){

	//Apply move on the head
	switch(dir){

		case NORTH:
			traveler->segmentList[0].row = traveler->segmentList[0].row - 1;
			break;
		case SOUTH:
			traveler->segmentList[0].row = traveler->segmentList[0].row + 1;
			break;
		case EAST:
			traveler->segmentList[0].col = traveler->segmentList[0].col + 1;
			break;
		case WEST: 
			traveler->segmentList[0].col = traveler->segmentList[0].col - 1;
			break;
		case NO_DIRECTION:
			break;
	}
	//Apply moves to all segments based on the direction of whats ahead of them
	int len = traveler->segmentList.size();
	Direction moveDir;
	for (int i = 1; i < len; i++)
	{
		moveDir = traveler->segmentList[i-1].dir;
		moveSegments(traveler, i, moveDir);
	}
	//Apply proper direction changes based on what is directly in front of each segment, manually do the head
	//Do this in reverse order, so starting from the tail
	for (int i = len - 1; i > 0; i--)
	{
		traveler->segmentList[i].dir = traveler->segmentList[i-1].dir;
	}
	traveler->segmentList[0].dir = dir;
		
}

