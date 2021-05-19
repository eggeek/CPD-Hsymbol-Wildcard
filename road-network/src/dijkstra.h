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
#include "geo.h"
#include "sector_wildcard.h"

using namespace std;

namespace H = Hsymbol;

class Dijkstra{
public:
  Dijkstra(const AdjGraph&g, const Mapper& mapper):
    g(g), q(g.node_count()), dist(g.node_count()), allowed(g.node_count()), mapper(mapper),
    directions(g.node_count()) {}

  const std::vector<unsigned short>&run(int source_node, int hLevel){
    std::fill(dist.begin(), dist.end(), std::numeric_limits<long long>::max());
    std::fill(allowed.begin(), allowed.end(), 0);
    fill(directions.begin(), directions.end(), 0);

    dist[source_node] = 0;
    allowed[source_node] = 0;
    directions[source_node] = warthog::ALLMOVE;

    auto reach = [&](const OutArc& a, long long d, unsigned short first_move){
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
      // quantize
      long long weight = (a.weight + mapper.quant - 1) / mapper.quant;
      reach(a, weight, (1 << a.direction));
    }

    while(!q.empty()){
      int x = q.pop();
      for(auto a:g.out(x)) {
        //if (neighbors & (1 << a.direction))
        
        // quantize
        long long weight = (a.weight + mapper.quant - 1) / mapper.quant;
        reach(a, dist[x] + weight, allowed[x]);
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

  long long distance(int target)const{
    return dist[target];
  }

  int get_directions(int t) {
    return directions[t];
  }

  const std::vector<unsigned short>& run(
      int source_node, int hLevel,
      Sectors& sectors, bool ext = false) {
    allowed = run(source_node, hLevel);
    vector<Sectors::item> items;
    xyLoc s = mapper(source_node);
    for (int i=0; i<g.node_count(); i++) if (i != source_node) {
      xyLoc t = mapper(i);
      long double angle = Geo::angle_ccw(xyLoc{t.x - s.x, t.y - s.y});
      // remove h symbol
      int item_mask = allowed[i] & (warthog::ALLMOVE ^ warthog::HMASK);
      items.push_back({item_mask, i, angle});
    }
    assert((int)items.size() == g.node_count() - 1);
    sort(items.begin(), items.end(), Sectors::itemcmp);
    sectors.init(g.out_deg(source_node));
    sectors.build(items);
    if (ext)
      sectors.extend(items, allowed);
    return allowed;
  }

  const vector<long long>& get_dist() const {
    return dist;
  }

private:
  const AdjGraph&g;
  min_id_heap<long long >q;
  std::vector<long long>dist;
  std::vector<unsigned short>allowed;
  const Mapper& mapper;
  vector<int> directions;
};

#endif
