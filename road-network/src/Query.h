#pragma once
#include "mapper.h"
#include "geo.h"
#include "Preprocessing.h"
using namespace std;
long long GetPathCostSRC(void *data, int s, int t, int hLevel, int limit=-1);
long long GetPath(void *data, int s, int g, vector<int> &path, int hLevel, int limit=-1);

long long GetPathCostSRCWithSectorWildCard(void *data, int s, int t, int hLevel, int limit=-1);
long long GetPathWithSectorWildCard(void *data, int s, int g, vector<int> &path, int hLevel, int limit=-1);
