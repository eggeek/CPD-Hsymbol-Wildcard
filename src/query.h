#pragma once
#include "preprocessing.h"
#include "cpd.h"
using namespace std;

double GetPathCostSRC(const Index& data, xyLoc s, xyLoc g, int hLevel, int limit=-1);
double GetPath(const Index& data, xyLoc s, xyLoc g, std::vector<xyLoc> &path,
    warthog::jpsp_oracle& oracle, int hLevel, int limit=-1);//, int &callCPD);
