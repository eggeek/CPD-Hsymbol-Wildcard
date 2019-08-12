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
  int steps;

  void reset(int size) {
    moves.resize(size);
    steps = 0;
  }

  void add(int move) {
    moves[steps++] = move;
    assert(steps <= (int)moves.size());
  }
};

double GetInvCPDCost(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c, Extracter& e, int limit=-1);
double GetInvCentroidCost(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c,
    Extracter& e1, Extracter& e2, int limit=-1);

double GetRectWildCardCost(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c, Extracter& e, int limit=-1);

double GetPathCostSRC(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c, Extracter& e, int limit=-1);
double GetForwardCentroidCost(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c, 
    Extracter& e1, Extracter& e2, int limit=-1);
