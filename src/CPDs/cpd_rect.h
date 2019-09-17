#pragma once
#include "cpd_base.h"
#include "rect_wildcard.h"
using namespace std;


class CPD_RECT: public CPDBASE {
public:
  vector<RectInfo> append_row(int s, const vector<unsigned short>& allowed, const Mapper& mapper, 
      const vector<RectInfo>& rects, const vector<int>& row_ordering,
      const int side);
  vector<int> compress(int s, const vector<unsigned short>& allowed,
      const RectInfo& rect, const Mapper& mapper, const vector<int>& row_ordering,
      const int side);

  bool in_rect(int id, const RectInfo& rect, const Mapper& mapper) {
    const xyLoc& loc = mapper(id);
    int y = rect.y(mapper.width());
    int x = rect.x(mapper.width());
    return (loc.x >= x-rect.L+1 && loc.x <= x && loc.y >= y-rect.U+1 && loc.y <= y);
  }

  int get_allowed(
    int x, int s, int side,
    const RectInfo& rect, const vector<int>& row_ordering,
    const vector<unsigned short>& fmoves,
    const Mapper& mapper) {
    if(x == s)
      return warthog::ALLMOVE;
    else if(row_ordering[s] < row_ordering[x])
      return warthog::ALLMOVE;
    else if (is_in_square(x, side, s, mapper))
      return warthog::ALLMOVE;
    else if (in_rect(x, rect, mapper))
      return warthog::ALLMOVE;
    else if(fmoves[x] == 0)
      return warthog::NOMOVE;
    else
      return fmoves[x];
  }
};
