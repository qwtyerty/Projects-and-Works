/**
 *	@brief		Implementation of the Cat class
 *	@details	Creates a Cat object that traverses the grid in search of the mouse,
 *				however the cat has 3 sets of possible behavior that it switches between
 *				the search for the mouse more fair. The first behavior the cat employs is
 *				searching optimally for the mouse using BFS, which will occur when the cat is
 *				sufficiently far away from the mouse and while the cat is not being lazy. The 
 *				next behavior is for the cat to act greedy, where once it is sufficiently close to
 *				the mouse, rather than search optimally the cat instead only moves to make its distance
 *				from the mouse as small as possible. The greed behavior cannot occur when the cat is 
 *				being lazy. The last behavior is for the cat to be lazy, where after taking a set number 
 *				of turns, the cat will take no actions for a set number of turns.
 */

#include "Cat.h"
#include <cmath>

const int NUM_LAZY_TURNS =		2;	// Number of turns where the cat stops moving
const int NUM_NORMAL_TURNS =	5;	// Number of turns where the cat is searching
const int GREEDY_RANGE_DENOM =	100;	// Denominator of the range (area/denom)

/**
 *	@brief		Constructor for a cat
 *	@details	Populates the class with member variables and finds the
 *				start location of the cat
 *	@param		pPlayGrid:	Pointer to the playgrid for movement
 *	@param		_pMouse:	Pointer to the mouse for tracking its location
 */
Cat::Cat(std::vector<std::vector<int>>* _pPlayGrid, Mouse* _pMouse)
{
	// Set a default orientation
	this->nextMove = UP;
	this->pPlayGrid = _pPlayGrid;
	gridUtilGetStartPosition(CAT, pPlayGrid, &(this->positionRow), &(this->positionCol));

	// Set Previous Position as the current position
	this->previousRow = this->positionRow;
	this->previousCol = this->positionCol;

	//Set the target psotions
	this->pMouse = _pMouse;

	// Set the number of Normal and lazy turns
	this->lazyTurnsLeft = 0;
	this->normalTurnsLeft = 0;
}

/**
 *	@brief		Selects a greedy move for the cat
 *	@details	Selects a greedy moved based on the distance of the 
 *				cat to the mouse
 *	@retval		None
 */
void Cat::selectGreedyMove()
{
	// Get a vector of possible moves
	std::vector<Direction> possibleMoves{ UP, DOWN, LEFT, RIGHT };

	// Set Default values
	int leastDistance = INT_MAX;
	Direction greedyMove = NUM_DIRECTIONS;
	int squareDistance;
	int dRow,dCol, targetRow, targetCol;
	std::pair<int, int> mousePos = this->pMouse->getCurrentPosition();

	for (auto move : possibleMoves) 
	{
		// Apply the move
		grdUtilTranslateDirections(move, &dRow, &dCol);
		targetRow = this->positionRow + dRow;
		targetCol = this->positionCol + dCol;

		// Calculate square distance to the mouse
		GridType tile = (GridType) (*(this->pPlayGrid))[targetRow][targetCol];


		// Check for valid square, then calculate square distance
		if (tile != EMPTY && tile != MOUSE && tile != CHEESE)
		{
			squareDistance = INT_MAX;
		}
		else 
		{
			squareDistance = (targetRow - mousePos.first) * (targetRow - mousePos.first) + (targetCol - mousePos.second) * (targetCol - mousePos.second);
		}

		if (squareDistance < leastDistance)
		{
			leastDistance = squareDistance;
			greedyMove = move;
		}
	}

	// Move has been selected, set it to the next
	this->nextMove = greedyMove;
}


/** 
 *	@brief		Checks all possible moves to be visited for BFS
 *	@details	Validates all possible moves and queues valid moves into the queue 
 *				for further searching. Returns early on finding the mouse.
 *	@param		grid:			Reference to the grid of visited nodes, and their location
 *	@param		row:			Row to check possible moves from
 *	@param		col:			Column to check the possible moves from
 *	@param		searchQueue:	Reference to queue of coordinates to visit
 *	@retval		Endpoint: Pair storing the coordinates of the mouse location
 */
