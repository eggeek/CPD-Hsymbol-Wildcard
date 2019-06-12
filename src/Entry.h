#pragma once
#include <stdint.h>
#include "jpsp_oracle.h"
#include "cpd.h"
using namespace std;

struct xyLoc {
  int16_t x;
  int16_t y;
};

struct State{
  CPD cpd;
  Mapper mapper;
  AdjGraph graph;
  vector<int> row_ordering;
  vector<int> square_sides;
  vector<RectInfo> rects;
  vector<unsigned char> reverse_moves;
  int current_node;
  int target_node;
};

void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename, int hLevel);
void PreprocessRectWildcard(std::vector<bool> &bits, int width, int height, const char *filename, int hLevel);
State *PrepareForSearch(std::vector<bool> &bits, int width, int height, const char *filename);
State LoadRectWildCard(vector<bool>& bits, int width, int height, const char* fname);
double GetPathCostSRC(void *data, xyLoc s, xyLoc g, int hLevel, int limit=-1);
double GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path,
    warthog::jpsp_oracle& oracle, int hLevel, int limit=-1);//, int &callCPD);
const char *GetName();
void LoadMap(const char *fname, std::vector<bool> &map, int &w, int &h);
