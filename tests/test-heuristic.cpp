#include <vector>
#include "catch.hpp"
#include "Entry.h"
#include "cpd.h"
#include "mapper.h"
#include "dijkstra.h"
#include "visualizer.h"
#include "jps.h"
using namespace std;

namespace TEST_HEURISTIC {
const string default_testcase_path = "./tests/input/test-heuristic/";
const string default_map_path = "./tests/maps/";
string mpath;
int height, width;
vector<bool> mapData;

bool LoadScen(ifstream& in, xyLoc& s, xyLoc& t) {
  if (!(in >> height >> width)) return false;
  mapData.resize(height * width);
  for (int y=0; y<height; y++) {
    string row;
    in >> row;
    for (int x=0; x<width; x++) {
      mapData[y*width + x] = (row[x] != '#');
      if (row[x] == 's') {
        s.x = x;
        s.y = y;
      }
      if (row[x] == 't') {
        t.x = x;
        t.y = y;
      }
    }
  }
  return true;
}

TEST_CASE("Hmove", "[hsymbol]") {
  ifstream file(default_testcase_path + "hmoves_0.in");
  xyLoc s, t;
  int expected;
  int hLevel = 1;
  while (LoadScen(file, s, t)) {
    file >> expected;
    cout << height << " " << width << endl;
    Mapper mapper(mapData, width, height);
    int hmove = Hsymbol::get_heuristic_move(mapper(s), mapper(t), mapper, hLevel);
    printf("expected (%d):", expected); Mapper::set2direct(expected);
    printf("actual (%d):", (1<<hmove)); Mapper::set2direct((1<<hmove));
    REQUIRE((1 << hmove) == expected);
  } 
}

TEST_CASE("Hmove2", "[hsymbol]") {
  ifstream file(default_testcase_path + "hmoves2_0.in");
  xyLoc s, t;
  int expected;
  int hLevel = 2;
  while (LoadScen(file, s, t)) {
    file >> expected;
    cout << height << " " << width << endl;
    Mapper mapper(mapData, width, height);
    int hmove = Hsymbol::get_heuristic_move2(mapper(s), mapper(t), mapper);
    printf("expected (%d): ", expected); Mapper::set2direct(expected);
    printf("actual (%d): ", (1<< hmove)); Mapper::set2direct(1<<hmove);
    REQUIRE((1<< hmove) == expected);
  }
}

TEST_CASE("badcase", "[.findworst]") {
  mpath = "./tests/maps/arena.map";
  LoadMap(mpath.c_str(), mapData, width, height);
  Mapper mapper(mapData, width, height);
  AdjGraph g(extract_graph(mapper));
  Dijkstra dij(g, mapper);

  vector<unsigned short> res;
  int worst = -1, v = 0;
  for (int i=0; i<mapper.node_count(); i++) {
    int cnth1 = 0;
    for (auto j: dij.run(i, 2)) if (j & warthog::HMASK) cnth1++;

    int cnth2 = 0;
    for (auto j: dij.run(i, 3)) if (j & warthog::HMASK) cnth2++;

    if (cnth2 < cnth1) {
      if (cnth1 - cnth2 > v) {
        v = cnth1 - cnth2;
        worst = i;
      }
    }
  }
  printf("worst id:%d, diff: %d\n", worst, v);
}


TEST_CASE("hextension", "[hsymbol]") {
  mpath = default_map_path + "3obs.map";
  LoadMap(mpath.c_str(), mapData, width, height);
  Mapper mapper(mapData, width, height);
  AdjGraph g(extract_graph(mapper));

  int s = 28, t = 0, hLevel = 2;
  int move = H::get_heuristic_move(s, t, mapper, hLevel);
  Mapper::set2direct(1 << move);

  Dijkstra dij(g, mapper);
  dij.run(s, hLevel);
  Mapper::set2direct(dij.get_directions(t));
}

}
