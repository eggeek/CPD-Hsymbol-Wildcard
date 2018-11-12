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

      int ds = directions[x];
      int neighbors = 0;
      while (ds > 0) {
        int d = ds & (-ds); //get the lowest bit
        neighbors |= warthog::jps::compute_successors((warthog::jps::direction)d, mapper.get_tiles(x));
        ds -= d;
      }

      for(auto a:g.out(x)) if (neighbors & (1 << a.direction)) {
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

private:
  const AdjGraph&g;
  min_id_heap<int>q;
  std::vector<int>dist;
  std::vector<unsigned short>allowed;
  const Mapper& mapper;
  vector<int> directions;
};

#endif
