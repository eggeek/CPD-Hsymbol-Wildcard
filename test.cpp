#define CATCH_CONFIG_RUNNER
#include <vector>
#include "catch.hpp"
#include "Entry.h"
#include "cpd.h"
#include "mapper.h"
#include "dijkstra.h"
#include "visualizer.h"
#include "jps.h"
using namespace std;

string mpath;
int height, width;
vector<bool> mapData;

TEST_CASE("ObsFree") {
  ifstream file("test/ObsFree.in");
  file >> mpath;
  LoadMap(mpath.c_str(), mapData, width, height);
  Mapper mapper(mapData, width, height);
  AdjGraph g(extract_graph(mapper));
  Dijkstra dij(g, mapper);
  int s = 0;
  int cnth = 0;
  auto res = dij.run(s);
  vector<string> vis = Visualizer(mapData, mapper).to_strings(s, res);
  for (string i: vis) cout << i << endl;
  for (auto i: res) if (i & warthog::HMASK) cnth++;
  REQUIRE(cnth == mapper.node_count() - 1);
}

TEST_CASE("OneObs") {
  ifstream file("test/OneObs.in");
  file >> mpath;
  LoadMap(mpath.c_str(), mapData, width, height);
  Mapper mapper(mapData, width, height);
  AdjGraph g(extract_graph(mapper));
  Dijkstra dij(g, mapper);
  int s = 2;
  int cnth = 0;
  auto res = dij.run(s);
  for (auto i: res) if (i & warthog::HMASK) cnth++;
  vector<string> vis = Visualizer(mapData, mapper).to_strings(s, res);
  for (string i: vis) cout << i << endl;
  REQUIRE(cnth == 13);
}

TEST_CASE("3Obs") {
  mpath = "test/maps/3obs.map";
  LoadMap(mpath.c_str(), mapData, width, height);
  Mapper mapper(mapData, width, height);
  AdjGraph g(extract_graph(mapper));
  Dijkstra dij(g, mapper);
  int s = 21;
  int cnth = 0;
  auto res = dij.run(s);
  for (auto i: res) if (i & warthog::HMASK) cnth++;
  vector<string> vis = Visualizer(mapData, mapper).to_strings(s, res);
  for (string i: vis) cout << i << endl;
  REQUIRE(cnth == 13);
}

TEST_CASE("canonical_succ") {
  uint32_t tiles, succ;
  vector<string> map;
  tiles = 65792;
  printf("tiles: %d\n", tiles);
  printf("map:\n");
  for (string i: Mapper::tiles2str(tiles)) cout << i << endl;

  tiles = 263168;
  printf("tiles: %d\n", tiles);
  printf("map:\n");
  for (string i: Mapper::tiles2str(tiles)) cout << i << endl;

  tiles = 257;
  printf("tiles: %d\n", tiles);
  printf("map:\n");
  for (string i: Mapper::tiles2str(tiles)) cout << i << endl;

  tiles = 1028;
  printf("tiles: %d\n", tiles);
  printf("map:\n");
  for (string i: Mapper::tiles2str(tiles)) cout << i << endl;

  tiles = 3;
  printf("tiles: %d\n", tiles);
  printf("map:\n");
  for (string i: Mapper::tiles2str(tiles)) cout << i << endl;

  tiles = 196608;
  printf("tiles: %d\n", tiles);
  printf("map:\n");
  for (string i: Mapper::tiles2str(tiles)) cout << i << endl;

  tiles = 6;
  printf("tiles: %d\n", tiles);
  printf("map:\n");
  for (string i: Mapper::tiles2str(tiles)) cout << i << endl;

  tiles = 393216;
  printf("tiles: %d\n", tiles);
  printf("map:\n");
  for (string i: Mapper::tiles2str(tiles)) cout << i << endl;

  tiles = 2;
  printf("tiles: %d\n", tiles);
  printf("map:\n");
  for (string i: Mapper::tiles2str(tiles)) cout << i << endl;

  tiles = 131072;
  printf("tiles: %d\n", tiles);
  printf("map:\n");
  for (string i: Mapper::tiles2str(tiles)) cout << i << endl;

  tiles = 1024;
  printf("tiles: %d\n", tiles);
  printf("map:\n");
  for (string i: Mapper::tiles2str(tiles)) cout << i << endl;

  tiles = 256;
  printf("tiles: %d\n", tiles);
  printf("map:\n");
  for (string i: Mapper::tiles2str(tiles)) cout << i << endl;

  tiles = 259;
  printf("tiles: %d\n", tiles);
  printf("map:\n");
  for (string i: Mapper::tiles2str(tiles)) cout << i << endl;

  tiles = 1030;
  printf("tiles: %d\n", tiles);
  printf("map:\n");
  for (string i: Mapper::tiles2str(tiles)) cout << i << endl;

  tiles = 196864;
  printf("tiles: %d\n", tiles);
  printf("map:\n");
  for (string i: Mapper::tiles2str(tiles)) cout << i << endl;

  tiles = 394240;
  printf("tiles: %d\n", tiles);
  printf("map:\n");
  for (string i: Mapper::tiles2str(tiles)) cout << i << endl;

  printf("Run test cases:\n");
  ifstream file("test/canonical_suc.in");
  int cas, direct, ans;
  warthog::jps::direction d;
  file >> cas;
  for (int i=0; i<cas; i++) {
    printf("-----------------\n");
    map.resize(3);
    for (int j=0; j<3; j++) file >> map[j];
    file >> direct;
    file >> ans;
    tiles = Mapper::str2tiles(map);
    succ = warthog::jps::compute_successors((warthog::jps::direction)direct, tiles);

    printf("map:\n");
    for (string i: map) cout << i << endl;
    printf("tiles: %d\n", tiles);
    printf("direction (%d): ", direct); Mapper::set2direct(direct);
    printf("succ (%d): ", succ); Mapper::set2direct(succ);
    REQUIRE(succ == ans);
  }
}

TEST_CASE("canonical_dijk") {
  mpath = "test/maps/oneobs.map";
  LoadMap(mpath.c_str(), mapData, width, height);
  Mapper mapper(mapData, width, height);
  AdjGraph g(extract_graph(mapper));
  Dijkstra dij(g, mapper);
  int s = 9, t = 10;
  vector<unsigned short> res;
  vector<string> vis;
  res = dij.run(s);
  vis = Visualizer(mapData, mapper).to_strings(s, res);
  for (string i: vis) cout << i << endl;
  printf("direction (%d): ", t); Mapper::set2direct(dij.get_directions(t));
  REQUIRE(dij.get_directions(t) == 3);

  s = 13, t = 2;
  res = dij.run(s);
  vis = Visualizer(mapData, mapper).to_strings(s, res);
  for (string i: vis) cout << i << endl;
  printf("direction (%d): ", t); Mapper::set2direct(dij.get_directions(t));
  REQUIRE(dij.get_directions(t) == 48);
}

int main(int argv, char* args[]) {
	cout << "Loading data..." << endl;
	cout << "Running test cases..." << endl;
	Catch::Session session;
	int res = session.run(argv, args);
  return res;
}
