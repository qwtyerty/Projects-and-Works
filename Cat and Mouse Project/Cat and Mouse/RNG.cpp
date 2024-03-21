/**
 *	@brief		Implementation of an RNG function for generating random ints
 *	@details	Uses a uniform int distribution along with a mersenne twister engine and
 *				time-based seed in order to generate a random integer within a specific bounds.
 */

#include "RNG.h"
#include <random>
#include <chrono>

/**
 *	@brief		Generate a random integer
 *	@details	Generate a random integer between min and max, non-inclusive
 *	@param		min:	Minimum integer that can be generated
 *	@param		max:	Non-inclusive maximum integer that can be generated
 *	@retval		Generated integer
 */
int RNGgenRandomInt(int min, int max) 
{
	// Create a device and generator for all function calls
	static unsigned seed = static_cast<unsigned> (std::chrono::system_clock::now().time_since_epoch().count());
	static std::mt19937 generator(seed);

	// Generate the uniform distribution
	std::uniform_int_distribution<int> distribution(min, max - 1);

	return distribution(generator);
}