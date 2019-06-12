#include "cpd.h"
#include "constants.h"

#include <fstream>
#include <stdexcept>
#include <cassert>

// compile with -O3 -DNDEBUG

static unsigned char find_first_allowed_out_arc(unsigned short allowed){
    assert(allowed != 0);
    return warthog::m2i.at(warthog::lowb(allowed));
}

inline vector<int> CPD::compress(int source_node,
    const vector<unsigned short>& fmoves,
    const RectInfo& rect,
    const Mapper& mapper,
    const vector<int>& row_ordering) {

  auto in_rect = [&](int id) {
    const xyLoc& loc = mapper(id);
    int y = rect.y(mapper.width());
    int x = rect.x(mapper.width());
    return (loc.y - y < rect.U && loc.x - x < rect.L);
  };

  auto get_allowed = [&](int x){
    if(x == source_node)
      return warthog::ALLMOVE;
    else if(row_ordering[source_node] < row_ordering[x])
      return warthog::ALLMOVE;
    else if (in_rect(x))
      return warthog::ALLMOVE;
    else if(fmoves[x] == 0)
      return warthog::NOMOVE;
    else
      return fmoves[x];
  };
  vector<int> compressed;
  int node_begin = 0;
  
  unsigned short allowed_up_to_now = get_allowed(0);
  for(int i=1; i<(int)fmoves.size(); ++i){
    int allowed_next = allowed_up_to_now & get_allowed(i);
    if(allowed_next == 0){
      compressed.push_back((node_begin << 4) | find_first_allowed_out_arc(allowed_up_to_now));
      node_begin = i;
      allowed_up_to_now = get_allowed(i);
    }else
      allowed_up_to_now = allowed_next;
  }
  compressed.push_back((node_begin << 4) | find_first_allowed_out_arc(allowed_up_to_now));
  return compressed;
}

vector<RectInfo> CPD::append_row(int s, const vector<unsigned short>& allowed, const Mapper& mapper,
    const vector<RectInfo>& rects, const vector<int>& row_ordering) {
  vector<RectInfo> used;
  vector<unsigned short> fmoves = vector<unsigned short>(allowed.begin(), allowed.end());
  vector<int> cur = compress(s, allowed, {0, 0, 0, 0}, mapper, row_ordering);

  auto fill_rect = [&](const RectInfo& rect) {
    int x = rect.x(mapper.width());
    int y = rect.y(mapper.width());
    for (int i=x-rect.L+1; i<=x; i++)
    for (int j=y-rect.U+1; j<=y; j++) {
      int id = mapper(xyLoc{(int16_t)i, (int16_t)j});
      if (id != -1) fmoves[id] = warthog::ALLMOVE;
    }
  };

  int  width = mapper.width();
  for (const auto& it: rects) {
    if (!used.empty()) {
      const RectInfo& pre = used.back();
      int xl0 = it.x(width) - it.L+1;
      int xr0 = it.x(width);
      int yl0 = it.y(width) - it.U+1;
      int yr0 = it.y(width);

      int xl1 = pre.x(width) - pre.L+1;
      int xr1 = pre.x(width);
      int yl1 = pre.y(width) - pre.U+1;
      int yr1 = pre.y(width);

      if (!(xr1 < xl0 || xl1 > xr0 || yr1 < yl0 || yl1 > yr0))
        continue;
    }

    vector<int> tmp = compress(s, allowed, it, mapper, row_ordering);
    if (tmp.size() * sizeof(int) + sizeof(RectInfo) < cur.size() * sizeof(int)) {
      cur = tmp;
      fill_rect(it);
      used.push_back(it);
    }
  }
  entry.insert(entry.end(), cur.begin(), cur.end());
  begin.push_back(entry.size());
  return used;
}


void CPD::append_row(int source_node, const std::vector<unsigned short>&allowed_first_move, const std::vector<int>&row_ordering,
        Mapper mapper, const int side){
  auto is_in_square = [&](int x)
  {
      xyLoc loc_source = mapper.operator()(source_node);
      xyLoc loc_x = mapper.operator()(x);
      int dx = abs(loc_source.x - loc_x.x);
      int dy = abs(loc_source.y- loc_x.y);
      if((dx <= (side-1)/2)&&(dy<= (side-1)/2))
      {
        return true;
      }
      return false;
  };

  auto get_allowed = [&](int x){
    if(x == source_node)
      return warthog::ALLMOVE;
    else if(is_in_square(x))
      return warthog::ALLMOVE;
    else if(row_ordering[source_node] < row_ordering[x])
      return warthog::ALLMOVE;
    else if(allowed_first_move[x] == 0)
      return warthog::NOMOVE;
    else
      return allowed_first_move[x];
  };

  int node_begin = 0;
  unsigned short allowed_up_to_now = get_allowed(0);

  for(int i=1; i<(int)allowed_first_move.size(); ++i){
    int allowed_next = allowed_up_to_now & get_allowed(i);
    if(allowed_next == 0){
      entry.push_back((node_begin << 4) | find_first_allowed_out_arc(allowed_up_to_now));
      node_begin = i;
      allowed_up_to_now = get_allowed(i);
    }else
      allowed_up_to_now = allowed_next;
  }
  entry.push_back((node_begin << 4) | find_first_allowed_out_arc(allowed_up_to_now));

  begin.push_back(entry.size());
}

void CPD::append_rows(const CPD&other){
  int offset = begin.back();
  for(auto x:make_range(other.begin.begin()+1, other.begin.end()))
    begin.push_back(x + offset);
  std::copy(other.entry.begin(), other.entry.end(), back_inserter(entry));
}

