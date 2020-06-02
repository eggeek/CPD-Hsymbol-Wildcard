#include "adj_graph.h"
#define CATCH_CONFIG_RUNNER
#include <tuple>
#include "catch.hpp"
#include "Preprocessing.h"
#include "Query.h"
#include "dijkstra.h"
#include "RoadNetworkLoader.h"
#include "order.h"
#include "geo.h"
#include "sector_wildcard.h"
#include "lifelong_dij.h"

using namespace std;

TEST_CASE("Load") {
  string grpath = "./test/maps/sample0/sample0.gr";
  ListGraph listg = RoadNetwork::Load(grpath);
  vector<xyLoc> coord;
  coord.resize(listg.node_count());
  Mapper mapper(listg, coord);
  NodeOrdering order = compute_real_dfs_order(listg);
  mapper.reorder(order);

  AdjGraph g(listg);
  Dijkstra dij(g, mapper);
  dij.run(0, 0);
  REQUIRE(dij.distance(1) == 2);
  REQUIRE(dij.distance(2) == 5);
  REQUIRE(dij.distance(3) == 6);
  REQUIRE(dij.distance(4) == 8);
}

TEST_CASE("LoadSub") {
  string mname = "NY";
  string grpath = "./maps/" + mname + "/" + mname + ".gr";
  string copath = "./maps/" + mname + "/" + mname + ".co";
  vector<xyLoc> coord;
  ListGraph listg = RoadNetwork::LoadSub(grpath, copath, coord, 0.03);
  printf ("Node: %d, Arc: %d\n", (int)coord.size(), (int)listg.arc.size());
}

TEST_CASE("CPD") {
  string mname = "ny3";
  string grpath = "./test/maps/" + mname + "/" + mname + ".gr";
  ListGraph listg = RoadNetwork::Load(grpath);
  vector<xyLoc> coord;
  int order_code = 0;
  coord.resize(listg.node_count());
  int hLevel = 0;
  string fname = "./index_data/" + mname + "-" + to_string(hLevel) +  ".cpd";
  PreprocessMapPurely(order_code, listg, coord, fname.c_str(), hLevel);

  hLevel = 1;
  fname = "./index_data/" + mname + "-" + to_string(hLevel) +  ".cpd";
  PreprocessMapPurely(order_code, listg, coord, fname.c_str(), hLevel);
}

TEST_CASE("Degree") {
  string grpath;

  grpath = "./maps/NY/NY.gr";
  ListGraph listg = RoadNetwork::Load(grpath);
  AdjGraph adj(listg);
  printf("Max degree of %s : %d, vert: %d\n", grpath.c_str(), adj.max_degree(), adj.max_degree_vert());

  grpath = "./maps/COL/COL.gr";
  listg = RoadNetwork::Load(grpath);
  adj = AdjGraph(listg);
  printf("Max degree of %s : %d, vert: %d\n", grpath.c_str(), adj.max_degree(), adj.max_degree_vert());



  grpath = "./maps/Rome99/rome99.gr";
  listg = RoadNetwork::Load(grpath);
  adj = AdjGraph(listg);
  printf("Max degree of %s : %d, vert: %d\n", grpath.c_str(), adj.max_degree(), adj.max_degree_vert());

}

TEST_CASE("CPDh") {
  string mname = "NY";
  string grpath = "./maps/" + mname + "/" + mname + ".gr";
  string copath = "./maps/" + mname + "/" + mname + ".co";
  vector<xyLoc> coord;
  double ratio = 0.03;
  int order_code = 0;
  ListGraph listg = RoadNetwork::LoadSub(grpath, copath, coord, ratio);
  int hLevel = 0;
  string fname = "./index_data/" + mname + "-" + to_string(ratio) + "-" + to_string(hLevel) + ".cpd";
  PreprocessMapPurely(order_code, listg, coord, fname.c_str(), hLevel);

  hLevel = 1; 
  fname = "./index_data/" + mname + "-" + to_string(ratio) + "-" + to_string(hLevel) + ".cpd";
  PreprocessMapPurely(order_code, listg, coord, fname.c_str(), hLevel);
}

