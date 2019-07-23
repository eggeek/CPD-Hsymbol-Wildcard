#include "cpd_base.h"
#include "constants.h"

#include <fstream>
#include <stdexcept>
#include <cassert>

// compile with -O3 -DNDEBUG

void CPDBASE::append_row(int source_node, const std::vector<unsigned short>&allowed_first_move,
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

void CPDBASE::append_rows(const CPDBASE&other){
  int offset = begin.back();
  for(auto x:make_range(other.begin.begin()+1, other.begin.end()))
    begin.push_back(x + offset);
  std::copy(other.entry.begin(), other.entry.end(), back_inserter(entry));
}
