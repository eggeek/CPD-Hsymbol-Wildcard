#include <vector>
#include <iomanip>
#include <sstream>
#include "catch.hpp"
#include "Entry.h"
#include "cpd.h"
#include "mapper.h"
#include "dijkstra.h"
#include "rect_wildcard.h"
#include "jps.h"
using namespace std;

namespace TEST_VISUAL {
  const string default_testcase_path = "./tests/input/test-visual/";
  const string default_map_path = "./tests/maps/";
  string mpath;
  int height, width;
  vector<bool> mapData;

  string mask2hexcolor(int mask) {
    std::stringstream sstream;
    sstream << "#"  << std::setfill('0') << std::setw(6) << std::hex << mask;
    return sstream.str();
  }

  TEST_CASE("pure", "[.fmoves]") {
    xyLoc s;
    int cnt = 0;
    ifstream file(default_testcase_path + "visual.in");

    while (file >> mpath >> s.x >> s.y) {
      ofstream output("visual_" + to_string(cnt) + ".out");
      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      AdjGraph g(extract_graph(mapper));
      Dijkstra dij(g, mapper);
      const auto& fmoves = dij.run(mapper(s), 0);

      string header = "lats,lons,latt,lont,pch,hex,mask";
      // In R plot, pch=0: square, pch=15: solid square
      output << header << endl;
      int lats = s.x, lons = s.y, pch=0;
      for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
          int latt = x, lont = y, color, mask;
          int id = mapper(xyLoc{(int16_t)x, (int16_t)y});
          color = id == -1?0: fmoves[id];
          mask = id == -1?warthog::ALLMOVE: fmoves[id];
          pch = id == -1?15: 0;
          string hexcolor = mask2hexcolor(color);
          output << lats << "," << lons << "," << latt << "," << lont << ","
                 << pch << "," <<  hexcolor << "," << mask << endl;
        }
      }
      cnt++;
    }
  }

  TEST_CASE("rect", "[.fmoves]") {
    xyLoc s;
    int cnt = 0;
    ifstream file(default_testcase_path + "rect.in");
    while (file >> mpath >> s.x >> s.y) {
      int lats = s.x, lons = s.y, pch=0;
      ofstream output("rect_" + to_string(cnt) + ".out");
      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      AdjGraph g(extract_graph(mapper));
      Dijkstra dij(g, mapper);
      const auto& fmoves = dij.run(mapper(s), 0);
      RectWildcard rw(mapper, s, fmoves);
      vector<RectInfo> rects = rw.computeRects();

      for (int r=0; r<height; r++) {
        for (int c=0; c<width; c++) {
          for (const auto& it: rw.L[c][r]) {
            REQUIRE(it.second <= c+1);
            for (int i=0; i<it.second; i++) {
              int id = mapper(xyLoc{(int16_t)(c-i), (int16_t)r});
              int mask = id == -1? warthog::ALLMOVE: fmoves[id];
              REQUIRE((mask & it.first) == it.first);
            }
          }
          for (const auto& it: rw.U[c][r]) {
            REQUIRE(it.second <= r+1);
            for (int i=0; i<it.second; i++) {
              int id = mapper(xyLoc{(int16_t)c, (int16_t)(r-i)});
              int mask = id == -1? warthog::ALLMOVE: fmoves[id];
              REQUIRE((mask & it.first) == it.first);
            }
          }
        }
      }

      vector<vector<bool>> used(height, vector<bool>(width, false));
      auto cmp = [&](const RectInfo& a, const RectInfo& b) {
        return a.L * a.U > b.L * b.U;
      };
      sort(rects.begin(), rects.end(), cmp);

      string header = "lats,lons,latt,lont,pch,hex,mask";
      output << header << endl;
      // In R plot, pch=0: square, pch=15: solid square
      for (const auto& it: rects) {
        int r = it.y(width); 
        int c = it.x(width);
        bool flag = true;
        for (int x=0; x<it.L && flag; x++) {
          for (int y=0; y<it.U; y++) {
            if (used[r-y][c-x]) {
              flag = false;
              break;
            }
          }
        }
        if (flag) {
          for (int x=0; x<it.L; x++) 
          for (int y=0; y<it.U; y++) {
            used[r-y][c-x] = true;
            int latt = c-x, lont = r-y;
            pch = 15;
            string hexcolor = mask2hexcolor(it.mask);
            output << lats << "," << lons << "," << latt << "," << lont << ","
                   << pch << "," << hexcolor << "," << it.mask << endl;
          }
        }
      }

      for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
          if (used[y][x]) continue;
          int latt = x, lont = y, color, mask;
          int id = mapper(xyLoc{(int16_t)x, (int16_t)y});
          color = id == -1?0: fmoves[id];
          mask = id == -1?warthog::ALLMOVE: fmoves[id];
          pch = id == -1?15: 0;
          string hexcolor = mask2hexcolor(mask);
          output << lats << "," << lons << "," << latt << "," << lont << ","
                 << pch << "," <<  hexcolor << "," << mask << endl;
        }
      }
      cnt++;
    }
  }
}
