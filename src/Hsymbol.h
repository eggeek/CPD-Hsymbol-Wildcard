#pragma once
#include "mapper.h"
using namespace std;
namespace Hsymbol {

inline int get_heuristic_move(int s, int t, const Mapper& mapper) {
  xyLoc sloc = mapper(s);
  xyLoc tloc = mapper(t);
  int dx = tloc.x - sloc.x;
  int dy = tloc.y - sloc.y;
  if (dx < 0) dx = -1;
  else dx = dx?1: 0;
  if (dy < 0) dy = -1;
  else dy = dy?1: 0;
  int res = warthog::v2i[dx+1][dy+1];
  return res;
}

inline void encode(const int source, vector<unsigned short>& allowed, const Mapper& mapper) {
  for (int v=0; v<(int)allowed.size(); v++) if (v != source) {
    int hmove = get_heuristic_move(source, v, mapper);
    if (allowed[v] & (1 << hmove))
      allowed[v] |= warthog::HMASK;
  }
}

inline int decode(int s, int t, const Mapper& mapper) {
  return get_heuristic_move(s, t, mapper);
}

};
