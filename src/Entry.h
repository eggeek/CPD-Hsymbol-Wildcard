#ifndef ENTRY_H
#define ENTRY_H
#include <stdint.h>
#include "jpsp_oracle.h"

struct xyLoc {
  int16_t x;
  int16_t y;
};

void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename, int hLevel);
void PreprocessRectWildcard(std::vector<bool> &bits, int width, int height, const char *filename, int hLevel);
void *PrepareForSearch(std::vector<bool> &bits, int width, int height, const char *filename);
double GetPathCostSRC(void *data, xyLoc s, xyLoc g, int hLevel, int limit=-1);
double GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path,
    warthog::jpsp_oracle& oracle, int hLevel, int limit=-1);//, int &callCPD);
const char *GetName();
void LoadMap(const char *fname, std::vector<bool> &map, int &w, int &h);

#endif
