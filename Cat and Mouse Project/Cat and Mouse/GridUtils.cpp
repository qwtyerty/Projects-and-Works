/**
 *	@brief		Utility module for functions used for the grid
 *	@details	Utility module that stores functions for translating a direction enum vlaue
 *				into a difference in row and in column and a function for getting the starting
 *				position of a given entity tile.
 */

#include "GridUtils.h"
#include <vector>

/**
 *	@brief		Translates a direction into a difference of row and col
 *	@details	Translates a direction into a difference of row and col
 *	@param		dir:	Direction to translate
 *  @param		row:	Pointer to row to store the offset 
 *	@param		col:	Pointer to column to store the offset
 *	@retval		None
 */
void grdUtilTranslateDirections(Direction dir, int* row, int* col)
{
	*row = 0;
	*col = 0;

	switch (dir)
	{
	case UP:
		*row = -1;
		break;

	case DOWN:
		*row = 1;
		break;

	case LEFT:
		*col = -1;
		break;

	case RIGHT:
		*col = 1;
		break;

	default:
		break;
	}

	return;
}

/**
 *	@brief		Gets the start position of a gridEntity
 *	@details	Iterate over the grid and search for the gridEntity
 *	@param		gridEntity:		Entity to be searching for
 *	@param		pPlayGrid:		Pointer to grid to read from
 *	@param		posRow:			Pointer to store the row location in
 *	@param		posCol:			Pointer to store the column location in
 */
void gridUtilGetStartPosition(GridType gridEntity, std::vector<std::vector<int>>* pPlayGrid, int* posRow, int* posCol)
{
	for (int row = 0; row < pPlayGrid->at(0).size(); row++)
	{
		for (int col = 0; col < pPlayGrid->size(); col++)
		{
			if ((*pPlayGrid)[row][col] == gridEntity)
			{
				*posCol = col;
				*posRow = row;

				return;
			}
		}
	}
}