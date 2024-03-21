/**
 *	@brief		Implementation of the Mouse class
 *	@details	Creates a mouse object that randomly traverses the grid until it can see 
 *				a cheese tile along a possible path the mouse can take (e.g. straight left, right, up, down)
 *				in which case the mouse will follow that path until it eats the cheese. In the even that the 
 *				cat reaches and eats the mouse, a random new location will be selected to place the mouse, which
 *				will then continue searching for cheese.
 */

#include "Mouse.h"
#include <algorithm>
#include "RNG.h"

 /**
  *	@brief		Constructor for a mouse
  *	@details	Populates the class with member variables and finds the tart location of the mouse
  *	@param		pPlayGrid:	Pointer to the playgrid for movement
  */
Mouse::Mouse(std::vector<std::vector<int>>* playGrid)
{
	// Set a default orientation
	this->nextMove = UP;
	this->pPlayGrid = playGrid;
	gridUtilGetStartPosition(MOUSE, playGrid, &(this->positionRow), &(this->positionCol));

	// Set Previous Position as the current position
	this->previousRow = this->positionRow;
	this->previousCol = this->positionCol;
}

/**
 *	@brief		Check if any move in a direction leads to cheese
 *	@details	Checks in cheese is "visible" in any direction from 
 *				the mouse
 *	@retval		True if the cheese is spotted, false otherwise
 */
bool Mouse::searchForCheese() 
{
	std::vector<Direction> possibleMoves{ UP, DOWN, LEFT, RIGHT };

	//Check if cheese is visable in a direction
	for (auto move : possibleMoves)
	{
		if (this->cheeseSpotted(move))
		{
			this->nextMove = move;
			return true;
		}
	}

	return false;
}

/**
 *	@brief		Follow a direction and check if cheese is along a valid direction
 *	@details	Follow a direction until an invalid move is made or a move leads
 *				into cheese
 *	@retval		True if direction leads to cheese, false otherwise
 */
bool Mouse::cheeseSpotted(Direction sightLine)
{
	int distance = 1;
	int dRow = 0;
	int dCol = 0;

	grdUtilTranslateDirections(sightLine, &dRow, &dCol);

	int targetRow = this->positionRow + dRow * distance;
	int targetCol = this->positionCol + dCol * distance;
	
	// Iterate until we hit anything
	while ((*(this->pPlayGrid))[targetRow][targetCol] == EMPTY)
	{
		distance++;
		targetRow = positionRow + dRow * distance;
		targetCol = positionCol + dCol * distance;
	}

	// Check if cheese was hit and return the result
	return (*(this->pPlayGrid))[targetRow][targetCol] == CHEESE;
}


/**
 *	@brief		Select a random move out of all possible moves
 *	@details	Select a random move from all possible directions and remove
 *				any invalid directions until a valid direction is selected or 
 *				all directions are exhausted.
 *	@retval		None
 */
void Mouse::selectRandomMove() 
{
	std::vector<Direction> possibleMoves{ UP, DOWN, LEFT, RIGHT };
	std::vector<Direction> selectableMoves{ UP, DOWN, LEFT, RIGHT };

	Direction move;

	while (selectableMoves.size() > 0) 
	{
		move = (Direction)RNGgenRandomInt(0,4);

		// Check if move has been selected before
		std::vector<Direction>::iterator location = std::find(selectableMoves.begin(), selectableMoves.end(), move);
		if (location != selectableMoves.end())
		{
			// Translate the movement into indices
			int dRow, dCol, targetRow, targetCol;
			grdUtilTranslateDirections(move, &dRow, &dCol);

			targetRow = this->positionRow + dRow;
			targetCol = this->positionCol + dCol;

			//Check if the position is valid
			GridType targetType = (GridType) (*(this->pPlayGrid))[targetRow][targetCol];
			if (targetType == EMPTY || targetType == CHEESE) 
			{
				// Select the move
				this->nextMove = move;
				return;
			}
			else 
			{
				// Remove from future searches
				selectableMoves.erase(location);
			}
		}
	} 

	// Set a default move, where the mouse doesn't move
	this->nextMove = NUM_DIRECTIONS;
}

/**
 *	@brief		Select a move for the mouse
 *	@details	Attempt to see if the cheese can be seen, and select a random move otherwise
 *  @retval		None
 */
void Mouse::selectNextMove()
{
	// Attempt to search for cheese
	if (!this->searchForCheese())
	{
		// Cheese is not visible, select random valid move
		this->selectRandomMove();
	}
}

/**
 *	@brief		Move the position of the mouse
 *	@details	Moves the position of the mouse and updates the previous position
 *				baed on the move made.
 *	@retval		None
 */
void Mouse::movePosition() 
{
	// Get the change in row and column position
	int dRow, dCol;
	grdUtilTranslateDirections(nextMove, &dRow, &dCol);

	// Update the previous positions
	this->previousRow = this->positionRow;
	this->previousCol = this->positionCol;

	// Update current position
	this->positionRow += dRow;
	this->positionCol += dCol;
}

/**
 *	@brief		Returns the position of the mouse
 *	@details	Returns the position of the mouse as a pair of ints
 *	@retval		Current position of the mouse
 */
std::pair<int, int> Mouse::getCurrentPosition()
{
	std::pair<int, int> mousePos(this->positionRow, this->positionCol);
	return mousePos;
}

/**
 *	@brief		Returns the previous position of the mouse
 *	@details	Returns the previous position of the mouse as a pair of ints
 *	@retval		Previous position of the mouse
 */
std::pair<int, int> Mouse::getPreviousPosition() 
{
	std::pair<int, int> mousePos(this->previousRow, this->previousCol);
	return mousePos;
}

/**
 *	@brief		Spawns the mouse in a new position
 *	@details	Spawns the mouse in a free position on the grid when the mouse has been eaten
 *	@retval		None
 */
void Mouse::spawnNewPosition() 
{
	// Casts are used as width and height are expected to be less than
	// INT_MAX

	// Select a random coordinate
	int targetRow = RNGgenRandomInt(0, (int)this->pPlayGrid->size());
	int targetCol = RNGgenRandomInt(0, (int)this->pPlayGrid->at(0).size());

	// Check if a location is valid, select new coordinates otherwise
	while ((*pPlayGrid)[targetRow][targetCol] != EMPTY)
	{
		targetRow = RNGgenRandomInt(0, (int)this->pPlayGrid->size());
		targetCol = RNGgenRandomInt(0, (int)this->pPlayGrid->at(0).size());
	}

	// Apply the translation
	this->positionRow = targetRow;
	this->positionCol = targetCol;
}