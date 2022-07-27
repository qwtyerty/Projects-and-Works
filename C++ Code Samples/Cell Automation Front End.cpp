 /*-------------------------------------------------------------------------+
 |	A graphic front end for a grid+state simulation.						|
 |																			|
 |	This application simply creates a glut window with a pane to display	|
 |	a colored grid and the other to display some state information.			|
 |	Sets up callback functions to handle menu, mouse and keyboard events.	|
 |	Normally, you shouldn't have to touch anything in this code, unless		|
 |	you want to change some of the things displayed, add menus, etc.		|
 |	Only mess with this after everything else works and making a backup		|
 |	copy of your project.  OpenGL & glut are tricky and it's really easy	|
 |	to break everything with a single line of code.							|
 |																			|
 |	Current keyboard controls:												|
 |																			|
 |		- 'ESC' --> exit the application									|
 |		- space bar --> resets the grid										|
 |																			|
 |		- 'c' --> toggle color mode on/off									|
 |		- 'b' --> toggles color mode off/on									|
 |		- 'l' --> toggles on/off grid line rendering						|
 |																			|
 |		- '+' --> increase simulation speed									|
 |		- '-' --> reduce simulation speed									|
 |																			|
 |		- '1' --> apply Rule 1 (Conway's classical Game of Life: B3/S23)	|
 |		- '2' --> apply Rule 2 (Coral: B3/S45678)							|
 |		- '3' --> apply Rule 3 (Amoeba: B357/S1358)							|
 |		- '4' --> apply Rule 4 (Maze: B3/S12345)							|
 |																			|
 +-------------------------------------------------------------------------*/

#include <iostream>
#include <sstream>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <pthread.h>
#include "gl_frontEnd.h"
#include <random>

using namespace std;

#if 0
//==================================================================================
#pragma mark -
#pragma mark Custom data types
//==================================================================================
#endif

typedef struct ThreadInfo
{
	//	you probably want these
	pthread_t threadID;
	int threadIndex;
	unsigned int startRow;
	unsigned int endRow;
	bool complete;
} ThreadInfo;


#if 0
//==================================================================================
#pragma mark -
#pragma mark Function prototypes
//==================================================================================
#endif

void displayGridPane(void);
void displayStatePane(void);
void initializeApplication(unsigned int numRows, unsigned int numCols);
void cleanupAndquit(void);
void* threadFunc(void*);
void swapGrids(void);
unsigned int cellNewState(unsigned int i, unsigned int j, unsigned int numRows, unsigned int numCols);


//==================================================================================
//	Precompiler #define to let us specify how things should be handled at the
//	border of the frame
//==================================================================================

#if 0
//==================================================================================
#pragma mark -
#pragma mark Simulation mode:  behavior at edges of frame
//==================================================================================
#endif

#define FRAME_DEAD		0	//	cell borders are kept dead
#define FRAME_RANDOM	1	//	new random values are generated at each generation
#define FRAME_CLIPPED	2	//	same rule as elsewhere, with clipping to stay within bounds
#define FRAME_WRAP		3	//	same rule as elsewhere, with wrapping around at edges

//	Pick one value for FRAME_BEHAVIOR
#define FRAME_BEHAVIOR	FRAME_DEAD

#if 0
//==================================================================================
#pragma mark -
#pragma mark Application-level global variables
//==================================================================================
#endif

//	Don't touch
extern int gMainWindow, gSubwindow[2];

//	The state grid and its dimensions.  We use two copies of the grid:
//		- currentGrid is the one displayed in the graphic front end
//		- nextGrid is the grid that stores the next generation of cell
//			states, as computed by our threads.
unsigned int* currentGrid;
unsigned int* nextGrid;
unsigned int** currentGrid2D;
unsigned int** nextGrid2D;

//	Piece of advice, whenever you do a grid-based (e.g. image processing),
//	implementastion, you should always try to run your code with a
//	non-square grid to spot accidental row-col inversion bugs.
//	When this is possible, of course (e.g. makes no sense for a chess program).
unsigned int NUM_ROWS, NUM_COLS, NUM_THREADS;

//	the number of live computation threads (that haven't terminated yet)
unsigned short numLiveThreads = 0;

unsigned int rule = GAME_OF_LIFE_RULE;

unsigned int colorMode = 0;

unsigned int generation = 0;



