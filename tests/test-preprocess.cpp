#include <vector>
#include <iomanip>
#include <fstream>
#include "catch.hpp"
#include "preprocessing.h"
#include "loader.h"
#include "balanced_min_cut.h"
#include "prefer_zero_cut.h"
using namespace std;

namespace TEST_PREPROCESS {
  const string default_map_path = "./tests/maps";
  const string default_testcase_path = "./tests/input/test-preprocess/";
  string mpath;
  int height, width;
  vector<bool> mapData;

  TEST_CASE("rwcard", "[.pre]") {
    int cnt = 0, hLevel = 0;
    ifstream file(default_testcase_path + "rwcard.in");
    while (file >> mpath >> hLevel) {
      string output = getMapName(mpath) + "-" + to_string(hLevel) + ".out";
      LoadMap(mpath.c_str(), mapData, width, height);
      Parameters p{"DFS", "rect", output, hLevel};
      PreprocessRectWildcard(mapData, width, height, p);
      cnt++;
    }
  }

  TEST_CASE("split-order", "[order]") {
    ifstream file(default_testcase_path + "split-order.in");
    while (file >> mpath) {
      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      NodeOrdering order = compute_split_dfs_order(extract_graph(mapper));
      REQUIRE(order.validate());
    }
  }

  TEST_CASE("dfs-order", "[order]") {
    ifstream file(default_testcase_path + "dfs-order.in");
    while (file >> mpath) {
      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      NodeOrdering order = compute_real_dfs_order(extract_graph(mapper));
      REQUIRE(order.validate());
    }
  }

  TEST_CASE("cut-order", "[order]") {
    ifstream file(default_testcase_path + "cut-order.in");
    while (file >> mpath) {
      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      NodeOrdering order = compute_cut_order(extract_graph(mapper), prefer_zero_cut(balanced_min_cut));
      REQUIRE(order.validate());
    }
  }

  TEST_CASE("centroid", "[.pre]") {
    int hLevel = 0;
    ifstream file(default_testcase_path + "centroid.in");
    while (file >> mpath >> hLevel) {
      string output = getMapName(mpath) + "-" + to_string(hLevel) + "-centroid.out";
      LoadMap(mpath.c_str(), mapData, width, height);
      Parameters p{"CUT", "vanilla", output, hLevel};
      PreprocessCentroid(mapData, width, height, p);
    }
  }
}