TEST_CASE("GetPath") {
  string grpath, copath, scenpath, index_path, wildcard;
  int hLevel;
  ifstream fin("./test/cases/getpath.in");
  fin >> grpath >> copath >> scenpath >> index_path >> wildcard >> hLevel;
  int order_code = 0;
  vector<xyLoc> coord;
  ListGraph listg = RoadNetwork::Load(grpath, copath, coord);
  vector<RoadNetwork::Scen> scens = RoadNetwork::LoadScenarios(scenpath);

  if (index_path == "null") {
    if (wildcard != "sector")
      PreprocessMapPurely(order_code, listg, coord, index_path.c_str(), hLevel);
    else
      PreprocessMapWithSectorWildCard(order_code, listg, coord, index_path.c_str(), hLevel);
  }
  void* ref;
  if (wildcard != "sector")
    ref = PrepareForSearchPurely(listg, coord, index_path.c_str());
  else
    ref = PrepareForSearchSectorWildCard(listg, coord, index_path.c_str());
  for (const auto& it: scens) {
    long long dist;
    if (wildcard != "sector")
      dist = GetPathCostSRC(ref, it.s, it.t, hLevel, -1); 
    else
      dist = GetPathCostSRCWithSectorWildCard(ref, it.s, it.t, hLevel, -1);
    REQUIRE(dist == it.dist);
  }
}

TEST_CASE("sphere_dist") {
  double lat1 = 41.125895;
  double lon1 = -73.529341;
  double lat2 = 41.125051;
  double lon2 = -73.530583;
  double expect_distance = 140.0;
  double d = Geo::distance_m(lat1, lon1, lat2, lon2);
  printf("d:%f expect:%f \n", d, expect_distance);
  REQUIRE(fabs(expect_distance - d) < 1);
}

TEST_CASE("fractal_sort") {
  int len = 4;
  vector<xyLoc> nodes;
  for (int i=0; i<len; i++)
    for (int j=0; j<len; j++) {
    nodes.push_back({i, j});
  }
  NodeOrdering order = compute_fractal_order(nodes);
  for (int i=0; i<len; i++) {
    for (int j=0; j<len; j++) {
      printf("%3d ", i*len+j);
      if (j == len-1) printf("  >>  ");
    }

    for (int j=0; j<len; j++) {
      printf("%3d ", order.to_new(i*len+j));
      if (j == len-1) printf("\n");
    }
  }
}

TEST_CASE("Sector_IO") {
  string mname = "sample1";
  string grpath = "./test/maps/" + mname + "/" + mname + ".gr";
  string copath = "./test/maps/" + mname + "/" + mname + ".co";
  int order_code = 0;
  vector<xyLoc> coord;
  ListGraph listg = RoadNetwork::Load(grpath, copath, coord);

  string index_path = "./index_data/sample1-1-sector.cpd";
  int hLevel = 1;
  PreprocessMapWithSectorWildCard(order_code, listg, coord, index_path.c_str(), hLevel);
  State* data = static_cast<State*>(PrepareForSearchSectorWildCard(listg, coord, index_path.c_str()));
}

TEST_CASE("build_sectors") {
  int deg = 2;
  vector<Sectors::item> items{
    {1<<0, 0, 0},
    {1<<1, 1, 1},
    {1<<1, 2, 2},
    {1<<0, 3, 3},
    {1<<0, 4, 4}
  };
  Sectors sec;
  sec.init(deg);
  sec.build(items);
  REQUIRE(sec.bounds[0].cnt == 2);
  REQUIRE(fabs(sec.bounds[0].lb - 3) < warthog::EPS);
  REQUIRE(fabs(sec.bounds[0].ub - 4) < warthog::EPS);

  REQUIRE(sec.bounds[1].cnt == 2);
  REQUIRE(fabs(sec.bounds[1].lb - 1) < warthog::EPS);
  REQUIRE(fabs(sec.bounds[1].ub - 2) < warthog::EPS);
}

