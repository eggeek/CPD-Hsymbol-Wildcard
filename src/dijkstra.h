#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <algorithm>
#include <vector>
#include <limits>

#include "adj_graph.h"
#include "heap.h"
#include "mapper.h"
#include "constants.h"
#include "Hsymbol.h"

using namespace std;

namespace H = Hsymbol;

class Dijkstra{
public:
  Dijkstra(const AdjGraph&g, const Mapper& mapper):
    g(g), q(g.node_count()), dist(g.node_count()), allowed(g.node_count()), mapper(mapper),
    directions(g.node_count()) {}

  const std::vector<unsigned short>&run(int source_node, int hLevel){
    std::fill(dist.begin(), dist.end(), std::numeric_limits<int>::max());
    std::fill(allowed.begin(), allowed.end(), 0);
    fill(directions.begin(), directions.end(), 0);

    dist[source_node] = 0;    
    allowed[source_node] = 0;
    directions[source_node] = warthog::ALLMOVE;

    auto reach = [&](const OutArc& a, int d, unsigned short first_move){
      int v = a.target;
      if(d < dist[v]){
        q.push_or_decrease_key(v, d);
        dist[v] = d;
        allowed[v] = first_move;
        directions[v] = 1 << a.direction;
      }else if(d == dist[v]){
        allowed[v] |= first_move;
        directions[v] |= 1 << a.direction;
      }
    };

    for(int i=0; i<g.out_deg(source_node); ++i){
      auto a = g.out(source_node, i);
      reach(a, a.weight, (1 << a.direction));
    }

    while(!q.empty()){
      int x = q.pop();

      /*
      int ds = directions[x];
      int neighbors = 0;
      while (ds > 0) {
        int d = ds & (-ds); //get the lowest bit
        neighbors |= warthog::jps::compute_successors((warthog::jps::direction)d, mapper.get_jps_tiles(x));
        ds -= d;
      }
      */

      for(auto a:g.out(x)) {
        //if (neighbors & (1 << a.direction))
        reach(a, dist[x] + a.weight, allowed[x]);
      }
    }
    if (hLevel)
      H::encode(source_node, allowed, mapper, hLevel);
    #ifndef NDEBUG
    for(int u=0; u<g.node_count(); ++u)
      for(auto uv : g.out(u)){
        int v = uv.target;
        assert(dist[u] >= dist[v] - uv.weight);
      }
    #endif
    return allowed;
  }

  int distance(int target)const{
    return dist[target];
  }

  int get_directions(int t) {
    return directions[t];
  }

    const std::vector<unsigned short>&run(int source_node, int hLevel, vector<int>&square_side){

        allowed = run(source_node, hLevel);
        int side = computeMaxSquare(mapper, mapper.operator()(source_node));
        square_side.push_back(side);
        return allowed;
  }

private:
  const AdjGraph&g;
  min_id_heap<int>q;
  std::vector<int>dist;
  std::vector<unsigned short>allowed;
  const Mapper& mapper;
  vector<int> directions;


    int computeMaxSquare(Mapper mapper, xyLoc loc)
    {
	const int16_t* dxv = warthog::dx;
  	const int16_t* dyv = warthog::dy;
        int x = loc.x;
        int y = loc.y;
        if(mapper.operator()(loc) == -1)
        {
            bool tryToGetObstacle = false;
        }
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

#endif
