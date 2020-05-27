#include "adj_graph.h"
#include "catch.hpp"
#include "dijkstra.h"
#include "lifelong_dij.h"
#include "loader.h"
#include "mapper.h"
#include <limits>
#include <tuple>
using namespace std;

namespace TEST_LIFELONG_DIJ {

  vector<string> mpaths = {
    "./tests/maps/test.map",
    "./tests/maps/arena.map",
    "./maps/brc000d.map"
    "./maps/bg/AR0204SR.map"
    "./maps/bg/AR0205SR.map"
  };
  int height, width;
  vector<bool> mapData;

  void build_succ(vector<int>& succ, const vector<int>& dist, const AdjGraph& g) {
    for (int i=0; i<g.node_count(); i++) {
      succ[i] = 0;
      for (auto& a: g.out(i)) {
        if (dist[i] + a.weight == dist[a.target]) succ[i] |= 1 << a.direction;
      }
    }
  }

  TEST_CASE("ll-dij-trivial", "[run-ll-dij]") {
    for (string mpath: { 
        "./tests/maps/test.map", 
        "./tests/maps/arena.map", 
        "./maps/brc000d.map"
    }) {
      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      AdjGraph g(extract_graph(mapper));
      Dijkstra dij(g, mapper);
      LifeLongDijkstra lldij(g, mapper);

      vector<int> succ(g.node_count(), 0);
      vector<int> dist(g.node_count(), numeric_limits<int>::max());

      int s = 0;
      lldij.run(s, dist, succ);
      dij.run(s, 0);

      const vector<int>& dist_dij = dij.get_dist();
      const vector<int>& dist_lldij = lldij.get_dist();
      for (int i=0; i<g.node_count(); i++) {
        REQUIRE(dist_dij[i] == dist_lldij[i]);
      }
    }
  }

  TEST_CASE("ll-dij-pre-pair", "[run-ll-dij]") {
    vector<tuple<string, int, int>> argvs = {
      {"./tests/maps/test.map",   24, 35      },
      {"./tests/maps/arena.map",  1, 2      },
      {"./tests/maps/arena.map",  744, 540  },
      {"./tests/maps/arena.map",  540, 744  },
      {"./maps/brc000d.map",      5488, 5614},
      {"./maps/brc000d.map",     5614, 5487},
    };

    for (auto& argv: argvs) {
      string mpath = get<0>(argv);
      int s0 = get<1>(argv), s1 = get<2>(argv);
      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      AdjGraph g(extract_graph(mapper));
      Dijkstra dij(g, mapper);
      LifeLongDijkstra lldij(g, mapper);

      vector<int> succ(g.node_count());
      dij.run(s0, 0);
      cerr << "building succ" << endl;
      fill(succ.begin(), succ.end(), 0);
      build_succ(succ, dij.get_dist(), g);

      cerr << "running lifelong dij" << endl;
      lldij.run(s1, dij.get_dist(), succ);

      dij.run(s1, 0);
      const vector<int>& dist_dij = dij.get_dist();
      const vector<int>& dist_lldij = lldij.get_dist();
      for (int i=0; i<g.node_count(); i++) {
        if (dist_dij[i] != dist_lldij[i])
          lldij.inq.p(mapper(i).x, mapper(i).y, s1, i, s1, 512);
        REQUIRE(dist_dij[i] == dist_lldij[i]);
      }
    }
  }

  TEST_CASE("ll-dij-pre-list", "[run-ll-dij]") {
    vector<tuple<string, vector<int>>> argvs = {
      {"./maps/dao/den204d.map", {1493, 1550, 1651}},
      {"./tests/maps/arena.map", {0,1,2,3,4,5,6,7,8,9,10,11,41}},
      {"./maps/brc000d.map",     {5488, 5614, 5487, 5360, 5243, 5242, 5131, 5130, 5020, 5019, 4910, 4909}},
      {"./maps/bg/AR0205SR.map", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 22, 36}},
      {"./tests/maps/arena.map", {3,4}},
      {"./maps/gppc/AcrosstheCape.map", {0, 1, 3, 4, 5, 6, 7, 8, 9, 10}},
    };
    for (auto& argv: argvs) {
      string mpath = get<0>(argv);
      vector<int> sources = get<1>(argv);
      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      AdjGraph g(extract_graph(mapper));
      Dijkstra dij(g, mapper);
      LifeLongDijkstra lldij(g, mapper);
      vector<int> succ(g.node_count());

      cerr << ">>>>> Map: " << mpath << ", size: " << g.node_count() << endl;
      double lldij_cost = 0, dij_cost = 0;
      auto stime = std::chrono::steady_clock::now();
      dij.run(sources[0], 0);
      auto etime = std::chrono::steady_clock::now();
      double init = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();
      lldij_cost += init;
      dij_cost += init;
      cerr << "init: (" << mapper(sources[0]).x << ", " << mapper(sources[0]).y << ")" << endl;
      for (int i=1; i<(int)sources.size(); i++) {
        cerr << "(" << mapper(sources[i]).x << ", " << mapper(sources[i]).y << ")" << endl;
        stime = std::chrono::steady_clock::now();
        build_succ(succ, dij.get_dist(), g);
        lldij.run(sources[i], dij.get_dist(), succ);
        etime = std::chrono::steady_clock::now();
        lldij_cost += std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();

        stime = std::chrono::steady_clock::now();
        dij.run(sources[i], 0);
        etime = std::chrono::steady_clock::now();
        dij_cost += std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();
        const auto& dist_dij = dij.get_dist();
        const auto& dist_lldij = lldij.get_dist();
        for (int j=0; j<g.node_count(); j++) {
          if (dist_dij[j] != dist_lldij[j])
            lldij.inq.p(mapper(j).x, mapper(j).y, sources[i], j, sources[i], 512);
          REQUIRE(dist_dij[j] == dist_lldij[j]);
        }
      }
      cerr << "dij tcost: " << dij_cost << ", lldij tcost: " << lldij_cost 
           << ", speed up: " << (dij_cost - lldij_cost) / dij_cost << endl;
      cerr << "==============" << endl;
    }
  }
}