TEST_CASE("angle_sorting") {
  int d = 1000000;
  vector<xyLoc> pos{
    {10 * d, 1},
    {10 * d, 0},
    {10 * d, 10},
    {1, 10 * d},
    {0, 10 * d},
    {-1, 10 * d},
    {-10 * d, 1},
    {-10 * d, -1}
  };
  auto cmp = [&](xyLoc u, xyLoc v) {
    return Geo::angle_ccw(u) < Geo::angle_ccw(v);
  };
  sort(pos.begin(), pos.end(), cmp);
  REQUIRE(pos[0] == xyLoc{-10 * d , -1});
  REQUIRE(pos[1] == xyLoc{ 10 * d ,  0});
  REQUIRE(pos[2] == xyLoc{ 10 * d ,  1});
  REQUIRE(pos[3] == xyLoc{ 10 * d , 10});
  REQUIRE(pos[4] == xyLoc{ 1 , 10 * d });
  REQUIRE(pos[5] == xyLoc{ 0 , 10 * d });
  REQUIRE(pos[6] == xyLoc{-1 , 10 * d });
  REQUIRE(pos[7] == xyLoc{-10 * d , 1 });
}

TEST_CASE("cpdrow") {
  string grpath, copath, index_path, wildcard;
  int s, hLevel;
  ifstream fin("./test/cases/cpdrow.in");
  fin >> grpath >> copath >> index_path >> wildcard >> s >> hLevel;

  vector<xyLoc> coord;
  ListGraph listg = RoadNetwork::Load(grpath, copath, coord);
  NodeOrdering order = compute_real_dfs_order(listg);
  Mapper mapper(listg, coord);
  mapper.reorder(order);
  Dijkstra dij(mapper.g, mapper);
  State* data;
  if (wildcard == "sector")
    data = (State*)PrepareForSearchSectorWildCard(listg, coord, index_path.c_str());
  else
    data = (State*)PrepareForSearchPurely(listg, coord, index_path.c_str());

  for (int i=0; i<listg.node_count(); i++) {
    REQUIRE(mapper.getPos(i) == data->mapper.getPos(i));
    REQUIRE(mapper(i) == data->mapper(i));
  }

  if (wildcard == "sector") {
    Sectors sectors;
    CPD cpd;
    vector<unsigned short> allowed = dij.run(s, hLevel, sectors);
    cpd.append_row(s, allowed, sectors, mapper);

    auto get_sector_move = [&](const Sectors& sects, xyLoc sl, xyLoc tl,
        const CPD& db, int start, int target) {
      int d = Sectors::find_sector(sects, sl, tl);
      if (d == -1) {
        d = db.get_first_move(start, target);
        if ((1 << d) == warthog::HMASK) {
          d = H::decode(s, target, mapper, hLevel);
        }
      }
      return d;
    };
    xyLoc sloc = mapper(s);
    for (int i=0; i<listg.node_count(); i++) if (i != s) {
      xyLoc tloc = mapper(i);
      int d1 = get_sector_move(sectors, sloc, tloc, cpd, 0, i);
      int d2 = get_sector_move(data->sectors[s], sloc, tloc, data->cpd, s, i);
      REQUIRE(d1 == d2);
    }
  } else {
    CPD cpd;
    vector<unsigned short> allowed = dij.run(s, hLevel);
    cpd.append_row(s, allowed);

    for (int i=0; i<listg.node_count(); i++) if (i != s) {
      int d1 = cpd.get_first_move(0, i);
      int d2 = data->cpd.get_first_move(s, i);
      REQUIRE(d1 == d2);
    } 
  }
}

