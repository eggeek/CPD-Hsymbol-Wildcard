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

  int height, width;
  vector<bool> mapData;

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

TEST_CASE("dfs-ll-dij") {
  vector<string> argvs = 
  {      
    // "./maps/dao/den204d.map",
    // "./tests/maps/arena.map",
    // "./maps/brc000d.map",
    "./maps/gppc/AcrosstheCape.map",
  };
  const long long INF = numeric_limits<int>::max();
  for (auto& argv: argvs) {
    string& mpath= argv;
    LoadMap(mpath.c_str(), mapData, width, height);
    Mapper mapper(mapData, width, height);
    NodeOrdering order = compute_real_dfs_order(extract_graph(mapper));
    // NodeOrdering order = compute_dij_dfs_order(mapper, 0);
    mapper.reorder(order);
    AdjGraph g(extract_graph(mapper));

    Dijkstra dij(g, mapper);
    LifeLongDijkstra lldij(g, mapper);

    double lldij_cost = 0, dij_cost = 0, avg_prop = 0, avg_expan = 0;

    vector<int> dist = vector<int>(g.node_count(), 0);
    vector<int> lldist = vector<int>(g.node_count(), 0);
    vector<int> from_direction = vector<int>(mapper.node_count(), 16);
    lldij.set_from_direction(from_direction);

    auto stime = std::chrono::steady_clock::now();
    for (int i=0; i<10; i++) {
      dij.run(i, 0);
    }
    auto etime = std::chrono::steady_clock::now();
    double sampledij = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();
    sampledij /= 10.0;

    int num = min(g.node_count(), 10000);
    for (int i=0; i<num; i++) {
      auto stime = std::chrono::steady_clock::now();
      dij.run(i, 0);
      auto etime = std::chrono::steady_clock::now();
      dij_cost += std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();

      stime = std::chrono::steady_clock::now();
      long long delta = i == 0? INF: dij.get_dist()[i-1];
      lldij.init(lldist, i, delta);
      auto res = lldij.run(i, lldist);
      etime = std::chrono::steady_clock::now();

      avg_prop += res.first;
      avg_expan += res.second;
      double arun = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();
      lldij_cost += arun;

      // Checking
      dist = vector<int>(dij.get_dist());
      lldist = vector<int>(lldij.get_dist());
      build_from_direction(from_direction, dist, g);
      validate_dist(dist, lldist, g);
      validate_from_direction(from_direction, dist, lldij.get_from_direction(), lldist, g);

      if (i % 10 == 0) {
        cout << "avg prop: " << avg_prop / (double)(i+1)
             << ", avg expan: " << avg_expan / (double)(i+1)
             << ", speed up: " << dij_cost / lldij_cost << endl;
      }
    }
    cout << "dij tcost: " << dij_cost << ", lldij tcost: " << lldij_cost 
         << ", speed up: " << dij_cost / lldij_cost
         << ", avg propagate: " << avg_prop / (double)num
         << ", v: " << g.node_count()
         << ", avg expan: " << avg_expan / (double)num << endl;
    cout << "==============" << endl;

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
      vector<int> from_direction;
      vector<xyLoc> coord;
      vector<int> dist, lldist;


      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      AdjGraph g(extract_graph(mapper));
      coord.resize(g.node_count());
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
      cout << "dij tcost: " << dij_cost << ", lldij tcost: " << lldij_cost 
           << ", speed up: " << dij_cost / lldij_cost
           << ", #node: " << nodes.size()
           << ", avg propagate: " << avg_prop / (double)nodes.size()
           << ", v: " << g.node_count()
           << ", avg expan: " << avg_expan / (double)nodes.size() << endl;
      cout << "==============" << endl;
    }
  }


}
