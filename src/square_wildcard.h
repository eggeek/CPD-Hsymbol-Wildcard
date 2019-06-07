#include "mapper.h"

class SquareWildcard {
  const Mapper& mapper;
  const xyLoc& loc;

  public:
  SquareWildcard(const Mapper& m, const xyLoc& l):
    mapper(m), loc(l) {}

  int computeMaxSquare(const vector<unsigned short>& allowed) {
	  const int16_t* dxv = warthog::dx;
  	const int16_t* dyv = warthog::dy;
      int x = loc.x;
      int y = loc.y;
      int side = 1;
      int node_count = 1;
      bool expandSquare = true;

      auto is_valid = [&](int dx, int dy)
      {
          xyLoc loc1;
          loc1.x = x+dx;
          loc1.y = y+dy;
          if((x+dx >= 0)&&(x+dx < mapper.width())&&(y+dy >= 0)&&(y+dy < mapper.height())&&(mapper.operator()(loc1) != -1))
          {
              for(int j=0; j<8; j++)
              {
                  if((allowed[mapper.operator()(loc1)]&(1 << j)) != 0)
                  {
                      xyLoc d;
                      d.x = loc.x;
                      d.y = loc.y;
                      d.x += dxv[j];
                      d.y += dyv[j];
                      if(is_nat(loc, d, loc1))
                      {
                          return 1;
                      }
                  }
              }
              return -1;
          }
          else
          {
              return 0;
          }
      };

      while(expandSquare)
      {
        int nodes_visited = 0;
        side += 2;
        for(int dy = -(side-1)/2; dy <= (side-1)/2; dy++)
        {
            int dx = (side-1)/2;
            if((is_valid(-dx, dy) == -1)||(is_valid(dx, dy) == -1))
            {
                return side-2;
            }
            else
            {
                nodes_visited += is_valid(-dx, dy);
                nodes_visited += is_valid(dx, dy);
            }
        }
        for(int dx = -(side-1)/2 + 1; dx<= (side-1)/2 -1; dx++)
        {
            int dy = (side-1)/2;
            if((is_valid(dx, -dy) == -1)||(is_valid(dx, dy) == -1))
            {
                return side-2;
            }
            else
            {
                nodes_visited += is_valid(dx, -dy);
                nodes_visited += is_valid(dx, dy);
            }
        }
        if(nodes_visited == 0)
        {
            return side-2;
        }
        else {
          node_count += nodes_visited;
          if (node_count == mapper.node_count()) {
              return side;
          }
        }
      }
    }

    bool is_nat(xyLoc source, xyLoc target, xyLoc node)
    {
        int dx_target = (source.x - target.x);
        int dy_target = (source.y - target.y);
        int dx_node = (source.x - node.x);
        int dy_node = (source.y - node.y);
        if(((dx_node > 0) && (dx_target > 0))||((dx_node < 0) && (dx_target < 0))||((dx_node == 0) && (dx_target == 0)))
        {
            if((dy_node > 0) && (dy_target > 0))
                return true;
            if((dy_node < 0) && (dy_target < 0))
                return true;
            if((dy_node == 0) && (dy_target == 0))
                return true;
        }
        return false;
    }
};