TEST_CASE("row") {
  string grpath, copath;
  int s, hLevel;
  ifstream fin("./test/cases/row.in");
  fin >> grpath >> copath >> s >> hLevel;

  vector<xyLoc> coord;
  ListGraph listg = RoadNetwork::Load(grpath, copath, coord);
  Mapper mapper(listg, coord);
  NodeOrdering order = compute_real_dfs_order(listg);
  mapper.reorder(order);
  Dijkstra dij(mapper.g, mapper); 
  xyLoc sloc = mapper(s);
  vector<unsigned short> allowed0 = dij.run(s, hLevel);

  SECTION("sector") {

    Sectors sectors;
    vector<unsigned short> allowed1 = dij.run(s, hLevel, sectors, false);

    for (int i=0; i<listg.node_count(); i++) {
      REQUIRE(allowed0[i] == allowed1[i]);
    }
    CPD cpd;
    cpd.append_row(s, allowed1, sectors, mapper);

    for (int i=0;i <listg.node_count(); i++) if (i != s) {
      xyLoc tloc = mapper(i);
      int d = Sectors::find_sector(sectors, sloc, tloc);
      if (d == -1) {
        d = cpd.get_first_move(0, i);
        if ((1 << d) == warthog::HMASK) {
          d = H::decode(s, i, mapper, hLevel);
        }
      }
      REQUIRE((allowed1[i] & (1 << d)));
    }
  }

  SECTION("ext-sector") {
    Sectors sectors_ext;
    vector<unsigned short> allowed2 = dij.run(s, hLevel, sectors_ext, true);

    CPD cpd_ext;
    cpd_ext.append_row(s, allowed2, sectors_ext, mapper);

    for (int i=0; i<listg.node_count(); i++) {
      REQUIRE((allowed0[i] & 0xFF) == (allowed2[i] & 0xFF));
    }

    for (int i=0; i<listg.node_count(); i++) if (i != s) {
      xyLoc tloc = mapper(i);
      long double rad = Geo::angle_ccw({tloc.x - sloc.x, tloc.y - sloc.y});
      int b = Sectors::find_closest_sector(sectors_ext, rad);
      int d = -1;
      if (!sectors_ext.is_in_sector(b, rad)) {
        int m = cpd_ext.get_first_move(0, i);
        if ((1<<m) == warthog::HMASK) {
          d = H::decode(s, i, mapper, hLevel);
        }
        else if ((1<<m) == warthog::SHMASK) {
          d = b;
        }
        else if ((1<<m) == warthog::SSMASK) {
          d = (b + 1) % sectors_ext.bounds.size();
        }
        else {
          d = m;
        }
      }
      else d = b;
      CHECK((allowed2[i] & (1 << d)));
    }
  }
}


void build_succ(vector<int>& succ, const vector<long long>& dist, const AdjGraph& g) {
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
  long long d = dij.get_dist()[t]; 
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

TEST_CASE("ll-dij") {
  vector<tuple<string, string, vector<int>>> argvs = 
  {
    {"./maps/NY/NY.gr", "./maps/NY/NY.co", {2, 3}},
    //{"./maps/NY/NY.gr", "./maps/NY/NY.co", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}},
    //{"./maps/COL/COL.gr", "./maps/COL/COL.co", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}},
    {"./maps/NY/NY.gr", "./maps/NY/NY.co", {124116,77419}},
    {"./maps/NY/NY.gr", "./maps/NY/NY.co", {224518,107452}},
    {"./maps/COL/COL.gr", "./maps/COL/COL.co", {1, 254}},
  };
  for (auto& argv: argvs) {
    string& gr = get<0>(argv);
    vector<int> nodes = get<2>(argv);
    vector<int> succ;
    vector<xyLoc> coord;


    ListGraph listg = RoadNetwork::Load(gr);
    coord.resize(listg.node_count());
    succ.resize(listg.node_count());
    Mapper mapper(listg, coord);
    AdjGraph g(listg);
    Dijkstra dij(g, mapper);
    LifeLongDijkstra lldij(g, mapper);

    if (nodes.size() == 2) {
      get_path(nodes[0], nodes[1], dij, g, nodes);
    }

    double lldij_cost = 0, dij_cost = 0;
    dij.run(nodes[0], 0);

    for (int i=1; i<(int)nodes.size(); i++) {
      build_succ(succ, dij.get_dist(), listg);
      lldij.init(dij.get_dist(), succ);
      auto stime = std::chrono::steady_clock::now();
      lldij.run(nodes[i], dij.get_dist(), succ);
      auto etime = std::chrono::steady_clock::now();
      lldij_cost += std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();

      stime = std::chrono::steady_clock::now();
      dij.run(nodes[i], 0);
      etime = std::chrono::steady_clock::now();
      dij_cost += std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();


      vector<long long> dist = dij.get_dist();
      vector<long long> lldist = lldij.get_dist();
      for (int j=0; j<listg.node_count(); j++) {
        REQUIRE(dist[j] == lldist[j]);
      }
      if (i >= 10) break;
    }
    cerr << "dij tcost: " << dij_cost << ", lldij tcost: " << lldij_cost 
         << ", speed up: " << (dij_cost - lldij_cost) / dij_cost << endl;
    cerr << "==============" << endl;
  }
}


int main(int argv, char* args[]) {
	cout << "Loading data..." << endl;
	cout << "Running test cases..." << endl;
	Catch::Session session;
	int res = session.run(argv, args);
  return res;
}
