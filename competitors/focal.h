#pragma once
#include <queue>
#include <algorithm>
#include "adj_graph.h"
#include "mapper.h"
#include "heap.h"
using namespace std;

class Focal {
public:
  const int INF = numeric_limits<int>::max();
  Focal(const AdjGraph& g, const Mapper& mapper):
    g(g), mapper(mapper), q(g.node_count()), dist(g.node_count()) {};

  double run(int s, int t, int L) {
    fill(dist.begin(), dist.end(), INF);
    q.clear();
    while (!q2.empty()) q2.pop();
    dist[s] = 0;

    auto reach = [&](const OutArc& a, double curd) {
      double nextd = curd + warthog::doublew[a.direction];
      if (nextd < dist[a.target]) {
        double f = nextd + heuristic(a.target, t);
        q.push_or_decrease_key(a.target, f);
        dist[a.target] = nextd;
      }
    };

    for (int i=0; i<g.out_deg(s); ++i) {
      auto a = g.out(s, i);
      reach(a, 0);
    }

    while (true) {
      if (q.empty() && q2.empty()) break;

      double Fmin = getFmin();
      while (!q.empty()) {
        double fvalue = q.peek_min_key();
        int id = q.peek_min_id();
        if (Fmin == INF || fvalue - L <= Fmin) {
          Fmin = min(Fmin, fvalue);
          FocalNode nxt{id, fvalue, dist[id]};
          q2.push(nxt);
          q.pop();
        }
        else break;
      }
      FocalNode cur = q2.top(); q2.pop();
      if (cur.id == t) {
        assert(cur.f == cur.g);
        return cur.g;
      }
      for (int i=0; i<g.out_deg(cur.id); ++i) {
        auto a = g.out(cur.id, i);
        reach(a, dist[cur.id]);
      }
    }
    return 0;
  }

private:
  struct FocalNode {
    int id;
    double f, g;
    bool operator < (const FocalNode& rhs) const {
      return g < rhs.g;
    }
  };

  template <class T, class S, class C>
      S& Container(priority_queue<T, S, C>& q) {
          struct HackedQueue : private priority_queue<T, S, C> {
              static S& Container(priority_queue<T, S, C>& q) {
                  return q.*&HackedQueue::c;
              }
          };
      return HackedQueue::Container(q);
  }

  double heuristic(int s, int t) {
    const xyLoc& sloc = mapper(s);
    const xyLoc& tloc = mapper(t);
    double dx = fabs(sloc.x - tloc.x);
    double dy = fabs(sloc.y - tloc.y);
    double dia = min(dx, dy);
    return dia * warthog::DBL_ROOT_TWO + (dx - dia) + (dy - dia);
  }

  double getFmin() {
    vector<FocalNode>& data = Container(q2);
    double res = INF;
    for (const auto& it: data) res = min(res, it.f);
    return res;
  }

  const AdjGraph& g;
  const Mapper& mapper;
  min_id_heap<double> q;
  priority_queue<FocalNode, vector<FocalNode>> q2;
  vector<double> dist;
};
