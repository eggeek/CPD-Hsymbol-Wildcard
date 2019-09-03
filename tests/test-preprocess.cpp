#include <vector>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <time.h>
#include <algorithm>
#include "catch.hpp"
#include "dijkstra.h"
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
      Parameters p{"DFS", "rect", output, hLevel, 0};
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
    int centroid = 0;
    ifstream file(default_testcase_path + "centroid.in");
    while (file >> mpath >> centroid) {
      string output = getMapName(mpath) + "-" + to_string(centroid) + "-centroid.out";
      LoadMap(mpath.c_str(), mapData, width, height);
      Parameters p{"DFS", "vanilla", output, 3, centroid};
      PreprocessCentroid(mapData, width, height, p);
    }
  }

  TEST_CASE("dijkstra", "[.pre]") {
    int repeat = 10;
    int queries = 1000;
    ifstream file(default_testcase_path + "dijkstra.in");
    string output = "dijkstra-tcost.txt";
    ofstream out(output);
    srand(time(NULL));
    string header = "map,i,source,s,h,w,e";
    out << header << endl;
    while (file >> mpath) {
      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      AdjGraph g(extract_graph(mapper));
      vector<int> arr;
      arr.resize(mapper.node_count());
      for (int i=0; i<(int)arr.size(); i++) 
        arr[i] = i;
      std::random_shuffle(arr.begin(), arr.end());
      queries = min(queries, mapper.node_count());
      Dijkstra dij(g, mapper);

      vector<unsigned short> allowed, inv_allowed;
      string mapname = getMapName(mpath);

      for (int i=0; i<repeat; i++) {
        for (int j=0; j<queries; j++) {
          auto stime = std::chrono::steady_clock::now();
            dij.run(arr[j], 0);
          auto etime = std::chrono::steady_clock::now();
          double searchcost = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();

          inv_allowed = vector<unsigned short>(dij.get_inv_allowed().begin(), dij.get_inv_allowed().end());
          allowed = vector<unsigned short>(dij.get_allowed().begin(), dij.get_allowed().end());

          stime = std::chrono::steady_clock::now();
            H::encode(arr[j], allowed, mapper, 3);
          etime = std::chrono::steady_clock::now();
          double hcost = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();

          stime = std::chrono::steady_clock::now();
            SquareWildcard(mapper, mapper(arr[j])).computeMaxSquare(allowed);
          etime = std::chrono::steady_clock::now();
          double scost = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();

          stime = std::chrono::steady_clock::now();
            H::add_extr_inv_move(arr[j], inv_allowed, mapper);
          etime = std::chrono::steady_clock::now();
          double ecost = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();

          out << mapname << "," << i << "," << arr[j] << "," << searchcost << "," << hcost << "," << scost << "," << ecost << endl;
        }
      }
    }
  }
}
