#pragma once
#include "mapper.h"
using namespace std;
namespace Hsymbol {

static void avoid_corner_cutting(const xyLoc& sloc, const Mapper& mapper, int& dx, int& dy) {
  xyLoc p1 = xyLoc{sloc.x, static_cast<std::int16_t>(sloc.y+dy)};
  xyLoc p2 = xyLoc{static_cast<std::int16_t>(sloc.x+dx), sloc.y};
  if (mapper(p1) == -1) dy = 0;
  if (mapper(p2) == -1) dx = 0;
}

static int get_heuristic_move(int s, int t, const Mapper& mapper) {
  xyLoc sloc = mapper(s);
  xyLoc tloc = mapper(t);
  int dx = tloc.x - sloc.x;
  int dy = tloc.y - sloc.y;
  if (dx < 0) dx = -1;
  else dx = dx?1: 0;
  if (dy < 0) dy = -1;
  else dy = dy?1: 0;

  // adjust (dx, dy) to avoid corner cutting
  avoid_corner_cutting(sloc, mapper, dx, dy);
  int res = warthog::v2i[dx+1][dy+1];
  return res;
}

static void encode(const int source, vector<unsigned short>& allowed, const Mapper& mapper) {
  for (int v=0; v<(int)allowed.size(); v++) if (v != source) {
    int hmove = get_heuristic_move(source, v, mapper);
    if (allowed[v] & (1 << hmove))
      allowed[v] |= warthog::HMASK;
  }
}

static int decode(int s, int t, const Mapper& mapper) {
  return get_heuristic_move(s, t, mapper);
}

};
