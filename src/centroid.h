#pragma once
#include "mapper.h"
#include "adj_graph.h"
#include <vector>
#include <queue>
using namespace std;

static inline int find_centroid_seed(int h0, int h1, int w0, int w1, const vector<int>& fa, const Mapper& mapper) {
  int mh = (h1 + h0) >> 1, mw = (w1 + w0) >> 1, h, w;
  for (int dh=h0-mh; dh<=h1-mh; dh++) {
    for (int dw=w0-mw; dw<=w1-mw; dw++) {
      h = mh + dh;
      w = mw + dw;
      int id = mapper(xyLoc{(int16_t)w, (int16_t)h});
      if (id != -1 && fa[id] == -1) return id;
    }
  }
  return -1;
}

static inline int bfs(int s, int r, vector<int>& fa, vector<int>& vis, const Mapper& mapper, bool refine=false) {
  priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>>q;
  vector<double> dist(mapper.node_count(), 1e10);
  q.push({0, s});
  dist[s] = 0;
  int best = -1;
  while (!q.empty()) {
    pair<double, int> c = q.top(); q.pop();
    if (c.first > dist[c.second]) continue;
    if (best == -1 || ((mapper(c.second).x + mapper(c.second).y) >= mapper(best).x + mapper(best).y && fa[best] == -1))
      best = c.second;
    if (refine) {
      fa[c.second] = s;
      vis[c.second] = int(c.first);
    }
    int neigbhors = mapper.get_neighbor(c.second);
    while (neigbhors) {
      int direction = warthog::lowb(neigbhors);
      neigbhors -= direction;
      int move = warthog::m2i.at(direction);
      int nxtx = mapper(c.second).x + warthog::dx[move];
      int nxty = mapper(c.second).y + warthog::dy[move];
      pair<double, int> nxt = {c.first + warthog::doublew[move], mapper(xyLoc{(int16_t)nxtx, (int16_t)nxty})};
      if (nxt.first > (double)r) continue;
      if (refine) {
        if (vis[nxt.second] > nxt.first && dist[nxt.second] > nxt.first) {
          dist[nxt.second] = nxt.first;
          q.push(nxt);
        }
      }
      else {
        if (dist[nxt.second] > nxt.first) {
          dist[nxt.second] = nxt.first;
          q.push(nxt);
        }
      }
    }
  }
  return best;
}

static inline vector<int> compute_centroid(Mapper& mapper, int r) {
  int height = mapper.height();
  int width = mapper.width();
  int INF = height * width + 1;
  vector<int> centroids;
  vector<int> fa(mapper.node_count(), -1);
  vector<int> vis(mapper.node_count(), INF);
  for (int h=0; h<height; h += r)
  for (int w=0; w<width;  w += r) {
    int h0 = h, h1 = min(h+r-1, height-1);
    int w0 = w, w1 = min(w+r-1, width-1);
    int c = find_centroid_seed(h0, h1, w0, w1, fa, mapper);
    if (c != -1) {
      c = bfs(c, r, fa, vis, mapper, false);
      bfs(c, r, fa, vis, mapper, true);
      centroids.push_back(c);
    }
  }

  for (int i=0; i<mapper.node_count(); i++) if (fa[i] == -1) {
    centroids.push_back(i);
    bfs(i, r, fa, vis, mapper, true);
  }
  sort(centroids.begin(), centroids.end());
  mapper.set_centroids(fa);
  cerr << "#centroids: " << centroids.size() << ", total: " << mapper.node_count() << endl;
  return centroids;
}