std::pair<int, int> Cat::checkPossibleMoves(std::vector<std::vector<std::pair<int, int>>> &grid, int row, int col, std::queue<std::pair<int,int>> &searchQueue)
{
	// Get a vector of possible moves
	std::vector<Direction> possibleMoves{ UP, DOWN, LEFT, RIGHT };

	int dRow = 0;
	int dCol = 0;

	for (auto move : possibleMoves) 
	{
		// Select the next move
		grdUtilTranslateDirections(move, &dRow, &dCol);
		int targetRow = row + dRow;
		int targetCol = col + dCol;

		if (validateMove(grid, targetRow, targetCol) == true)
		{
			// Check if the mouse has been found
			if ((*(this->pPlayGrid))[targetRow][targetCol] == MOUSE)
			{
				grid[targetRow][targetCol] = { row, col };
				return { targetRow, targetCol};
			}
			else 
			{
				// Push a new potential path and mark the target as visited 
				searchQueue.push({ targetRow, targetCol });
				grid[targetRow][targetCol] = {row, col};
			}
		}
	}

	return {-1, -1};
}

/**
 *	@brief		Validate a given coordinate is valid
 *	@details	Checks if the selected row and column is an available possition and has not
 *				visited previously
 *	@param		grid:	Reference to the grid of visited nodes, and their location
 *	@param		row:	Row of the coordinate to validate
 *	@param		col:	Column of the coordinate to validate
 *	@retval		True if the coordinate is valid, false otherwise
 */
bool Cat::validateMove(std::vector<std::vector<std::pair<int, int>>> &grid, int row, int col)
{
	// Get the tile type and previous coordinate
	GridType tile = (GridType) (*(this->pPlayGrid))[row][col];
	std::pair<int, int> coordinate = grid[row][col];

	// If grid is occupied, the move is invalid
	if (tile != EMPTY && tile != MOUSE && tile != CHEESE)
	{
		return false;
	}

	// Other wise if has been visited the move is invalid
	if (coordinate.first != -1 && coordinate.second != -1) 
	{
		return false;
	}

	return true;
}

/**
 *	@brief		Find the direction to move based on the end point
 *	@details	traverses the grid in reverse to find the cat's position then translates it
 *				to a direction to move
 *	@param		grid:	Reference to the grid of visited nodes, and their location
 *	@param		row:	Row of the coordinate to validate
 *	@param		col:	Column of the coordinate to validate
 *	@retval		None
 */
void Cat::reconstructPath(std::vector<std::vector<std::pair<int, int>>>& grid, int row, int col)
{

	// Check for an invalid endpoint (mouse and cat overlap)
	if (row == -1 || col == -1)
	{
		this->nextMove = NUM_DIRECTIONS;
		return;
	}

	std::pair<int, int> previousTile = grid[row][col];
	std::pair<int, int> nextTile;
	bool sourceFound = false;

	// Check for distance of 1
	if ((GridType)(*(this->pPlayGrid))[previousTile.first][previousTile.second] == CAT)
	{
		sourceFound = true;
	}

	while (sourceFound == false)
	{
		nextTile = grid[previousTile.first][previousTile.second];
		// Move to the previous tile in the path

		if ((GridType)(*(this->pPlayGrid))[nextTile.first][nextTile.second] == CAT)
		{
			sourceFound = true;
		}
		else 
		{
			previousTile = nextTile;
		}
	}

	// We have located the cat find the difference in the dx dy
	int dRow = previousTile.first - this->getCurrentPosition().first;
	int dCol = previousTile.second - this->getCurrentPosition().second;

	if (dRow != 0 && dCol == 0) 
	{
		if (dRow < 0)
		{
			this->nextMove = UP;
		}
		else 
		{
			this->nextMove = DOWN;
		}
	}

	else if (dRow == 0 && dCol != 0)
	{
		if (dCol < 0) 
		{
			this->nextMove = LEFT;
		}
		else 
		{
			this->nextMove = RIGHT;
		}
	}

	else 
	{
		// Error occured, do not move
		this->nextMove = NUM_DIRECTIONS;
	}
}

/**
 *	@brief		Selects an optimal move using BFS
 *	@details	Searches for the mouse using BFS and select the next move
 *				based on the generated path
 *	@retval		None
 */
