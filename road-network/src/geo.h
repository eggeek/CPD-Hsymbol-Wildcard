#pragma once
#include <cmath>
#include "coord.h"
using namespace std;

namespace Geo {
  const double earthRadiusM = 6371008.8;
  static inline double distance_m(double lat1, double lon1, double lat2, double lon2) {
    auto deg2rad  = [&](double deg) {
      return (deg * M_PI) / 180.0;
    };
    double lat1r, lon1r, lat2r, lon2r, u, v;
    lat1r = deg2rad(lat1);
    lon1r = deg2rad(lon1);
    lat2r = deg2rad(lat2);
    lon2r = deg2rad(lon2);
    u = sin((lat2r - lat1r)/2);
    v = sin((lon2r - lon1r)/2);
    return 2.0 * earthRadiusM * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
  }

  static inline double distance_m(xyLoc u, xyLoc v) {
    double lat1 = (double)u.x / 1000000.0;
    double lon1 = (double)u.y / 1000000.0;
    double lat2 = (double)v.x / 1000000.0;
    double lon2 = (double)v.y / 1000000.0;
    return distance_m(lat1, lon1, lat2, lon2);
  }

  static inline long double angle_ccw(xyLoc pos) {
    return atan2l((long double)pos.y, (long double)pos.x);
  }
}

