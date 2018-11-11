#ifndef ENTRY_H
#define ENTRY_H
#include <stdint.h>
#include "stats.h"

void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename, const bool random_row, const bool old_array,
		const bool mix, const bool plus_heuristic, const bool plus_heuristic_2, const int k_factor, const int threshold);
void *PrepareForSearch(std::vector<bool> &bits, int width, int height, const char *filename);
void GetPath(void *data, const xyLoc s, const xyLoc g, std::vector<xyLoc> &thePath, std::vector<xyLoc> &backward_segment, const bool plus_heuristic, const bool plus_heuristic_2, const int k_factor, const int threshold);
const char *GetName();

#ifdef DETAILS
int Get20ForwardMoveDetails(void *data, const xyLoc s, const xyLoc g, std::vector<xyLoc> &first_20_moves, std::vector<xyLoc> &backward_segment, const bool plus_heuristic_2, const bool plus_heuristic, const int k_factor, const int threshold);
int GetStepFirstForwardMove(void *data, const xyLoc s, const xyLoc g, std::vector<xyLoc> &thePath, std::vector<xyLoc> &backward_segment, const bool plus_heuristic, const bool plus_heuristic_2, const int k_factor, const int threshold);
#endif

#endif
