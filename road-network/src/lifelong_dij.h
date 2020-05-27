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

namespace H = Hsymbol;

class LifeLongDijkstra {
public:

  struct node {
    long long dist;
    int pid, from_direction;
  };

  LifeLongDijkstra(const AdjGraph& g, const Mapper& mapper): g(g), mapper(mapper) {
    q = min_id_heap<long long>(g.node_count());
    dist.resize(g.node_count());
    allowed.resize(g.node_count());
    inv_allowed.resize(g.node_count());
    succ.resize(g.node_count());
    pa.resize(g.node_count());
    nodes.resize(g.node_count());
    propa_q.resize(g.node_count());
  }

  inline void reach(const OutArc& a, int from, int d) {
    if (d < dist[a.target]) {
      node& c = nodes[a.target];
      c.dist = d, c.pid = from, c.from_direction = a.direction;
      dist[a.target] = d;
      q.push_or_decrease_key(a.target, c.dist);
    }
  }

  inline int old_dist(int v, const vector<long long>& dist0) {
    if (dist0[source] == INF || dist0[v] == INF) return INF;
    else return dist0[source] + dist0[v];
  }

  void repair_dist(const vector<long long>& dist0, const vector<int>& succ0) {
    for (int i=0; i<propa_q_idx; i++) {
      int v = propa_q[i];
      for (auto& a: g.out(v)) {
        if ((succ0[v] & (1 << a.direction)) == 0 &&
            old_dist(a.target, dist0) >= dist[v] + a.weight) 
          reach(a, v, dist[v] + a.weight);
      }
    }
  }

  void propagate_dist(int v, long long bound, const vector<long long>& dist0, const vector<int>& succ0) {
    //cerr << "propagate v: " << v << ", dist: " << dist[v] << endl;
    propa_q[propa_q_idx++] = v;
    for (auto& a: g.out(v))
    if (succ0[v] & (1 << a.direction)) {
      if (dist[v] + a.weight < dist[a.target]) {
        if (dist[v] + a.weight >= bound) {
          reach(a, v, dist[v] + a.weight);
        }
        else {
          dist[a.target] = dist[v] + a.weight;
          propagate_dist(a.target, bound, dist0, succ0);
        }
      }
    }
  }

  void init(const vector<long long>& dist0, const vector<int>& succ0) {
    tot_prop = 0, pop_num = 0, valid_pop = 0;
    fill(dist.begin(), dist.end(), INF);
  }

  void run(int s, const vector<long long>& dist0, const vector<int>& succ0) {
    this->source = s;
    //init(dist0, succ0);
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
      long long f = 50;
      if (dist[cid] > dist0[s]) {
        propa_q_idx = 0;
        propagate_dist(cid, dist[cid]*f, dist0, succ0);
        tot_prop += propa_q_idx;
        repair_dist(dist0, succ0);
      }
      else {
        for (const auto& a: g.out(cid)) 
            reach(a, cid, dist[cid] + a.weight);
      }
    }
    cerr << "#propagation: " << tot_prop << ", #size: " << g.node_count() <<
      ", dist(s0, s): " << dist0[s] <<
      ", #pop: " << pop_num << ", #valid pop: " << valid_pop << endl;
  }

  vector<long long>& get_dist() { return dist; }
  const vector<int>& get_pa() { return pa; }

private:
  const AdjGraph& g;
  const Mapper& mapper;
  min_id_heap<long long> q;
  vector<long long> dist;
  vector<int> pa;
  vector<int> succ;
  vector<int> propa_q;
  int propa_q_idx;
  vector<unsigned short> allowed;
  vector<unsigned short> inv_allowed;
  vector<node> nodes;
  long long INF = numeric_limits<long long>::max();
  int source;
  long long tot_prop, pop_num, valid_pop;
};