// Global var for the time its between threads, its increments and a max and min
unsigned int threadSleepTime = 50000;
const unsigned int threadSleepIncrement = 10000;
const unsigned int maxThreadSleepTime = 100000;
const unsigned int minThreadSleepTime = 10000;

//Set up the thread and lock as global vars
ThreadInfo* threadInfo;
pthread_mutex_t** lockArr;

//Set up a rendom engine
random_device randDev;
default_random_engine engine(randDev());
uniform_int_distribution<unsigned int> uniformDist(0, 1);
uniform_int_distribution<unsigned int> uniformDistLong(0,8);

//------------------------------
//	Threads and synchronization
//	Reminder of all declarations and function calls
//------------------------------
//pthread_mutex_t myLock;
//pthread_mutex_init(&myLock, NULL);
//int err = pthread_create(pthread_t*, NULL, threadFunc, ThreadInfo*);
//int pthread_join(pthread_t , void**);
//pthread_mutex_lock(&myLock);
//pthread_mutex_unlock(&myLock);

#if 0
//==================================================================================
#pragma mark -
#pragma mark Computation functions
//==================================================================================
#endif


int main(int argc, const char* argv[])
{
	unsigned int numRow, numCol, numThread;
	if(argc == 4)
	{
		stringstream ss;
		ss << argv[1] << ' ' << argv[2] << ' ' << argv[3];
		ss >> numRow >> numCol >> numThread;
		if(numThread > numRow){
			cout << "Number of threads must be less than number of rows" << endl;
			exit(2);
		}
		else if(numRow <= 5 || numCol <= 5 || numThread < 1)
		{
			cout << "Width and Height must be greater than 5, and number of threads must be greater than 0" << endl;
		}
	}
	else
	{
		cout << "Usage:" << argv[0] << " <Width> <Height> <Number of threads, must be less than height>" <<  endl;
		exit(1);
	}
	NUM_ROWS = numRow;
	NUM_COLS = numCol;
	NUM_THREADS = numThread;
	//	This takes care of initializing glut and the GUI.
	//	You shouldn’t have to touch this
	initializeFrontEnd(argc, argv, displayGridPane, displayStatePane);
	
	//	Now we can do application-level initialization
	initializeApplication(numRow, numCol);
	lockArr = new pthread_mutex_t* [numRow];
	//Create a 2D array of locks
	for(unsigned int i = 0; i < numRow; ++i)
	{
		lockArr[i] = new pthread_mutex_t[numCol];
		for(unsigned int j = 0; j < numRow; ++j)
			pthread_mutex_init(&lockArr[i][j], NULL);
	}


	threadInfo = new ThreadInfo[numThread];
	//Define a chunk size for a thread to work on
	int chunkSize;
	//Chunck divdes evenly
	if(NUM_ROWS % NUM_THREADS  == 0)
		chunkSize = NUM_ROWS/NUM_THREADS;
	else 
		chunkSize = 1 + NUM_ROWS/NUM_THREADS;
		
	//Create all the threads
	for(unsigned int i = 0; i < NUM_THREADS; ++i)
	{
		threadInfo[i].startRow = i * chunkSize;
		threadInfo[i].endRow = (i + 1) * chunkSize;
		//If the thread is too large under cut thread
		if(threadInfo[i].endRow > NUM_ROWS)
			threadInfo[i].endRow = NUM_ROWS;
		threadInfo[i].threadIndex = i;
		threadInfo[i].complete = false;
		++numLiveThreads;
		
		int err = pthread_create(&threadInfo[i].threadID, NULL, threadFunc, threadInfo + i);
		if(err != 0)
		{
			cout << "Unable to create thread " << i << ". [" << err << "]: " << 
				strerror(err) << endl << flush;
			exit(1);
		}
	}

	//	Now we enter the main loop of the program and to a large extent
	//	"lose control" over its execution.
	glutMainLoop();
	
	//	In fact this code is never reached because we only leave the glut main
	//	loop through an exit call.
	//	Free allocated resource before leaving (not absolutely needed, but
	//	just nicer.  Also, if you crash there, you know something is wrong
	//	in your code.
	free(currentGrid2D);
	free(currentGrid);
		
	//	This will never be executed (the exit point will be in one of the
	//	call back functions).
	return 0;
}


