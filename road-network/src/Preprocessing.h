#pragma once
#include <stdint.h>
#include "mapper.h"
#include "cpd.h"
using namespace std;

struct State{
  CPD cpd;
  Mapper mapper;
  AdjGraph graph;
  int current_node;
  int target_node;
  vector<Sectors> sectors;
};


void PreprocessMapPurely(
  int order_code, const ListGraph& listg,
  const vector<xyLoc>& coord, const char *filename,
  int hLevel, long long quant=1
);

void PreprocessMapWithSectorWildCard(
  int order_code, const ListGraph& listg,
  const vector<xyLoc>& coord, const char *filename,
  int hLevel, bool ext = false, long long quant=1
);

void *PrepareForSearchPurely(
  const ListGraph& listg,
  const vector<xyLoc>& coord,
  const char *filename
);

void *PrepareForSearchSectorWildCard(
  const ListGraph& listg,
  const vector<xyLoc>& coord,
  const char *filename
);
