#include <vector>
#include <iomanip>
#include <fstream>
#include "catch.hpp"
#include "preprocessing.h"
#include "loader.h"
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
      PreprocessRectWildcard(mapData, width, height, output, hLevel);
      cnt++;
    }
  }
}
