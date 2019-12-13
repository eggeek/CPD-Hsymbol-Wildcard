#pragma once
#include "preprocessing.h"
using namespace std;

struct Counter {
  int access_cnt;
  int steps;
  double pathcost;
};

struct Extracter {
  vector<int> moves;
  vector<int> vis;
  int steps;
  int scenid;
  int last;

  void init(int size) {
    moves.resize(size);
    vis.resize(size);
    fill(vis.begin(), vis.end(), -1);
    scenid = -1;
    steps = 0;
  }

  void reset(int id) {
    steps = 0;
    scenid = id;
  }

  void add(int move) {
    moves[steps++] = move;
    assert(steps <= (int)moves.size());
  }

  void mark(int locid) {
    vis[locid] = scenid;
  }

  inline bool isVis(int locid) {
    return vis[locid] == scenid;
  }
};

double GetInvCPDCost(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c, Extracter& e, int limit=-1);
double GetInvCentroidCost(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c,
    Extracter& e1, Extracter& e2, int limit=-1);

double GetRectWildCardCost(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c, Extracter& e, int limit=-1);

double GetPathCostSRC(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c, Extracter& e, int limit=-1);
double GetForwardCentroidCost(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c, 
    Extracter& e1, Extracter& e2, int limit=-1);
