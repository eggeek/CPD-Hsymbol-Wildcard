#pragma once
#include <fstream>
#include <sstream>
#include <math.h>
#include "list_graph.h"
#include "mapper.h"
using namespace std;

namespace RoadNetwork {

struct Scen {
  int s, t;
  long long dist;
  string mname;
};


vector<Scen> LoadScenarios(string path) {
  ifstream in(path);
  string line;
  string header = "s,t,dist,map";
  vector<Scen> scens;
  while (getline(in, line)) {
    if (line == header) continue; // ignore header
    istringstream tokenStream(line);
    string s, t, dist, mname;
    getline(tokenStream, s, ',');
    getline(tokenStream, t, ',');
    getline(tokenStream, dist, ',');
    getline(tokenStream, mname, ',');
    Scen scen = Scen{stoi(s), stoi(t), stoll(dist), mname};
    scens.push_back(scen);
  }
  return scens;
}


void LoadCoord(string copath, vector<xyLoc>& coord) {
  ifstream co(copath);
  string line;
  int n;
  while (getline(co, line)) {
    stringstream ss(line);
    string desc;
    ss >> desc;
    if (desc == "c") continue;
    else if (desc == "p") {
      string aux, sp, CO;
      ss >> aux >> sp >> CO >> n;
      coord.resize(n);
    }
    else if (desc == "v") {
      int vid;
      long long x, y;
      ss >> vid >> x >> y;
      vid--;
      coord[vid] = {x, y};
    }
  }
}

void LoadGraph(string grpath, vector<Arc>& es, vector<vector<int>>& g) {
  ifstream gr(grpath);
  int n, m = 0, eid = 0;
  string line;
  while (getline(gr, line)) {
    stringstream ss(line);
    string desc;
    ss >> desc;
    if (desc == "p") {
      string sp;
      ss >> sp >> n >> m;
      g.resize(n);
      es.resize(m);
    }
    else if (desc == "a") {
      int u, v;
      long long w;
      ss >> u >> v >> w;
      // 1-based to 0-based
      u--, v--;
      es[eid++] = Arc{u, v, w, 0};
    }
  }

  for (int i=0; i<m; i++) {
    g[es[i].source].push_back(i);
  }
}

ListGraph Load(string grpath) {
  vector<Arc> es;
  vector<vector<int>> g;
  LoadGraph(grpath, es, g);
  ListGraph listg(g.size());
  for (int i=0; i<(int)g.size(); i++) {
    for (int j=0; j<(int)g[i].size(); j++) {
      const Arc& e = es[g[i][j]];
      listg.arc.push_back({e.source, e.target, e.weight, j});
    }
  }
  return listg;
}


ListGraph Load(string grpath, string copath, vector<xyLoc>& coord) {
  vector<Arc> es;
  vector<vector<int>> g;
  LoadGraph(grpath, es, g);
  LoadCoord(copath, coord);
  ListGraph listg(g.size());
  for (int i=0; i<(int)g.size(); i++) {
    auto cmp = [&](int u, int v) {
      const Arc& e1 = es[u];
      const Arc& e2 = es[v];
      int v1 = e1.source + e1.target - i;
      int v2 = e2.source + e2.target - i;
      long long dx1 = coord[v1].x - coord[i].x;
      long long dy1 = coord[v1].y - coord[i].y;
      long long dx2 = coord[v2].x - coord[i].x;
      long long dy2 = coord[v2].y - coord[i].y;
      return atan2((double)dy1, (double)dx1) < atan2((double)dy2, (double)dx2);
    };
    sort(g[i].begin(), g[i].end(), cmp);
    for (int j=0; j<(int)g[i].size(); j++) {
      const Arc& e = es[g[i][j]];
      int from = i;
      int to = e.source + e.target - from;
      listg.arc.push_back({from, to, e.weight, j});
    }
  }
  return listg; 
}

ListGraph LoadSub(string grpath, string copath, vector<xyLoc>& coord, double ratio) {
  vector<xyLoc> coord_all;
  vector<Arc> es;
  vector<vector<int>> g;
  map<int, int> reorder;
  LoadGraph(grpath, es, g);
  LoadCoord(copath, coord_all);
  long long maxx, maxy, minx, miny;
  maxx = minx = coord_all[0].x;
  maxy = miny = coord_all[0].y;
  for (const auto& it: coord_all) {
    maxx = max(maxx, it.x);
    minx = min(minx, it.x);
    maxy = max(maxy, it.y);
    miny = min(miny, it.y);
  }
  long long midx = (maxx + minx) / 2, midy = (maxy + miny) / 2;
  double rx = (maxx - minx) * ratio;
  double ry = (maxy - miny) * ratio;
  for (int i=0; i<(int)coord_all.size(); i++) {
    if (fabs(coord_all[i].x - midx) > rx ||
        fabs(coord_all[i].y - midy) > ry) continue;
    else {
      reorder[i] = coord.size();
      coord.push_back(coord_all[i]);
    }
  }
  ListGraph listg(coord.size());
  for (int i=0; i<(int)g.size(); i++) {
    if (reorder.find(i) == reorder.end()) continue;
    auto cmp = [&](int u, int v) {
      const Arc& e1 = es[u];
      const Arc& e2 = es[v];
      int v1 = e1.source + e1.target - i;
      int v2 = e2.source + e2.target - i;
      long long dx1 = coord_all[v1].x - coord_all[i].x;
      long long dy1 = coord_all[v1].y - coord_all[i].y;
      long long dx2 = coord_all[v2].x - coord_all[i].x;
      long long dy2 = coord_all[v2].y - coord_all[i].y;
      return atan2((double)dy1, (double)dx1) < atan2((double)dy2, (double)dx2);
    }; 
    sort(g[i].begin(), g[i].end(), cmp);
    int cnte = 0;
    for (int j=0; j<(int)g[i].size(); j++) {
      const Arc& e = es[g[i][j]];
      int from = i;
      int to = e.source + e.target - from;
      if (reorder.find(to) == reorder.end()) continue;
      listg.arc.push_back({reorder[from], reorder[to], e.weight, cnte++});
    }
  }
  return listg;
}

void print(const AdjGraph& g) {
  int arc_cnt = 0;
  for (int i=0; i<(int)g.node_count(); i++) {
    arc_cnt += g.out_deg(i);
  }
  assert(arc_cnt % 2 == 0);
  arc_cnt /= 2;
  cout << "p sp " << g.node_count() << " " << arc_cnt << endl;
  for (int i=0; i<(int)g.node_count(); i++) {
    for (const auto& it: g.out(i)) {
      if (it.target > i)  {
        cout << "a " << i+1 << " " << it.target + 1 << " " << it.weight << endl; 
        arc_cnt --;
      }
    }
  }
  assert(arc_cnt == 0);
}


void print(const vector<xyLoc>& coord) {
  cout << "p aux sp co " << coord.size() << endl;
  for (int i=0; i<(int)coord.size(); i++) {
    cout << "v " << i+1 << " " << coord[i].x << " " << coord[i].y << endl;
  }
}

};
