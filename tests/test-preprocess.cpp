#include <vector>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <time.h>
#include <algorithm>
#include <random>
#include <square_wildcard.h>
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
      Parameters p{"DFS", "fwd", output, 3, centroid};
      PreprocessMap(mapData, width, height, p);
    }
  }

  TEST_CASE("build", "[.pre]") {
    int centroid = 0;
    string itype;
    ifstream file(default_testcase_path + "build.in");

    string output = "build-tcost.txt";
    ofstream out(output);
    string header = "map,itype,r,tcost";
    out << header << endl; 
    while (file >> mpath >> itype >> centroid) {
      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      AdjGraph g(extract_graph(mapper));
      string mapname = getMapName(mpath.c_str());

      auto stime = std::chrono::steady_clock::now();
      if (centroid == 0) {
        if (itype == "inv") {
          string output = getMapName(mpath) + "-opt-inv.out";
          Parameters p{"DFS", "inv", output, 3, centroid};
          PreprocessMap(mapData, width, height, p);
        }
        else {
          string output = getMapName(mpath) + "-opt.out";
          Parameters p{"DFS", "fwd", output, 3, centroid};
          PreprocessMap(mapData, width, height, p);
        }
      }
      else if (itype == "inv") {
        string output = getMapName(mpath) + "-" + to_string(centroid) + "-inv.out";
        Parameters p{"DFS", "inv", output, 3, centroid};
        PreprocessMap(mapData, width, height, p);
      }
      else {
        string output = getMapName(mpath) + "-" + to_string(centroid) + ".out";
        Parameters p{"DFS", "fwd", output, 3, centroid};
        PreprocessMap(mapData, width, height, p);
      }
      auto etime = std::chrono::steady_clock::now();
      double tcost = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();
      out << mapname << "," << itype << "," << centroid << "," << tcost << endl;
    }
  }

  TEST_CASE("square-wildcard", "[.pre]") {
    ifstream file(default_testcase_path + "square-wildcard.in");
    xyLoc s;
    int hLevel = 3;
    while (file >> mpath >> s.x >> s.y) {
      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      AdjGraph g(extract_graph(mapper));
      Dijkstra dij(g, mapper);
      dij.run(mapper(s), hLevel);
      SquareWildcard sq(mapper, s);
      int side = sq.computeMaxSquare(dij.get_allowed());
    }
  }

  TEST_CASE("csymbol", "[.computed]") {
    ifstream file(default_testcase_path + "centroid-symbol.in");
    xyLoc s;
    string indexpath;
    int hLevel = 3;
    while (file >> mpath >> indexpath) {
      LoadMap(mpath.c_str(), mapData, width, height);
      Index data = LoadIndexData(mapData, width, height, indexpath.c_str());
      REQUIRE(data.p.csymbol > 0);
      Mapper mapper = data.mapper;
      AdjGraph g(extract_graph(mapper));
      Dijkstra dij(g, mapper);
      Dijkstra dijc(g, mapper);
      while (file >> s.x >> s.y) {
        if (s.x == 0 && s.y == 0) break;
        int sid = data.mapper(s);
        int cent = data.mapper.get_fa()[sid];
        vector<int> temp_sides, temp_sidesc;

        dij.run(sid, hLevel, temp_sides);
        REQUIRE(data.square_sides[sid] == temp_sides.back());

        dijc.run(cent, hLevel, temp_sidesc);
        REQUIRE(data.square_sides[cent] == temp_sidesc.back());

        for (int i=0; i<data.mapper.node_count(); i++) if (i != sid) {
          xyLoc loc = data.mapper(i);
          int m = data.cpd.get_first_move(sid, i);
          if ((1<<m) != warthog::CENTMASK) continue;
          if (data.cpd.is_in_square(i, data.square_sides[sid], sid, data.mapper)) continue;
          int side = data.square_sides[cent];
          int move;
          if (data.cpd.is_in_square(i, side, cent, data.mapper))
            move = warthog::m2i.at(warthog::HMASK);
          else
            move = data.cpd.get_first_move(cent, i);
          int mask1 = dij.get_allowed()[i];
          int mask2 = dijc.get_allowed()[i];
          REQUIRE((mask1 & (1<<move)) > 0);
        }
      }
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
      unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
      std::shuffle(arr.begin(), arr.end(), std::default_random_engine(seed));
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
