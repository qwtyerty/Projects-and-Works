/** 
 *	@brief		Header file for the Grid Utilities Module
 *	@details	Delcares functions for the module and additionally provides definitions for 
 *				the Direction and GridType enums for use in other modules.
 */

#pragma once
#include <vector>

enum Direction
{
	UP,
	DOWN,
	LEFT,
	RIGHT,
	NUM_DIRECTIONS
};

enum GridType
{
	EMPTY,			// 0
	WALL,			// 1
	MOUSE,			// 2
	CAT,			// 3
	CHEESE,			// 4
	NUM_TILE_TYPES	// 5
};

void grdUtilTranslateDirections(Direction dir, int* row, int* col);														//Tranlate direction into dx dy
void gridUtilGetStartPosition(GridType gridEntity, std::vector<std::vector<int>>* pPlayGrid, int* posRow, int* posCol);		// Helper Function used to find starting coords