#pragma once
#include <utility>
using namespace std;

struct xyLoc {
  long long x, y;

  bool operator == (const xyLoc& o) const {
    return this->x == o.x && this->y == o.y;
  }

  bool operator <(const xyLoc& o) const {
    return make_pair(this->x, this->y) < make_pair(o.x, o.y);
  }
};

//bool operator<(xyLoc a, xyLoc b) {
//  return make_pair(a.x, a.y) < make_pair(b.x, b.y);
//}


