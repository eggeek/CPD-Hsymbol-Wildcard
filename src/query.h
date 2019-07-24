#pragma once
#include "preprocessing.h"
using namespace std;

struct Counter {
  int access_cnt;
  int steps;
  double pathcost;
};

double GetInvCPDCost(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c, int limit=-1);
double GetInvCentroidCost(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c, int limit=-1);

double GetRectWildCardCost(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c, int limit=-1);

double GetPathCostSRC(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c, int limit=-1);
double GetForwardCentroidCost(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c, int limit=-1);

double GetPath(const Index& data, xyLoc s, xyLoc g, std::vector<xyLoc> &path,
    warthog::jpsp_oracle& oracle, int hLevel, int limit=-1);//, int &callCPD);
