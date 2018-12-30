#pragma once

/**
 * Processes cork images and returns cork found and defect found.
 */
class CorkAnalyser
{
public:
	
	/**
	 * Check if a value is in the neighborhood of another one.
	 *
	 * @param value
	 * @param center
	 * @param neighborhood
	 */
	static bool isNeighbor(int value, int center, int neighborhood)
	{
		return value > (center - neighborhood) && value < (center + neighborhood);
	}
};