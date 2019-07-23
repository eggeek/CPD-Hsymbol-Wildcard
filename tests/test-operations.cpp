#include <vector>
#include "catch.hpp"
#include "preprocessing.h"
#include "mapper.h"
#include "dijkstra.h"
#include "visualizer.h"
#include "jps.h"

using namespace std;

TEST_CASE("operations") {
  SECTION("bitop") {
    REQUIRE(signbit(10) == 1);
    REQUIRE(signbit(0) == 0);
    REQUIRE(signbit(-1) == -1);
    REQUIRE(signbit(-0) == 0);
    REQUIRE(signbit(1<<30) == 1);
    REQUIRE(signbit(-(1<<30)) == -1);

    REQUIRE(iabs(10) == 10);
    REQUIRE(iabs(-10) == 10);
    REQUIRE(iabs(0) == 0);
    REQUIRE(iabs((1<<30)) == (1<<30));
    REQUIRE(iabs(-(1<<30)) == (1<<30));
  }

  SECTION("quadrant") {
    auto func = [&](int x, int y) {
      x = signbit(x);
      y = signbit(y);
      return H::quadrant[x+1][y+1];
    };
    REQUIRE(func(0, 1) == 0);
    REQUIRE(func(1, 1) == 0);
    REQUIRE(func(1, 0) == 1);
    REQUIRE(func(1, -1) == 1);
    REQUIRE(func(0, -1) == 2);
    REQUIRE(func(-1, -1) == 2);
    REQUIRE(func(-1, 0) == 3);
    REQUIRE(func(-1, 1) == 3);
  }

  SECTION("optDirection") {
    using namespace warthog::jps;
    REQUIRE(1 << H::closestDirection(100, 100) == direction::SOUTHEAST);
    REQUIRE(1 << H::closestDirection(49, 100) == direction::SOUTH);
    REQUIRE(1 << H::closestDirection(1, 100) == direction::SOUTH);
    REQUIRE(1 << H::closestDirection(-49, 100) == direction::SOUTH);
    REQUIRE(1 << H::closestDirection(-100, 100) == direction::SOUTHWEST);
  }

  SECTION("coordpart") {
    REQUIRE(H::get_coord_part(49, 100) == 0);
    REQUIRE(H::get_coord_part(99, 100) == 1);
    REQUIRE(H::get_coord_part(101, 100) == 2);
    REQUIRE(H::get_coord_part(100, 49) == 3);
  }
}
