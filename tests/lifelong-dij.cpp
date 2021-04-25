#include "adj_graph.h"
#include "catch.hpp"
#include "dijkstra.h"
#include "lifelong_dij.h"
#include "loader.h"
#include "mapper.h"
#include <limits>
#include <tuple>
#include <chrono>
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

  void get_path(int s, int t, Dijkstra& dij, AdjGraph& g, vector<int>& path) {
    dij.run(s, 0);
    path.clear();
    int d = dij.get_dist()[t]; 
    int cur = t;
    while (d > 0) {
      path.push_back(cur);
      int pre = -1;
      for (auto a: g.out(cur)) if (dij.get_dist()[a.target] + a.weight == d) {
        pre = a.target;
        d -= a.weight;
        break;
      }
      assert(pre != -1);
      cur = pre;
    }
    path.push_back(s);
    reverse(path.begin(), path.end());
  }

  void build_from_direction(vector<int>& from_direction, const vector<int>& dist, const AdjGraph& g) {
    fill(from_direction.begin(), from_direction.end(), -1);
    for (int i=0; i<g.node_count(); i++) {
      for (auto& a: g.out(i)) {
        if (dist[i] + a.weight == dist[a.target]) 
          from_direction[a.target] = a.direction;
      }
    }
  }

  void validate_dist(const vector<int>& dist, 
    const vector<int>& dist_ll, const AdjGraph& g) {
    for (int i=0; i<g.node_count(); i++) {
      REQUIRE(dist[i] == dist_ll[i]);
    }
  }

  void validate_from_direction(
    const vector<int>& fromd, const vector<int>& dist,
    const vector<int>& fromd_ll, const vector<int>& dist_ll,
    const AdjGraph& g) {
    int n = fromd.size();
    REQUIRE(dist.size() == n);
    REQUIRE(fromd_ll.size() == n);
    REQUIRE(dist_ll.size() == n);

    for (int i=0; i<n; i++) {
      for (auto& a: g.out(i)) 
      if (fromd[a.target] == a.direction) {
        REQUIRE(dist[i] + a.weight == dist[a.target]);
      }
      for (auto& a: g.out(i)) 
      if (fromd_ll[a.target] == a.direction) {
        assert(dist_ll[i] + a.weight == dist_ll[a.target]);
        REQUIRE(dist_ll[i] + a.weight == dist_ll[a.target]);
      }
    }
  }

  TEST_CASE("ll-dij") {
    vector<tuple<string, vector<int>>> argvs = {
      {"./maps/dao/den204d.map", {1493, 1550}},
      {"./tests/maps/arena.map", {3,4}},
      {"./maps/gppc/AcrosstheCape.map", {2312, 23256}},
      {"./maps/brc000d.map",     {5488, 4909}},
    };

    for (auto& argv: argvs) {
      string& mpath = get<0>(argv);
      vector<int> nodes = get<1>(argv);
      vector<int> succ, from_direction;
      vector<xyLoc> coord;
      vector<int> dist, lldist;


      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      AdjGraph g(extract_graph(mapper));
      coord.resize(g.node_count());
      succ.resize(g.node_count());
      from_direction.resize(g.node_count());
      Dijkstra dij(g, mapper);
      LifeLongDijkstra lldij(g, mapper);

      if (nodes.size() == 2) {
        get_path(nodes[0], nodes[1], dij, g, nodes);
      }

      double lldij_cost = 0, dij_cost = 0, avg_prop = 0, avg_expan = 0;
      dij.run(nodes[0], 0);
      dist = vector<int>(dij.get_dist());
      build_from_direction(from_direction, dist, g);
      lldij.set_from_direction(from_direction);

      for (int i=1; i<(int)nodes.size(); i++) {

        auto stime = std::chrono::steady_clock::now();
        dij.run(nodes[i], 0);
        auto etime = std::chrono::steady_clock::now();
        dij_cost += std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();

        stime = std::chrono::steady_clock::now();
        lldij.init(dist, nodes[i], dist[nodes[i]]);
        auto res = lldij.run(nodes[i], dist);
        etime = std::chrono::steady_clock::now();
        avg_prop += res.first;
        avg_expan += res.second;

        lldij_cost += std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();

        // Checking
        dist = vector<int>(dij.get_dist());
        lldist = vector<int>(lldij.get_dist());
        build_from_direction(from_direction, dist, g);
        validate_dist(dist, lldist, g);
        validate_from_direction(from_direction, dist, lldij.get_from_direction(), lldist, g);
      }
      cerr << "dij tcost: " << dij_cost << ", lldij tcost: " << lldij_cost 
           << ", speed up: " << dij_cost / lldij_cost
           << ", #node: " << nodes.size()
           << ", avg propagate: " << avg_prop / (double)nodes.size()
           << ", v: " << g.node_count()
           << ", avg expan: " << avg_expan / (double)nodes.size() << endl;
      cerr << "==============" << endl;
    }
  }


}
