#pragma once
#include <stdint.h>
#include "geo.h"
#include "mapper.h"
using namespace std;
const uint32_t MASK = ((uint32_t)1<<31) - 1;
#define signbit(x) (((x) >> 31) - (-(x) >> 31))
#define iabs(x) ( (((x) >> 31) ^ (x)) - (((x) >> 31)) )
#define iabs2(x) (((x) & MASK) - ((x) >> 31))
namespace Hsymbol {

  static inline int get_angle_heuristic(const int s, const int t, const Mapper& mapper) {
    const xyLoc sloc = mapper(s);
    const xyLoc tloc = mapper(t);
    double dx = (double)(tloc.x - sloc.x) / (double)mapper.quant;
    double dy = (double)(tloc.y - sloc.y) / (double)mapper.quant;
    double cos = -1;
    int move = warthog::INVALID_MOVE;
    for (const auto& it: mapper.g.out(s)) {
      const xyLoc nxt = mapper(it.target);
      double vx = (double)(nxt.x - sloc.x) / (double)mapper.quant;
      double vy = (double)(nxt.y - sloc.y) / (double)mapper.quant;

      double crossp = vx * dx + vy * dy;
      double nxt_mag = sqrt((double)(vx * vx) + (double)(vy * vy));
      double d_mg = sqrt((double)(dx * dx) + (double)(dy * dy));
      double cosi = crossp / (nxt_mag * d_mg);
      if (cosi > cos) {
        cos = cosi;
        move = it.direction;
      }
    }
    return move;
  }


  static inline int get_angle_heuristic_order(int s, int t, int mask, const Mapper& mapper) {
    struct item {
      int mask;
      long double value;
    };
    vector<item> items;
    xyLoc sloc = mapper(s);
    xyLoc tloc = mapper(t);
    double dx = (double)(tloc.x - sloc.x);
    double dy = (double)(tloc.y - sloc.y);

    for (const auto& it: mapper.g.out(s)) {
      const xyLoc nxt = mapper(it.target);
      double vx = (double)(nxt.x - sloc.x);
      double vy = (double)(nxt.y - sloc.y);

      double crossp = vx * dx + vy * dy;
      double nxt_mag = sqrt((double)(vx * vx) + (double)(vy * vy));
      double d_mg = sqrt((double)(dx * dx) + (double)(dy * dy));
      double cosi = crossp / (nxt_mag * d_mg);
      items.push_back({1 << it.direction, cosi});
    }
    auto cmp = [&](const item& a, const item& b) {
      return a.value > b.value;
    };
    sort(items.begin(), items.end(), cmp);

    int res = mask;
    if (items.size() > 0 && items[0].mask & mask) res |= warthog::HMASK;
    if (items.size() > 1 && items[1].mask & mask) res |= warthog::SHMASK;
    /*
    for (int i=0; i<(int)items.size(); i++) if (mask & items[i].mask) {
      res |= 1 << i;
    }
    */
    return res;
  }


  static inline int get_distance_heuristic(const int s, const int t, const Mapper& mapper) {
    int move = warthog::INVALID_MOVE;
    const xyLoc end = mapper(t);
    double best = -1;
    for (const auto& it: mapper.g.out(s)) {
      const xyLoc nxt = mapper(it.target);
      double hvalue = Geo::distance_m(nxt, end) * 10 + it.weight;
      if (best < 0 || best > hvalue) {
        best = hvalue;
        move = it.direction;
      }
    }
    return move;
  }

  static inline int get_distance_heuristic_order(int s, int t, int mask, const Mapper& mapper) {
    struct item {
      int mask;
      double value;
    };
    vector<item> items;
    const xyLoc end = mapper(t);
    for (const auto& it: mapper.g.out(s)) {
      const xyLoc nxt = mapper(it.target);
      double hvalue = Geo::distance_m(nxt, end) * 10 + it.weight;
      items.push_back({1 << it.direction, hvalue});
    }
    auto cmp = [&](const item& a, const item& b) {
      return a.value < b.value;
    };
    sort(items.begin(), items.end(), cmp);
    int res = mask;
    if (items.size() > 0 && items[0].mask & mask) res |= warthog::HMASK;
    if (items.size() > 1 && items[1].mask & mask) res |= warthog::SHMASK;
    /*
    for (int i=0; i<(int)items.size(); i++) if (mask & items[i].mask) {
      res |= 1 << i; 
    }
    */
    return res;
  }


  static inline void encode (const int s, vector<unsigned short>& allowed, const Mapper& mapper, int hLevel) {
    for (int v=0; v<(int)allowed.size(); v++) if (v != s) {
      int hmove = warthog::INVALID_MOVE;
      switch (hLevel) {
        case 1:
          hmove = get_angle_heuristic(s, v, mapper);
          break;
        case 2:
          hmove = get_distance_heuristic(s, v, mapper);
          break;
        case 3:
          allowed[v] = get_angle_heuristic_order(s, v, allowed[v], mapper);
          break;
        case 4:
          allowed[v] = get_distance_heuristic_order(s, v, allowed[v], mapper);
          break;
        default:
          cerr << "undefined hlevel" << endl;
          break;
      }

      if (allowed[v] & (1 << hmove))
        allowed[v] |= warthog::HMASK;
    }
  }

  static inline int decode(const int s, const int t, const Mapper& mapper, int hLevel) {
    int hmove = warthog::INVALID_MOVE;
    switch (hLevel) {
      case 1:
        hmove = get_angle_heuristic(s, t, mapper);
        break;
      case 2:
        hmove = get_distance_heuristic(s, t, mapper);
        break;
      default:
        cerr << "undefined hlevel" << endl;
        break;
    }
    return hmove;
  }
};
