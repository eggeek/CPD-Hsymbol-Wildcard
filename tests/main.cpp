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

namespace TEST_MAIN {

string mpath;
int height, width;
vector<bool> mapData;

TEST_CASE("canonical_succ") {
  uint32_t tiles, succ;
  vector<string> map;
  SECTION("testing") {
    printf("Run test cases:\n");
    ifstream file("tests/input/default/canonical_suc_0.in");
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

  SECTION("visualization") {
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
  }
}

};


int main(int argv, char* args[]) {
	cout << "Running test cases..." << endl;
	Catch::Session session;
	int res = session.run(argv, args);
  return res;
}

