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
  RectWildcardIndex rwobj;
  vector<int> row_ordering;
  vector<int> square_sides;
  vector<unsigned char> reverse_moves;
  Index(){}
};

struct Parameters {
  string otype; // ordering type: dfs, split, cut
  string itype; // index type: vanilla, rect, inv
  string filename;
  int hLevel; // heuristic level: 0, 1, 2, 3
};

void PreprocessMap(std::vector<bool> &bits, int width, int height, const Parameters& p);
void PreprocessRectWildcard(std::vector<bool> &bits, int width, int height, const Parameters& p);
