#pragma once
#include "constants.h"
#include "vec_io.h"
#include "coord.h"
#include "geo.h"
#include <cmath>
#include <vector>
#include <iostream>
using namespace std;
class Sectors {
public:
  struct bound {
    long double lb, ub;
    int cnt, mask;
  };

  struct item {
    int mask, id;
    long double rad;
  };

  static bool itemcmp(item a, item b) {
    return a.rad < b.rad;
  }

  vector<bound> bounds;

  void init(int deg) {
    bounds.resize(deg);

    for (auto& it: bounds) {
      it.lb = M_PI;
      it.ub = -M_PI;
      it.cnt = 0;
    }
  }

  void update(int direction, long double lb, long double ub, int cnt, int mask) {
    assert(direction < bounds.size());
    if (bounds[direction].cnt < cnt) {
      bounds[direction].cnt = cnt;
      bounds[direction].lb = lb;
      bounds[direction].ub = ub;
      bounds[direction].mask = mask;
    }
  }

  void build(const vector<item>& items) {
    int tot_items = (int)items.size();
    for (int i=0; i<tot_items; i++) {
      int cur_mask = items[i].mask;
      int j = i;
      while (j+1 < tot_items && (cur_mask & items[j+1].mask)) {
        cur_mask &= items[j+1].mask;
        ++j;
      }
      int mask = warthog::lowb(cur_mask);
      int direction = warthog::m2i.at(mask);
      update(direction, items[i].rad, items[j].rad, j - i + 1, cur_mask);
      i = j;
    }
  }

  void extend(const vector<item>& items, vector<unsigned short>& allowed) {
    int deg = bounds.size();
    for (const auto& it: items) {
      int d = find_closest_sector(*this, it.rad);
      if (is_in_sector(d, it.rad)) continue;
      else if (allowed[it.id] & (1 << d)) {
        allowed[it.id] |= warthog::SHMASK;
      }
      else if (allowed[it.id] & (1 << ((d + 1) % deg))) {
        allowed[it.id] |= warthog::SSMASK;
      }
    }
  }

  inline bool is_in_sector(int d, long double rad) const {
    if (d >= (int)bounds.size()) return false;
    return rad >= bounds[d].lb && rad <= bounds[d].ub;
  }

  void save(FILE* f) {
    save_vector(f, bounds); 
  }

  void load(FILE* f) {
    bounds = load_vector<bound>(f);
  }

  static inline int find_sector(
    const Sectors& sectors,
    const xyLoc& s, const xyLoc& t) {
    long double rad = Geo::angle_ccw({t.x - s.x, t.y - s.y});
    for (int i=0; i<(int)sectors.bounds.size(); i++) {
      if (sectors.is_in_sector(i, rad)) return i;
    }
    return -1;
  }

  static inline int find_closest_sector(
    const Sectors& sectors,
    const long double& rad) {
    int res = -1;
    long double maxr = -2*M_PI;
    for (int i=0; i<(int)sectors.bounds.size(); i++) {
      if (sectors.bounds[i].lb < rad && sectors.bounds[i].lb > maxr) {
        maxr = sectors.bounds[i].lb;
        res = i;
      }
    }
   
    if (res == -1) {
      for (int i=0; i<(int)sectors.bounds.size(); i++) {
        if (sectors.bounds[i].lb > maxr) {
          maxr = sectors.bounds[i].lb;
          res = i;
        }
      }
    }
    return res;
  }
};