void initializeApplication(unsigned int numRows, unsigned int numCols)
{
    //--------------------
    //  Allocate 1D grids
    //--------------------
    currentGrid = new unsigned int[numRows * numCols];
    nextGrid = new unsigned int[numRows * numCols];

    //---------------------------------------------
    //  Scaffold 2D arrays on top of the 1D arrays
    //---------------------------------------------
    currentGrid2D = new unsigned int*[numRows];
    nextGrid2D = new unsigned int*[numRows];
    currentGrid2D[0] = currentGrid;
    nextGrid2D[0] = nextGrid;
    for (unsigned int i=1; i<numRows; i++)
    {
        currentGrid2D[i] = currentGrid2D[i-1] + numCols;
        nextGrid2D[i] = nextGrid2D[i-1] + numCols;
    }
	
	resetGrid();
}


void* threadFunc(void* arg)
{
	ThreadInfo* info = (ThreadInfo*) arg;
	uniform_int_distribution<int> uniformDistI(0, NUM_ROWS - 1);
	uniform_int_distribution<int> uniformDistJ(0, NUM_COLS - 1);
	int i;
	int j;
	//Have threads loop infinutely to be eventually terminated when the user hits esc
	while(true)
	{
		i = uniformDistI(engine);
		j = uniformDistJ(engine);
		//lock access
		for(int k = i - 1; k < i + 2; ++k)
		{
			if(k < 0 || k > (int) (NUM_ROWS - 1))
				continue;
			for(int l = j - 1; l < j +2; ++l)
			{
				if(l < 0 || l > (int) (NUM_COLS - 1))
					continue;
				cout << "about to lock (" << k << ", " << l <<")" << endl;
				pthread_mutex_lock(&lockArr[k][l]);
				cout << "Locked." << endl;
			}
		}
		cout << "Access locked" << endl;
		//Get the new state of each cell
		unsigned int newState = cellNewState(i, j, NUM_ROWS, NUM_COLS);
		
		if(colorMode == 0 || newState == 0)
		{
			nextGrid2D[i][j] = newState;
		}
		else
		{
			if(currentGrid2D[i][j] < NB_COLORS-1)
				nextGrid2D[i][j] = currentGrid2D[i][j] + 1;
			else
				nextGrid2D[i][j] = currentGrid2D[i][j];
		}
		
		cout << "Unlocking" << endl;
		//Reopen for access
		for(int k = i - 1; k < i + 2; ++k)
		{
			if(k < 0 || k > (int) (NUM_ROWS - 1))
				continue;
			for (int l = j - 1; l < j +2; ++l)
			{
				if(l < 0 || l > (int) (NUM_COLS - 1))
					continue;
				cout << "About to unlock (" << k << ", " << l <<")" << endl;
				pthread_mutex_unlock(&lockArr[k][l]);
				cout << "Unlocked." << endl;
			}
		}
		cout << "Work Complete swapping grids ..." << endl;
		
		usleep(threadSleepTime);
		cout << "Done waiting returning to top" << endl;
	}
	return NULL;
}


