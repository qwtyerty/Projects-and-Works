/**
 *	@brief		Header file for the Mouse class
 *	@details	Defines the mouse class including member functions and variables
 */

#pragma once
#include <vector>
#include "GridUtils.h"

class Mouse
{
	private:
		// Member variables
		int positionRow, positionCol, previousRow, previousCol;	// Position variables
		Direction nextMove;										// Next move

		// Member Functions
		std::vector<std::vector<int>>* pPlayGrid;				// Pointer to the play grid for easier reference
		bool searchForCheese();									// Behavior Function for checking if cheese is visible
		bool cheeseSpotted(Direction sightLine);				// Helper function to see if cheese is visible
		void selectRandomMove();								// Behavior Function used for selecting a random move

	public:
		// Member functions
		Mouse(std::vector<std::vector<int>>* playGrid);			// Mouse constructor
		void selectNextMove();									// Behavior function for selecting the next move
		void movePosition();									// Behavior function for applying a selected move
		std::pair<int, int> getCurrentPosition();				// Getter function for the current position of the mouse
		std::pair<int, int> getPreviousPosition();				// Getter function for the previous position of the mouse
		void spawnNewPosition();								// function for spawning the mouse in a new position
};



