#pragma once
#include <vector>
#include "jpsp_oracle.h"
#include "cpd.h"
#include "rect_wildcard.h"
#include "mapper.h"
using namespace std;

struct Index {
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
