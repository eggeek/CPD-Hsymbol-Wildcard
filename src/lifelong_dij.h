#pragma once
#include <algorithm>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

#include "adj_graph.h"
#include "heap.h"
#include "jps.h"
#include "mapper.h"
#include "constants.h"
#include "Hsymbol.h"

using namespace std;

class LifeLongDijkstra {
public:

  struct node {
    int dist;
    int pid, from_direction;
  };

  LifeLongDijkstra(const AdjGraph& g, const Mapper& mapper): g(g), mapper(mapper) {
    q = min_id_heap<int>(g.node_count());
    dist.resize(g.node_count());
    pa.resize(g.node_count());
    nodes.resize(g.node_count());
    propa_q.resize(g.node_count());
    to_repair.resize(g.node_count() << 3);
  }

  inline void reach(const OutArc& a, int from, int d) {
    node& c = nodes[a.target];
    c.dist = d, c.pid = from, c.from_direction = a.direction;
    dist[a.target] = d;
    q.push_or_decrease_key(a.target, c.dist);
  }

  inline int old_dist(int v, const vector<int>& dist0) {
    if (this->delta == INF || dist0[v] == INF) return INF;
    return this->delta + dist0[v];
  }

  void repair_dist(const vector<int>& dist0) {
    const int threshold = INF;
    // parallelize when there are many
    if (propa_q_idx > threshold) {
      // parallel
      int idx = 0;
      for (int i=0; i<propa_q_idx; i++) {
        int v = propa_q[i];
        for (auto& a: g.out(v)) {
          if (pa[a.target] != v &&
              min(old_dist(a.target, dist0), dist[a.target]) >= dist[v] + a.weight) 
          to_repair[idx++] = {v, a};
        }
      }

      // single
      assert(idx < to_repair.size());
      // cerr << "propagated: " << propa_q_idx << ", to repair: " << idx << endl;
      for (int i=0; i<idx; i++) {
        const int& v = to_repair[i].first;
        const OutArc& a = to_repair[i].second;
        if (dist[v] + a.weight <= dist[a.target]) {
          reach(a, v, dist[v] + a.weight);
        }
      }
    }
    // otherwise single
    else {
      for (int i=0; i<propa_q_idx; i++) {
        int v = propa_q[i];
        for (auto& a: g.out(v)) {
          if (pa[a.target] != v &&
              min(old_dist(a.target, dist0), dist[a.target]) >= dist[v] + a.weight) 
            reach(a, v, dist[v] + a.weight);
        }
      }
    }
  }

  void propagate_dist(int v,  const vector<int>& dist0) {
    //cerr << "propagate v: " << v << ", dist: " << dist[v] << endl;
    int front = 0, tail = 0;
    propa_q[tail++] = v;
    while (front < tail) {
      int cur = propa_q[front++];
      propa_q[propa_q_idx++] = cur;
      for (auto& a: g.out(cur))
      if (pa[a.target] == cur && 
         // dist[a.target] may be modified by `repair_dist`
         (dist[cur] + a.weight < dist[a.target])) {

         // this can be written by only one thread, so it is safe.
        dist[a.target] = dist[cur] + a.weight;
        propa_q[tail++] = a.target;
      }
    }
  }

  void set_pa(const vector<int>& pa0) {
    pa = vector<int>(pa0.begin(), pa0.end());
  }

  void init(const vector<int>& dist0, int newS, int delta) {
    tot_prop = 0, pop_num = 0, valid_pop = 0;
    this->delta = delta;
    if (delta == INF) {
      fill(pa.begin(), pa.end(), -1);
      fill(dist.begin(), dist.end(), INF);
    }
    else {
      for (int i=0; i<g.node_count(); i++) {
        dist[i] = dist0[i] == INF? INF: dist0[i] + this->delta;
      }
    }
  }

  pair<int, int> run(int s, const vector<int>& dist0) {
    this->source = s;
    nodes[s] = {0, -1, -1};
    dist[s] = 0;
    q.push_or_decrease_key(s, 0);

    while (!q.empty()) {
      int cid = q.pop();
      node& c = nodes[cid];
      pop_num++;
      assert(nodes[cid].dist >= dist[cid]);
      if (nodes[cid].dist != dist[cid]) continue;
      valid_pop++;
      pa[cid] = c.pid;

      // we can control the propagation by setting up variable `f`
      // larger `f` results in more normal expansion at the beginning
      // we can also set an upper bound to avoid propagation in deep level.
      const int f = 1;
      if (dist[cid] > this->delta * f) {
        propa_q_idx = 0;
        propagate_dist(cid, dist0);
        tot_prop += propa_q_idx;
        repair_dist(dist0);
      }
      else {
        for (const auto& a: g.out(cid)) 
        if (dist[a.target] > dist[cid] + a.weight)
            reach(a, cid, dist[cid] + a.weight);
      }
    }
    cerr << "#propagation: " << tot_prop << ", #size: " << g.node_count() <<
      ", dist(s0, s): " << dist0[s] <<
      ", #pop: " << pop_num << ", #valid pop: " << valid_pop << endl;
    return {tot_prop, pop_num};
  }

  vector<int>& get_dist() { return dist; }
  const vector<int>& get_pa() { return pa; }

private:
  const AdjGraph& g;
  const Mapper& mapper;
  min_id_heap<int> q;
  vector<int> dist;
  vector<int> pa;

  vector<int> propa_q;
  int propa_q_idx;
  vector<pair<int, OutArc>> to_repair;

  vector<node> nodes;
  int INF = numeric_limits<int>::max();
  int delta;
  int source;
  int tot_prop, pop_num, valid_pop;
};
