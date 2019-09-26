#pragma once
#include <vector>
#include <cstdio>
#include <stdexcept>
#include "jpsp_oracle.h"
#include "cpd_base.h"
#include "rect_wildcard.h"
#include "mapper.h"
using namespace std;

struct Parameters {
  string otype; // ordering type: dfs, split, cut
  string itype; // index type: fwd, rect, inv
  string filename;
  int hLevel; // heuristic level: 0, 1, 2, 3
  int centroid;
  void save(FILE* f) const {
    save_string(f, otype);
    save_string(f, itype);
    if (std::fwrite(&hLevel, sizeof(int), 1, f) != 1)
      throw runtime_error("std::fwrite failed");
    if (std::fwrite(&centroid, sizeof(int), 1, f) != 1)
      throw runtime_error("std::fwrite failed");
  }

  void load(FILE* f) {
    otype = load_string(f);
    itype = load_string(f);
    if (std::fread(&hLevel, sizeof(int), 1, f) != 1)
      throw runtime_error("std::fread failed");
    if (std::fread(&centroid, sizeof(int), 1, f) != 1)
      throw runtime_error("std::fread failed");
  }
};

struct Index {
  CPDBASE cpd;
  Mapper mapper;
  AdjGraph graph;
  RectWildcardIndex rwobj;
  Parameters p;
  vector<int> row_ordering;
  vector<int> square_sides;
  Index(){}
};

void PreprocessMap(vector<bool>& bits, int width, int height, const Parameters& p);
