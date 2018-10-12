#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <algorithm>
#include <vector>
#include <limits>

#include "adj_graph.h"
#include "heap.h"
#include "mapper.h"
#include "constants.h"

class Dijkstra{
public:
  Dijkstra(const AdjGraph&g, const Mapper& mapper):
    g(g), q(g.node_count()), dist(g.node_count()), allowed(g.node_count()), mapper(mapper){}

  static int get_heuristic_move(int s, int t, const Mapper& mapper) {
    xyLoc sloc = mapper(s);
    xyLoc tloc = mapper(t);
    int dx = sloc.x - tloc.x;
    int dy = sloc.y - tloc.y;
    if (dx < 0) dx = -1;
    else dx = dx?1: 0;
    if (dy < 0) dy = -1;
    else dy = dy?1: 0;
    return warthog::v2i[dx+1][dy+1];
  }

  const std::vector<unsigned short>&run(int source_node){
    std::fill(dist.begin(), dist.end(), std::numeric_limits<int>::max());
    std::fill(allowed.begin(), allowed.end(), 0);

    dist[source_node] = 0;    
    allowed[source_node] = 0;
    int cnth = 0;

    auto reach = [&](int v, int d, unsigned short first_move){
      if(d < dist[v]){
        q.push_or_decrease_key(v, d);
        dist[v] = d;
        allowed[v] = first_move;
      }else if(d == dist[v]){
        // previous
        ///* my 
        allowed[v] |= first_move;
        //*/
        int hmove = get_heuristic_move(source_node, v, mapper);
        if (allowed[v] & (1 << hmove)) {
          allowed[v] |= warthog::HMASK;
          cnth++;
        }
        if (allowed[v] & warthog::DIAGs) {
          allowed[v] &= warthog::ALLMOVE ^ warthog::NEWSs;
        }
      }
    };

    for(int i=0; i<g.out_deg(source_node); ++i){
      auto a = g.out(source_node, i);
      reach(a.target, a.weight, (1 << a.direction));
    }

    while(!q.empty()){
      int x = q.pop();

      for(auto a:g.out(x)) 
        reach(a.target, dist[x] + a.weight, allowed[x]);
    }

    printf("cnth: %d, ratio: %f\n", cnth, (double)cnth / (double)(mapper.node_count()));
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
private:
  const AdjGraph&g;
  min_id_heap<int>q;
  std::vector<int>dist;
  std::vector<unsigned short>allowed;
  const Mapper& mapper;
};

#endif
