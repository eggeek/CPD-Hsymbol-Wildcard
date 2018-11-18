#pragma once
#include <stdint.h>
#include "mapper.h"
using namespace std;
const uint32_t MASK = ((uint32_t)1<<31) - 1;
#define signbit(x) (((x) >> 31) - (-(x) >> 31))
#define iabs(x) ( (((x) >> 31) ^ (x)) - (((x) >> 31)) )
#define iabs2(x) (((x) & MASK) - ((x) >> 31))
namespace Hsymbol {
/*            y+ 
 *
 *            S
 *
 *            |    
 *         3  |  0
 * x- W ------|-------- E x+
 *         2  |  1
 *            |
 *
 *            N
 *
 *            y-
 * get quadrant: quadrant[dx+1][dy+1]
 */
const static int quadrant[3][3] = {
      {2, 3, 3},
      {2, -1, 0},
      {1, 1, 0}
};

const static int qDirection[3][4]  = {
// q0, q1, q2, q3
  {2,  2,  3,  3},// go x
  {1,  0,  0,  1},// go y
  {6,  4,  5,  7},// go diag
};

static inline void avoid_corner_cutting(const xyLoc& sloc, const Mapper& mapper, int& dx, int& dy) {
  xyLoc p1 = xyLoc{sloc.x, static_cast<std::int16_t>(sloc.y+dy)};
  xyLoc p2 = xyLoc{static_cast<std::int16_t>(sloc.x+dx), sloc.y};
  if (mapper(p1) == -1) dy = 0;
  if (mapper(p2) == -1) dx = 0;
}

static inline int closestDirection(int dx, int dy) {
  int quad = quadrant[signbit(dx) + 1][signbit(dy) + 1];
  bool gox = iabs(dx) > (iabs(dy) << 1);
  bool goy = iabs(dy) > (iabs(dx) << 1);
  bool diag = (!gox) && (!goy);
  int direct;
  switch ((diag << 2) | (goy << 1) | gox) {
    case 1: direct = qDirection[0][quad];
               break;
    case 1<<1: direct = qDirection[1][quad];
               break;
    case 1<<2 : direct = qDirection[2][quad];
                break;
    default: direct = - 1;
             break;
  };
  return direct;
}

static inline int get_coord_part(int x, int y) {
  bool p0 = (iabs(x) << 1) <= iabs(y);
  bool p1 = ((iabs(x) << 1) > iabs(y)) & (iabs(x) <= iabs(y));
  bool p2 = (iabs(x) > iabs(y)) & ((iabs(y) << 1) >= iabs(x));
  bool p3 = (iabs(y) << 1) < iabs(x);
  //bool p0 = iabs(x) == 0;
  //bool p1 = (iabs(x) > 0) & (iabs(x) <= iabs(y));
  //bool p2 = (iabs(y) > 0) & (iabs(x) > iabs(y));
  //bool p3 = iabs(y) == 0;
  int res = -1;
  switch ( p0 | (p1 << 1) | (p2 << 2) | (p3 << 3)) {
    case 1<<0: res = 0;
               break;
    case 1<<1: res = 1;
               break;
    case 1<<2: res = 2;
               break;
    case 1<<3: res = 3;
               break;
    default: res = -1;
             break;
  };
  return res;
}

static inline int get_heuristic_move2(int s, int t, const Mapper& mapper) {
    double min_cost = warthog::INF;
    int hmove = warthog::INVALID_MOVE;
    xyLoc sloc = mapper(s);
    xyLoc tloc = mapper(t);

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
    return hmove;
}

static inline int get_heuristic_move1(int s, int t, const Mapper& mapper) {
  xyLoc sloc = mapper(s);
  xyLoc tloc = mapper(t);
  int dx = signbit(tloc.x - sloc.x);
  int dy = signbit(tloc.y - sloc.y);
  avoid_corner_cutting(sloc, mapper, dx, dy);
  return warthog::v2i[dx+1][dy+1];
}

static inline int get_heuristic_move3(int s, int t, const Mapper& mapper) {
  int move = warthog::INVALID_MOVE;
  xyLoc sloc = mapper(s);
  xyLoc tloc = mapper(t);
  int dx = tloc.x - sloc.x;
  int dy = tloc.y - sloc.y;
  int quad = quadrant[signbit(dx) + 1][signbit(dy) + 1];
  int part = get_coord_part(dx, dy);
  //int no_diagonal= (dx == 0) | (dy == 0);
  int no_diagnonal = 1;
  move = mapper.get_valid_move(s, quad, part, no_diagnonal);
  return move;
}

static inline int get_heuristic_move(int s, int t, const Mapper& mapper, int hLevel) {
  int move = warthog::INVALID_MOVE;
  switch (hLevel) {
    case 1: move = get_heuristic_move1(s, t, mapper);
            break;
    case 2: move = get_heuristic_move2(s, t, mapper);
            break;
    case 3: move = get_heuristic_move3(s, t, mapper);
            break;
    default:
            cerr << "undefined hlevel" << endl;
            break;
  }
  return move;
}

static inline void encode(const int source, vector<unsigned short>& allowed, const Mapper& mapper,
    int hLevel) {
  for (int v=0; v<(int)allowed.size(); v++) if (v != source) {
    int hmove = get_heuristic_move(source, v, mapper, hLevel);
    if (allowed[v] & (1 << hmove))
      allowed[v] |= warthog::HMASK;
  }
}

static inline int decode(int s, int t, const Mapper& mapper, int hLevel) {
  return get_heuristic_move(s, t, mapper, hLevel);
}

static inline int decode(int s, int t, const Mapper& mapper, int (*func) (int, int, const Mapper&) ) {
  return func(s, t, mapper);
}

};