void Cat::selectOptimalMove() 
{
	// Get the width and height
	int height = (int) this->pPlayGrid->size();
	int width = (int) this->pPlayGrid->at(0).size();
	std::pair<int, int> endPoint= {-1, 1};
	
	// Create a 2d vector of invalid coordinate pairs 
	std::vector<std::vector<std::pair<int, int>>> searchGrid
		(
		height,
		std::vector<std::pair<int, int>>(width, {-1, -1})
	);

	// Create a queue for processing possible paths
	std::queue<std::pair<int, int>> searchQueue;

	std::pair<int, int> currentPosition = this->getCurrentPosition();
	// Push the starting cell as visited
	searchQueue.push(currentPosition);
	searchGrid[currentPosition.first][currentPosition.second] = { currentPosition.first,  currentPosition.second };


	// Process the queue until it is empty
	while (!searchQueue.empty() && (endPoint.first == -1 || endPoint.second == -1))
	{
		// Get the next path
		std::pair<int, int> cell = searchQueue.front();
		searchQueue.pop();

		// Check possible moves, and if there is a path to the mouse
		endPoint = checkPossibleMoves(searchGrid, cell.first, cell.second, searchQueue);
	}

	// Get the direction of the next move
	reconstructPath(searchGrid, endPoint.first, endPoint.second);
}


/**
 *	@brief		Select greedy or optimal behavior
 *	@details	Select behavior based on the distance of the cat from the mouse
 *	@retval		None
 */
void Cat::selectBehavior() 
{
	std::pair<int, int> mousePos = this->pMouse->getCurrentPosition();
	

	int squareDist = (this->positionRow - mousePos.first) * (this->positionRow - mousePos.first) + (this->positionCol - mousePos.second) * (this->positionCol - mousePos.second);
	int greedyRange = (int) (this->pPlayGrid->size() * this->pPlayGrid->at(0).size()) / GREEDY_RANGE_DENOM;

	// Check if we are within greed range
	if (greedyRange >= squareDist) 
	{
		this->selectGreedyMove();
	}
	else 
	{
		this->selectOptimalMove();
	}
}

/**
 *	@brief		Select whether the cat will move or not
 *	@details	Select the whether the cat will move based on the number
 *				of @ref normalTurnsLeft and @ref lazyTurnsLeft
	@retval		None
 */
void Cat::selectNextMove()
{
	if (this->normalTurnsLeft > 0) 
	{
		this->normalTurnsLeft--;
		this->selectBehavior();
	}

	else if (this->lazyTurnsLeft > 0)
	{
		
		// Select an invalid move and don't move
		this->nextMove = NUM_DIRECTIONS;
		this->lazyTurnsLeft--;
	}
	else 
	{
		this->normalTurnsLeft = NUM_NORMAL_TURNS;
		this->lazyTurnsLeft = NUM_LAZY_TURNS;
		this->selectBehavior();
		
	}
}

/**
 *	@brief		Move the position of the cat
 *	@details	Moves the position of the cat and updates the previous position
 *				baed on the move made.
 *	@retval		None
 *
 */
void Cat::movePosition() 
{
	// Get the change in row and column position
	int dRow, dCol;
	grdUtilTranslateDirections(this->nextMove, &dRow, &dCol);

	// Update the previous positions
	this->previousRow = this->positionRow;
	this->previousCol = this->positionCol;

	// Update current position
	this->positionRow += dRow;
	this->positionCol += dCol;
}

/**
 *	@brief		Returns the position of the cat
 *	@details	Returns the position of the cat as a pair of ints
 *	@retval		Current position of the cat
 */
std::pair<int, int> Cat::getCurrentPosition()
{
	std::pair<int, int> catPos(this->positionRow, this->positionCol);
	return catPos;
}

/**
 *	@brief		Returns the previous position of the cat
 *	@details	Returns the previous position of the cat as a pair of ints
 *	@retval		Previous position of the cat
 */
std::pair<int, int> Cat::getPreviousPosition()
{
	std::pair<int, int> catPos(this->previousRow, this->previousCol);
	return catPos;
}