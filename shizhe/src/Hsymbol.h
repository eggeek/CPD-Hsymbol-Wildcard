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

static double minimize_hvalue(const xyLoc& sloc, const xyLoc& tloc, const Mapper& mapper, int& hmove) {
    double min_cost = warthog::INF;

    for (int d=0; d<8; d++) {
      xyLoc nxt = xyLoc{(int16_t)(sloc.x + warthog::dx[d]), (int16_t)(sloc.y + warthog::dy[d])};
      xyLoc p1 = xyLoc{sloc.x, (int16_t)(sloc.y + warthog::dy[d])};
      xyLoc p2 = xyLoc{(int16_t)(sloc.x + warthog::dx[d]), sloc.y};
      // avoid corner cutting
      if (mapper(nxt) == -1 || mapper(p1) == -1 || mapper(p2) == -1) 
        continue;
      //int common = min(abs(tloc.x - nxt.x), abs(tloc.y - nxt.y));
      //double cost = common * warthog::DBL_ROOT_TWO + abs(tloc.x - nxt.x) + abs(tloc.y - nxt.y) - 2.0 * common + warthog::doublew[d];
      double cost = sqrt((tloc.x - nxt.x) * (tloc.x - nxt.x) + (tloc.y - nxt.y) * (tloc.y - nxt.y)) + warthog::doublew[d];
      if (cost < min_cost) {
        min_cost = cost;
        hmove = d;
      }
    }
    return min_cost;
}

static int get_heuristic_move(int s, int t, const Mapper& mapper, int hLevel) {
  xyLoc sloc = mapper(s);
  xyLoc tloc = mapper(t);
  int dx = tloc.x - sloc.x;
  int dy = tloc.y - sloc.y;
  dx = dx? dx / abs(dx): 0;
  dy = dy? dy / abs(dy): 0;

  if (hLevel == 1) { // the default hmove encoding
    // adjust (dx, dy) to avoid corner cutting
    avoid_corner_cutting(sloc, mapper, dx, dy);
    return warthog::v2i[dx+1][dy+1];
  }
  else if (hLevel == 2) {
    int move = warthog::INVALID_MOVE;
    minimize_hvalue(sloc, tloc, mapper, move);
    return move;
  }
  throw "value of hLevel is undefined";
  return -1;
}

static void encode(const int source, vector<unsigned short>& allowed, const Mapper& mapper,
    int hLevel) {
  for (int v=0; v<(int)allowed.size(); v++) if (v != source) {
    int hmove = get_heuristic_move(source, v, mapper, hLevel);
    if (allowed[v] & (1 << hmove))
      allowed[v] |= warthog::HMASK;
  }
}

static int decode(int s, int t, const Mapper& mapper, int hLevel) {
  return get_heuristic_move(s, t, mapper, hLevel);
}

};
