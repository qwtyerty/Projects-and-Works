/**
 *	@brief		Header file for the Cat class
 *	@details	Defines the cat class including member functions and variables
 */
#pragma once
#include "Mouse.h"
#include <queue>

class Cat 
{
private:
	// Member variables
	int positionRow, positionCol, previousRow, previousCol;		// Position variables
	Direction nextMove;											// Next move 
	int lazyTurnsLeft;											// Number of turns where the cat does nothing
	int normalTurnsLeft;										// Number of turns where the cat searches for the mouse normally
	std::vector<std::vector<int>>* pPlayGrid;					// Pointer to the play grid for easier reference
	Mouse *pMouse;												// Pointer to the mouse for easier reference to its location

	// Member functions
	void selectGreedyMove();																		// Behavior funcation for selecting the move using a greddy algorithm
	std::pair<int, int> checkPossibleMoves(std::vector<std::vector<std::pair<int, int>>> &grid, int row, int col, std::queue<std::pair<int, int>>& searchQueue);	// Checks all possible moves then adds them to the searchQueue
	bool validateMove(std::vector<std::vector<std::pair<int, int>>> &grid, int row, int col);		// Helper function that verifies a given move is legal and has not been previously visited
	void reconstructPath(std::vector<std::vector<std::pair<int, int>>>& grid, int row, int col);	// Helper function that follows the path in reverse to determine the move needed to take
	void selectOptimalMove();																		// Behavior function for selecting an optimal move using BFS
	void selectBehavior();																			// Selects the behavior of the cat for the next move

public:
	// Member functions
	Cat(std::vector<std::vector<int>>* _pPlayGrid, Mouse *_pMouse);			// Constructor for the cat
	void selectNextMove();													// Behavior function for selecting the next move
	void movePosition();													// Behavior function for applying a selected move
	std::pair<int, int> getCurrentPosition();								// Getter function for the current position of the mouse
	std::pair<int, int> getPreviousPosition();								// Getter function for the previous position of the mouse

};

