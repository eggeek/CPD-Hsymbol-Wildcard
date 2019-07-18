#include "mapper.h"
#include "adj_graph.h"
#include <vector>
#include <queue>
using namespace std;

int find_square_centroid(int h0, int h1, int w0, int w1, const Mapper& mapper) {
  int mh = (h1 + h0) >> 1, mw = (w1 + w0) >> 1, h, w;
  for (int dh=h0-mh; dh<=h1-mh; dh++) {
    for (int dw=w0-mw; dw<=w1-mw; dw++) {
      h = mh + dh;
      w = mw + dw;
      int id = mapper(xyLoc{(int16_t)w, (int16_t)h});
      if (id != -1) return id;
    }
  }
  return -1;
}

void bfs(int s, int r, vector<int>& fa, vector<bool>& vis, const Mapper& mapper,
    int x0, int x1, int y0, int y1) {
  priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>>q;
  vector<double> dist(mapper.node_count(), 1e10);
  q.push({0, s});
  while (!q.empty()) {
    pair<double, int> c = q.top(); q.pop();
    vis[c.second] = true;
    fa[c.second] = s;
    int neigbhors = mapper.get_neighbor(c.second);
    while (neigbhors) {
      int direction = warthog::lowb(neigbhors);
      neigbhors -= direction;
      int move = warthog::m2i.at(direction);
      int nxtx = mapper(c.second).x + warthog::dx[move];
      int nxty = mapper(c.second).y + warthog::dy[move];
      pair<double, int> nxt = {c.first + warthog::doublew[move], mapper(xyLoc{(int16_t)nxtx, (int16_t)nxty})};
      dist[nxt.first] = min(dist[nxt.second], nxt.first);
      if (x0 == -1) {
        if (nxt.first <= (double)r/2 && !vis[nxt.second]) {
          q.push(nxt);
        }
      }
      else if (nxtx >= x0 && nxtx <= x1 && nxty >= y0 && nxty <= y1) {
        if (!vis[nxt.second]) {
          q.push(nxt);
        }
      }
    }
  }
}

vector<int> compute_centroid(Mapper& mapper, int r) {
  int height = mapper.height();
  int width = mapper.width();
  vector<int> centroids;
  vector<int> fa(mapper.node_count(), -1);
  vector<bool> vis(mapper.node_count(), false);
  for (int h=0; h<height; h += r)
  for (int w=0; w<width;  w += r) {
    int h0 = h, h1 = min(h+r-1, height-1);
    int w0 = w, w1 = min(w+r-1, width-1);
    int c = find_square_centroid(h0, h1, w0, w1, mapper);
    if (c != -1) {
      centroids.push_back(c);
      bfs(c, r, fa, vis, mapper, w0, w1, h0, h1);
    }
  }

  for (int i=0; i<mapper.node_count(); i++) if (!vis[i]) {
    centroids.push_back(i);
    bfs(i, r, fa, vis, mapper, -1, -1, -1, -1);
  }
  sort(centroids.begin(), centroids.end());
  mapper.set_centroids(fa);
  cerr << "#centroids: " << centroids.size() << ", total: " << mapper.node_count() << endl;
  return centroids;
}