//	This is the function that determines how a cell update its state
unsigned int cellNewState(unsigned int i, unsigned int j, unsigned int numRows, unsigned int numCols)
{
	//	First count the number of neighbors that are alive
	int count = 0;

	//	Away from the border, we simply count how many among the cell's
	//	eight neighbors are alive (cell state > 0)
	if (i>0 && i<numRows-1 && j>0 && j<numCols-1)
	{
		//	remember that in C, (x == val) is either 1 or 0
		count = (currentGrid2D[i-1][j-1] != 0) +
				(currentGrid2D[i-1][j] != 0) +
				(currentGrid2D[i-1][j+1] != 0)  +
				(currentGrid2D[i][j-1] != 0)  +
				(currentGrid2D[i][j+1] != 0)  +
				(currentGrid2D[i+1][j-1] != 0)  +
				(currentGrid2D[i+1][j] != 0)  +
				(currentGrid2D[i+1][j+1] != 0);
	}
	//	on the border of the frame...
	else
	{
		#if FRAME_BEHAVIOR == FRAME_DEAD
		
			//	Hack to force death of a cell
			count = -1;
		
		#elif FRAME_BEHAVIOR == FRAME_RANDOM
		
			count = uniformDistLong(engine);
		#elif FRAME_BEHAVIOR == FRAME_CLIPPED
	
			if (i>0)
			{
				if (j>0 && currentGrid2D[i-1][j-1] != 0)
					count++;
				if (currentGrid2D[i-1][j] != 0)
					count++;
				if (j<numCols-1 && currentGrid2D[i-1][j+1] != 0)
					count++;
			}

			if (j>0 && currentGrid2D[i][j-1] != 0)
				count++;
			if (j<numCols-1 && currentGrid2D[i][j+1] != 0)
				count++;

			if (i<numRows-1)
			{
				if (j>0 && currentGrid2D[i+1][j-1] != 0)
					count++;
				if (currentGrid2D[i+1][j] != 0)
					count++;
				if (j<numCols-1 && currentGrid2D[i+1][j+1] != 0)
					count++;
			}
			
	
		#elif FRAME_BEHAVIOR == FRAME_WRAPPED
	
			unsigned int 	iM1 = (i+numRows-1)%numRows,
							iP1 = (i+1)%numRows,
							jM1 = (j+numCols-1)%numCols,
							jP1 = (j+1)%numCols;
			count = currentGrid2D[iM1][jM1] != 0 +
					currentGrid2D[iM1][j] != 0 +
					currentGrid2D[iM1][jP1] != 0  +
					currentGrid2D[i][jM1] != 0  +
					currentGrid2D[i][jP1] != 0  +
					currentGrid2D[iP1][jM1] != 0  +
					currentGrid2D[iP1][j] != 0  +
					currentGrid2D[iP1][jP1] != 0 ;

		#else
			#error undefined frame behavior
		#endif
		
	}	//	end of else case (on border)
	
	//	Next apply the cellular automaton rule
	//----------------------------------------------------
	//	by default, the grid square is going to be empty/dead
	unsigned int newState = 0;
	
	//	unless....
	
	switch (rule)
	{
		//	Rule 1 (Conway's classical Game of Life: B3/S23)
		case GAME_OF_LIFE_RULE:

			//	if the cell is currently occupied by a live cell, look at "Stay alive rule"
			if (currentGrid2D[i][j] != 0)
			{
				if (count == 3 || count == 2)
					newState = 1;
			}
			//	if the grid square is currently empty, look at the "Birth of a new cell" rule
			else
			{
				if (count == 3)
					newState = 1;
			}
			break;
	
		//	Rule 2 (Coral Growth: B3/S45678)
		case CORAL_GROWTH_RULE:

			//	if the cell is currently occupied by a live cell, look at "Stay alive rule"
			if (currentGrid2D[i][j] != 0)
			{
				if (count > 3)
					newState = 1;
			}
			//	if the grid square is currently empty, look at the "Birth of a new cell" rule
			else
			{
				if (count == 3)
					newState = 1;
			}
			break;
			
		//	Rule 3 (Amoeba: B357/S1358)
		case AMOEBA_RULE:

			//	if the cell is currently occupied by a live cell, look at "Stay alive rule"
			if (currentGrid2D[i][j] != 0)
			{
				if (count == 1 || count == 3 || count == 5 || count == 8)
					newState = 1;
			}
			//	if the grid square is currently empty, look at the "Birth of a new cell" rule
			else
			{
				if (count == 3 || count == 5 || count == 7)
					newState = 1;
			}
			break;
		
		//	Rule 4 (Maze: B3/S12345)							|
		case MAZE_RULE:

			//	if the cell is currently occupied by a live cell, look at "Stay alive rule"
			if (currentGrid2D[i][j] != 0)
			{
				if (count >= 1 && count <= 5)
					newState = 1;
			}
			//	if the grid square is currently empty, look at the "Birth of a new cell" rule
			else
			{
				if (count == 3)
					newState = 1;
			}
			break;

			break;
		
		default:
			cout << "Invalid rule number" << endl;
			exit(5);
	}

	return newState;
}

void cleanupAndquit(void)
{
	//	join the threads
	for(unsigned int i = 0; i < NUM_THREADS; i++)
	{
		if(!threadInfo[i].complete)
			pthread_join(threadInfo[i].threadID, NULL);
		
	}
	//	free the grids
	free(currentGrid2D);
	free(currentGrid);
	exit(0);
}



#if 0
#pragma mark -
#pragma mark GUI functions
#endif

//==================================================================================
//	GUI functions
//	These are the functions that tie the simulation with the rendering.
//==================================================================================


