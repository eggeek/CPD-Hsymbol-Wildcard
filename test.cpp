#define CATCH_CONFIG_RUNNER
#include <vector>
#include "catch.hpp"
#include "Entry.h"
#include "cpd.h"
#include "mapper.h"
#include "dijkstra.h"
#include "visualizer.h"
using namespace std;

string mpath;
int height, width;
vector<bool> mapData;

TEST_CASE("ObsFree") {
  ifstream file("test/ObsFree.in");
  file >> mpath;
  LoadMap(mpath.c_str(), mapData, width, height);
  Mapper mapper(mapData, width, height);
  CPD cpd;
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
  CPD cpd;
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

int main(int argv, char* args[]) {
	cout << "Loading data..." << endl;
	cout << "Running test cases..." << endl;
	Catch::Session session;
	int res = session.run(argv, args);
  return res;
}
