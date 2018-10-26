#pragma once
#include "mapper.h"
using namespace std;

class Visualizer {
  public:
    const vector<bool>& bits;
    const Mapper& mapper;
    Visualizer(const vector<bool>& mapData, const Mapper& mapper): bits(mapData), mapper(mapper) {};

    vector<string> to_strings() {
      vector<string> res;
      for (int y=0; y<mapper.height(); y++) {
        string row = "";
        for (int x=0; x<mapper.width(); x++) {
          int idx = y * mapper.width() + x;
          if (bits[idx]) row.push_back('.');
          else row.push_back('T');
        }
        res.push_back(row);
      }
      return res;
    }

    vector<string> to_strings(int source, const vector<unsigned short>& allowed) {
      vector<string> res = to_strings();
      for (int i=0; i<(int)allowed.size(); i++) {
        xyLoc cur = mapper(i);
        if (i == source) {
          res[cur.y][cur.x] = 'S';
        } else if (allowed[i] & warthog::HMASK) {
          res[cur.y][cur.x] = 'H';
        } 
      }
      return res;
    }
};