void displayGridPane(void)
{
	//	This is OpenGL/glut magic.  Don't touch
	glutSetWindow(gSubwindow[GRID_PANE]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//---------------------------------------------------------
	//	This is the call that makes OpenGL render the grid.
	//
	//---------------------------------------------------------
	drawGrid(currentGrid2D, NUM_ROWS, NUM_COLS);
	
	//	This is OpenGL/glut magic.  Don't touch
	glutSwapBuffers();
	
	glutSetWindow(gMainWindow);
}

void displayStatePane(void)
{
	//	This is OpenGL/glut magic.  Don't touch
	glutSetWindow(gSubwindow[STATE_PANE]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//---------------------------------------------------------
	//	This is the call that makes OpenGL render information
	//	about the state of the simulation.
	//---------------------------------------------------------
	drawState(numLiveThreads, threadSleepTime);
	
	//	This is OpenGL/glut magic.  Don't touch
	glutSwapBuffers();
	glutSetWindow(gMainWindow);
}


//	This callback function is called when a keyboard event occurs
//
void myKeyboardFunc(unsigned char c, int x, int y)
{
	(void) x; (void) y;
	
	switch (c)
	{
		//	'ESC' --> exit the application
		case 27:
			cleanupAndquit();
			break;

		//	spacebar --> resets the grid
		case ' ':
			resetGrid();
			break;

		//	'+' --> increase simulation speed
		case '+':
			threadSleepTime -= threadSleepIncrement;
			if(threadSleepTime < minThreadSleepTime)
				threadSleepTime = minThreadSleepTime;
			break;

		//	'-' --> reduce simulation speed
		case '-':
			threadSleepTime += threadSleepIncrement;
			if(threadSleepTime >  maxThreadSleepTime)
				threadSleepTime = maxThreadSleepTime;
			break;

		//	'1' --> apply Rule 1 (Game of Life: B23/S3)
		case '1':
			rule = GAME_OF_LIFE_RULE;
			break;

		//	'2' --> apply Rule 2 (Coral: B3_S45678)
		case '2':
			rule = CORAL_GROWTH_RULE;
			break;

		//	'3' --> apply Rule 3 (Amoeba: B357/S1358)
		case '3':
			rule = AMOEBA_RULE;
			break;

		//	'4' --> apply Rule 4 (Maze: B3/S12345)
		case '4':
			rule = MAZE_RULE;
			break;

		//	'c' --> toggles on/off color mode
		//	'b' --> toggles off/on color mode
		case 'c':
		case 'b':
			colorMode = !colorMode;
			break;

		//	'l' --> toggles on/off grid line rendering
		case 'l':
			toggleDrawGridLines();
			break;

		default:
			break;
	}
	
	glutSetWindow(gMainWindow);
	glutPostRedisplay();
}

void myTimerFunc(int value)
{
	//	value not used.  Warning suppression
	(void) value;
	
	//myDisplayFunc();
    	

	//Lock the board then swap and display then unlock it
	for(unsigned int i = 0; i < NUM_ROWS; ++i)
	{
		for(unsigned int j = 0; j < NUM_COLS; ++j)
		{
			pthread_mutex_lock(&lockArr[i][j]);
		}
	}
	swapGrids();
	//glutPostRedisplay();
	for(unsigned int i = 0; i < NUM_ROWS; ++i)
	{
		for(unsigned int j = 0; j < NUM_COLS; ++j)
		{
			pthread_mutex_unlock(&lockArr[i][j]);
		}
	}
//	And finally I perform the rendering
	glutTimerFunc(100, myTimerFunc, 0);
}


void resetGrid()
{
	for (unsigned int i=0; i<NUM_ROWS; i++)
	{
		for (unsigned int j=0; j<NUM_ROWS; j++)
		{
			nextGrid2D[i][j] = uniformDist(engine);
		}
	}
	swapGrids();
}

//	This function swaps the current and next grids, as well as their
//	companion 2D grid.  Note that we only swap the "top" layer of
//	the 2D grids.
void swapGrids(void)
{
	//	swap grids
	unsigned int* tempGrid;
	unsigned int** tempGrid2D;
	
	tempGrid = currentGrid;
	currentGrid = nextGrid;
	nextGrid = tempGrid;
	//
	tempGrid2D = currentGrid2D;
	currentGrid2D = nextGrid2D;
	nextGrid2D = tempGrid2D;
}

