#include <vector>
#include <fstream>
#include "catch.hpp"
#include "loader.h"
using namespace std;

namespace TEST_LOAD {
  const string default_testcase_path = "./tests/input/test-load/";
  string mpath, indexpath;
  int height, width;
  vector<bool> mapData;

  TEST_CASE("load-index", "[.load]") {
    ifstream file(default_testcase_path + "load-index.in");
    while (file >> mpath >> indexpath) {
      LoadMap(mpath.c_str(), mapData, width, height);
      vector<int> centroids;
      Index data;
      data.mapper = Mapper(mapData, width, height);

      FILE* f = fopen(indexpath.c_str(), "rb");
      data.p.load(f);
      if (data.p.itype == "fwd") {
        data.square_sides = load_vector<int>(f);
        REQUIRE((int)data.square_sides.size() == data.mapper.node_count());
      }
      if (data.p.centroid) {
        centroids = load_vector<int>(f);
        REQUIRE((int)centroids.size() == data.mapper.node_count());
      }
      NodeOrdering order;
      order.load(f);
      REQUIRE(order.validate() == true);
      REQUIRE(order.is_complete() == true);
      REQUIRE(order.node_count() == data.mapper.node_count());

      data.cpd.load(f);
      data.mapper.reorder(order);
      if (data.p.centroid)
        data.mapper.set_centroids(centroids);
    }
  }
}
