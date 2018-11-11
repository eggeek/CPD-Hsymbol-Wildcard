/*
 * stats.h
 *
 *  Created on: 3 Oct 2016
 *      Author: matteo
 */

#ifndef STATS_H_
#define STATS_H_

#include <algorithm>


struct xyLoc {
  int16_t x;
  int16_t y;
};

struct Rect {
	int n = 0;
	int s = 0;
	int e = 0;
	int w = 0;
};


struct Stats {

	double total_time;
	double first_20_moves_fwd_time;
	double first_move_fwd_time;
	std::vector<xyLoc> path;
	std::vector<xyLoc> first_20_moves;
	std::vector<int> lengths;
	int step_first_forward_move;
	int step_20_forward_move;

	double GetTotalTime()
	{
		return total_time;
	}
	double Get20MoveTime()
	{
		return first_20_moves_fwd_time;
	}
	double GetFirstMoveTime()
	{
		return first_move_fwd_time;
	}
	double GetPathLength()
	{
		double len = 0;
		for (int x = 0; x < (int)path.size()-1; x++)
		{
			if (path[x].x == path[x+1].x || path[x].y == path[x+1].y)
			{
				len++;
			}
			else {
				len += 1.4142;
			}
		}
		return len;
	}
	bool ValidatePath(int width, int height, const std::vector<bool> &mapData)
	{
		for (int x = 0; x < (int)path.size()-1; x++)
		{
			if (abs(path[x].x - path[x+1].x) > 1)
				return false;
			if (abs(path[x].y - path[x+1].y) > 1)
				return false;
			if (!mapData[path[x].y*width+path[x].x])
				return false;
			if (!mapData[path[x+1].y*width+path[x+1].x])
				return false;
			if (path[x].x != path[x+1].x && path[x].y != path[x+1].y)
			{
				if (!mapData[path[x+1].y*width+path[x].x])
					return false;
				if (!mapData[path[x].y*width+path[x+1].x])
					return false;
			}
		}
		return true;
	}
};

#endif /* STATS_H_ */
