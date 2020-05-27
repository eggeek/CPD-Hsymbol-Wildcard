#pragma once
#include <algorithm>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

#include "adj_graph.h"
#include "catch.hpp"
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
    int dist, pid, from_direction;
  };

  struct Logger {
    string header;
    ofstream out;
    int source;

    Logger() {
      header = "x,y,rowid,order,cid,mask";
    }

    void init(string fname) {
      out = ofstream(fname);
    }

    void pheader() {
      out << header << endl;
    }

    void p(int x, int y, int s, int id, int cid, int mask) {
      out << x << "," << y << "," << s << "," << id << "," << cid << "," << mask << endl;
    }
  };
  Logger inq, pop, propa;

  LifeLongDijkstra(const AdjGraph& g, const Mapper& mapper): g(g), mapper(mapper) {
    q = min_id_heap<int>(g.node_count());
    dist.resize(g.node_count());
    allowed.resize(g.node_count());
    inv_allowed.resize(g.node_count());
    succ.resize(g.node_count());
    pa.resize(g.node_count());
    nodes.resize(g.node_count());
    propa_q.resize(g.node_count());
  }

  inline void reach(const int& from, const int& to, const int& direct, const int& d) {
    assert(dist[to] >= d);
    node& c = nodes[to];
    c.dist = d, c.pid = from, c.from_direction = direct;
    dist[to] = d;
#ifndef NDEBUG
    inq.p(mapper(to).x, mapper(to).y, inq.source, to, from, 1<<direct);
#endif
    q.push_or_decrease_key(to, c.dist);
  }

  inline int old_dist(int v, const vector<int>& dist0) {
    if (dist0[source] == INF || dist0[v] == INF) return INF;
    else return dist0[source] + dist0[v];
  }

  void repair_dist(const vector<int>& dist0, const vector<int>& succ0) {
    for (int i=0; i<propa_q_idx; i++) {
      int v = propa_q[i];
      for (auto& a: g.out(v)) {
        if ((succ0[v] & (1 << a.direction)) == 0 &&
            old_dist(a.target, dist0) >= dist[v] + a.weight &&
            dist[v] + a.weight < dist[a.target]) 
          reach(v, a.target, a.direction, dist[v] + a.weight);
      }
    }
  }

  void propagate_dist(int v, int bound, const vector<int>& dist0, const vector<int>& succ0) {
    //cerr << "propagate v: " << v << ", dist: " << dist[v] << endl;

    propa_q[propa_q_idx++] = v;
    for (auto& a: g.out(v))
    if (succ0[v] & (1 << a.direction)) {
      if (dist[v] + a.weight < dist[a.target]) {
        if (dist[v] + a.weight >= bound) {
          reach(v, a.target, a.direction, dist[v] + a.weight);
        } else {
          dist[a.target] = dist[v] + a.weight;
#ifndef NDEBUG
          propa.p(mapper(a.target).x, mapper(a.target).y, v, a.target, propa.source, propa.source / 64);
#endif
          propagate_dist(a.target, bound, dist0, succ0);
        }
      }
    }
  }

  void init(const vector<int>& dist0, const vector<int>& succ0) {
    #ifndef NDEBUG 
    int s = this->source;
    // print s0
    int s0 = -1;
    for (int i=0; i<g.node_count(); i++) if (dist0[i] == 0) {
      s0 = i;
      break;
    }

    inq.init("pushin-" + to_string(s0) + "-" + to_string(s) + ".csv");
    pop.init("popout-" + to_string(s0) + "-" + to_string(s) + ".csv");
    propa.init("propagate-" + to_string(s0) + "-" + to_string(s) + ".csv");

    inq.source = s;
    inq.pheader();

    propa.source = s;
    propa.pheader();

    pop.source = s;
    pop.pheader();

    // print s0
    inq.p(mapper(s0).x, mapper(s0).y, s0, s0, s0, 0);
    propa.p(mapper(s0).x, mapper(s0).y, s0, s0, s0, 0);
    pop.p(mapper(s0).x, mapper(s0).y, s0, s0, s0, 0);
    // print s1
    inq.p(mapper(s).x, mapper(s).y, s, s, s0, 0);
    propa.p(mapper(s).x, mapper(s).y, s, s, s0, 0);
    pop.p(mapper(s).x, mapper(s).y, s, s, s0, 0);

    // print obstacle
    for (int y=0; y<mapper.height(); y++)
    for (int x=0; x<mapper.width(); x++) if (mapper(xyLoc{(int16_t)x, (int16_t)y}) == -1) {
      inq.p(x, y, -1, -1, -1, -1);
      propa.p(x, y, -1, -1, -1, -1);
      pop.p(x, y, -1, -1, -1, -1);
    }
    #endif

    tot_prop = 0, pop_num = 0, valid_pop = 0;
    //for (int i=0; i<g.node_count(); i++) {
      //pa[i] = -1;
    //}
    fill(dist.begin(), dist.end(), INF);
  }

  void run(int s, const vector<int>& dist0, const vector<int>& succ0) {
    this->source = s;
    init(dist0, succ0);
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

      #ifndef NDEBUG
      pop.p(mapper(cid).x, mapper(cid).y, s, cid, pa[cid], 1<<c.from_direction);
      //printf("[pop queue %d (%d, %d)] dist: %d\n", cid, mapper(cid).x, mapper(cid).y, dist[cid]);
      #endif
      int f = 2;
      if (dist[cid] / 1000 > 0) { //max(mapper.height(), mapper.width()) / 10) {
        propa.source = cid;
        inq.source = cid;
        propa_q_idx = 0;
        #ifndef NDEBUG
        propa.p(mapper(cid).x, mapper(cid).y, cid, cid, cid, cid/64);
        #endif
        propagate_dist(cid, dist[cid] * f, dist0, succ0);
        tot_prop += propa_q_idx;
        repair_dist(dist0, succ0);
        propa.source = s;
        inq.source = s;
      }
      else {
        int pruned_succ = mapper.get_pruned_neighbor(cid);
        for (const auto& a: g.out(cid)) {
          reach(cid, a.target, a.direction, dist[cid] + a.weight);
        }
      }
    }
    cerr << "#propagation: " << tot_prop << ", #pop: " << pop_num << ", #valid pop: " << valid_pop << endl;
  }

  const vector<int>& get_dist() { return dist; }
  const vector<int>& get_pa() { return pa; }

private:
  const AdjGraph& g;
  const Mapper& mapper;
  min_id_heap<int> q;
  vector<int> dist;
  vector<int> pa;
  vector<int> succ;
  vector<int> propa_q;
  int propa_q_idx;
  vector<unsigned short> allowed;
  vector<unsigned short> inv_allowed;
  vector<node> nodes;
  int INF = numeric_limits<int>::max();
  int source;
  long long tot_prop, pop_num, valid_pop;
};
