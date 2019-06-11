#include <vector>
#include <map>
#include "mapper.h"
#include "constants.h"
using namespace std;

struct RectInfo {
  // +----+
  // |    |
  // |    | U: len of upper edge
  // |    |
  // +----+ id: (r, c)
  // L: len of left edge
  int pos, mask, U, L;
  inline int x(int w) const { return pos % w; }
  inline int y(int w) const { return pos / w; }
};

class RectWildcard {
public:
  struct Rect {
    int L, U;
    inline int size() {
      return L * U;
    }
  };

  const Mapper& mapper;
  const xyLoc& loc;
  const vector<unsigned short>& allowed;
  vector<vector<map<int, int>>> L;
  vector<vector<map<int, int>>> U;
  vector<vector<map<int, Rect>>> rect;

  RectWildcard(const Mapper& m, const xyLoc& l, const vector<unsigned short>& allowed):
    mapper(m), loc(l), allowed(allowed) {
    L = vector<vector<map<int, int>>>(m.width(), vector<map<int, int>>(m.height()));
    U = vector<vector<map<int, int>>>(m.width(), vector<map<int, int>>(m.height()));
    rect = vector<vector<map<int, Rect>>>(m.width(), vector<map<int, Rect>>(m.height()));

    for (int x=0; x<m.width(); x++) {
      for (int y=0; y<m.height(); y++) {
        int id = mapper(xyLoc{(int16_t)x, (int16_t)y});
        int mask = id == -1? warthog::ALLMOVE: allowed[id];
        L[x][y][mask] = 1;
        U[x][y][mask] = 1;
        rect[x][y][mask] = {1, 1};
      }
    }
  };

  void updateL(int c, int r) {
    map<int, int> newEntries;
    for (auto it=L[c][r].rbegin(); it != L[c][r].rend(); it++) {
      for (const auto& it2: L[c-1][r]) {
        const int& len = it2.second;
        int key = it->first & it2.first;
        if (key == 0) continue;
        if (L[c][r].find(key) != L[c][r].end()) L[c][r][key] = max(L[c][r][key], len+1);
        else if (newEntries.find(key) == newEntries.end()) newEntries[key] = len + 1;
        else newEntries[key] = max(newEntries[key], len+1);
      }
    }
    L[c][r].insert(newEntries.begin(), newEntries.end());
  }

  void updateU(int c, int r) {
    map<int, int> newEntries;
    for (auto it=U[c][r].rbegin(); it != U[c][r].rend(); it++) {
      for (const auto& it2: U[c][r-1]) {
        const int& len = it2.second;
        int key = it->first & it2.first;
        if (key == 0) continue;
        if (U[c][r].find(key) != U[c][r].end()) U[c][r][key] = max(U[c][r][key], len+1);
        else if (newEntries.find(key) == newEntries.end()) newEntries[key] = len+1;
        else newEntries[key] = max(newEntries[key], len+1);
      }
    }
    U[c][r].insert(newEntries.begin(), newEntries.end());
  }

  int getMaxValue(int mask, const map<int, int>& entry) {
    int best = 0;
    for (const auto& it: entry) if ((it.first & mask) == mask) best = max(best, it.second);
    return best;
  }

  void updateRect(int c, int r) {
    map<int, Rect> newEntries;
    for (auto it=rect[c][r].rbegin(); it != rect[c][r].rend(); it++) {
      if (r-1 >= 0) {
        for (const auto& it2: rect[c][r-1]) {
          int key = it->first & it2.first;
          if (key == 0) continue;

          int l = min(it2.second.L, getMaxValue(key, L[c][r])); 
          int u = it2.second.U + 1;
          assert(l <= c+1);
          assert(u <= r+1);

          if (rect[c][r].find(key) == rect[c][r].end()) {
            if (newEntries.find(key) == newEntries.end())
               newEntries[key] = {l, u};
            else if (l * u > newEntries[key].size())
              newEntries[key] = {l, u};
          }
          else if (l * u > rect[c][r][key].size()) {
            rect[c][r][key] = {l, u};
          }
        }
      }
      if (c-1 >= 0) {
        for (const auto& it2: rect[c-1][r]) {
          int key = it->first & it2.first;
          if (key == 0) continue;

          int l = it2.second.L + 1;
          int u = min(getMaxValue(key, U[c][r]), it2.second.U);
          assert(l <= c+1);
          assert(u <= r+1);

          if (rect[c][r].find(key) == rect[c][r].end()) {
            if (newEntries.find(key) == newEntries.end())
              newEntries[key] = {l, u};
            else if (l * u > newEntries[key].size())
              newEntries[key] = {l, u};
          }
          else if (l * u > rect[c][r][key].size()) {
            rect[c][r][key] = {l, u};
          }
        } 
      }
    }
    rect[c][r].insert(newEntries.begin(), newEntries.end());
  }

  vector<RectInfo> computeRects() {
    vector<RectInfo> res;
    int h = mapper.height(), w = mapper.width();
    for (int r=0; r<h; r++) {
      for (int c=0; c<w; c++) {
        if (c-1 >= 0) updateL(c, r);
        if (r-1 >= 0) updateU(c, r);
      }
    }

    for (int r=0; r<h; r++) {
      for (int c=0; c<w; c++) {
        updateRect(c, r);
        for (const auto& it: rect[c][r])
          res.push_back({r*w + c, it.first, it.second.U, it.second.L});
      }
    }
    return res;
  }
};
