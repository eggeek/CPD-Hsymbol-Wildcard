#include "cpd_rect.h"
using namespace std;


inline vector<int> CPD_RECT::compress(int source_node,
    const vector<unsigned short>& fmoves,
    const RectInfo& rect,
    const Mapper& mapper,
    const vector<int>& row_ordering,
    const int side) {

  auto get_allowed_local = [&](int x){
    return this->get_allowed(x, source_node, side, rect, row_ordering, fmoves, mapper);
  };
  vector<int> compressed;
  int node_begin = 0;
  
  unsigned short allowed_up_to_now = get_allowed_local(0);
  for(int i=1; i<(int)fmoves.size(); ++i){
    int allowed_next = allowed_up_to_now & get_allowed_local(i);
    if(allowed_next == 0){
      compressed.push_back((node_begin << 4) | find_first_allowed_out_arc(allowed_up_to_now));
      node_begin = i;
      allowed_up_to_now = get_allowed_local(i);
    }else
      allowed_up_to_now = allowed_next;
  }
  compressed.push_back((node_begin << 4) | find_first_allowed_out_arc(allowed_up_to_now));
  return compressed;
}

vector<RectInfo> CPD_RECT::append_row(int s, const vector<unsigned short>& allowed, const Mapper& mapper,
    const vector<RectInfo>& rects, const vector<int>& row_ordering,
    const int side) {
  vector<RectInfo> used;
  vector<unsigned short> fmoves = vector<unsigned short>(allowed.begin(), allowed.end());
  vector<int> cur = compress(s, allowed, {0, 0, 0, 0}, mapper, row_ordering, side);

  auto fill_rect = [&](const RectInfo& rect) {
    int x = rect.x(mapper.width());
    int y = rect.y(mapper.width());
    for (int i=x-rect.L+1; i<=x; i++)
    for (int j=y-rect.U+1; j<=y; j++) {
      int id = mapper(xyLoc{(int16_t)i, (int16_t)j});
      if (id != -1) fmoves[id] = warthog::ALLMOVE;
    }
  };

  int width = mapper.width();
  int limit = 5;
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

    vector<int> tmp = compress(s, allowed, it, mapper, row_ordering, side);
    if (tmp.size() * sizeof(int) + sizeof(RectInfo) < cur.size() * sizeof(int)) {
      cur = tmp;
      used.push_back(it);
      if ((int)used.size() >= limit) break;
    }
    fill_rect(it);
  }
  entry.insert(entry.end(), cur.begin(), cur.end());
  begin.push_back(entry.size());
  return used;
}
