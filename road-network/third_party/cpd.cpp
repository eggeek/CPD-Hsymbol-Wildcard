#include "cpd.h"
#include "constants.h"
#include "geo.h"
#include <fstream>
#include <stdexcept>
#include <cassert>

// compile with -O3 -DNDEBUG

static unsigned int find_first_allowed_out_arc(unsigned short allowed){
    assert(allowed != 0);
    return warthog::m2i.at(warthog::lowb(allowed));
}

inline vector<int> CPD::compress(int source_node, const vector<unsigned short>& allowed_first_move) {
  auto get_allowed = [&](int x){
    if(x == source_node)
      return warthog::ALLMOVE;
    else if(allowed_first_move[x] == 0)
      return warthog::NOMOVE;
    else
      return allowed_first_move[x];
  };
  vector<int> compressed;
  int node_begin = 0;
  
  unsigned short allowed_up_to_now = get_allowed(0);
  for(int i=1; i<(int)allowed_first_move.size(); ++i){
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

static inline bool is_in_sector(const xyLoc& s, int tid,
  const std::vector<unsigned short>& first_move,
  const Sectors& sectors,
  const Mapper& mapper) {
  xyLoc t = mapper(tid);
  long double rad = Geo::angle_ccw({t.x - s.x, t.y - s.y});
  int mask = first_move[tid];
  while (mask) {
    int move_mask = warthog::lowb(mask);
    int direction = warthog::m2i.at(move_mask);
    if (sectors.is_in_sector(direction, rad))
      return true;
    mask -= move_mask;
  }
  return false;
}

inline vector<int> CPD::compress(
  int source_node,
  const vector<unsigned short>& first_move,
  const Sectors& sectors,
  const Mapper& mapper
) {
  vector<int> compressed;
  xyLoc s = mapper(source_node);

  auto get_allowed = [&](int x){
    if(x == source_node)
      return warthog::ALLMOVE;
    else if (is_in_sector(s, x, first_move, sectors, mapper))
      return warthog::ALLMOVE;
    else if(first_move[x] == 0)
      return warthog::NOMOVE;
    else
      return first_move[x];
  };

  int node_begin = 0;
  
  unsigned short allowed_up_to_now = get_allowed(0);
  for(int i=1; i<(int)first_move.size(); ++i){
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

void CPD::append_row(int source_node, const std::vector<unsigned short>&allowed_first_move) {
  vector<int> compressed = compress(source_node, allowed_first_move);
  entry.insert(entry.end(), compressed.begin(), compressed.end());
  begin.push_back(entry.size());
}

void CPD::append_row(
  int source_node,
  const std::vector<unsigned short>& first_move,
  const Sectors& sectors,
  const Mapper& mapper) {
  vector<int> compressed = compress(source_node, first_move, sectors, mapper);
  entry.insert(entry.end(), compressed.begin(), compressed.end());
  begin.push_back(entry.size());
}

void CPD::append_rows(const CPD&other){
  int offset = begin.back();
  for(auto x:make_range(other.begin.begin()+1, other.begin.end()))
    begin.push_back(x + offset);
  std::copy(other.entry.begin(), other.entry.end(), back_inserter(entry));
}

