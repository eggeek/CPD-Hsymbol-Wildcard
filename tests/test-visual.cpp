#include <vector>
#include <iomanip>
#include <sstream>
#include "catch.hpp"
#include "cpd.h"
#include "dijkstra.h"
#include "jps.h"
#include "order.h"
#include "loader.h"
#include "centroid.h"
#include "balanced_min_cut.h"
#include "prefer_zero_cut.h"
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

  void print_fmoves(int lats, int lons, 
    const Mapper& mapper, vector<vector<bool>>& flag,
    const vector<unsigned short>& fmoves, ofstream& output,
    int pch=15) {
    for (int y=0; y<height; y++) {
      for (int x=0; x<width; x++) {
        if (flag[y][x]) continue;
        flag[y][x] = true;
        int latt = x, lont = y, color, mask;
        int id = mapper(xyLoc{(int16_t)x, (int16_t)y});
        color = id == -1?0: fmoves[id];
        mask = id == -1?warthog::ALLMOVE: fmoves[id];
        int P = id == -1?pch: 0;
        if (mask & warthog::HMASK) P = 18;
        string hexcolor = mask2hexcolor(mask);
        output << lats << "," << lons << "," << latt << "," << lont << ","
               << P << "," <<  hexcolor << "," << mask << endl;
      }
    }
  }

  void print_obstacle(int lats, int lons,
      const Mapper& mapper, vector<vector<bool>>& flag,
      ofstream& output,
      int pch=15) {
    for (int y=0; y<height; y++)
    for (int x=0; x<width; x++) {
      if (flag[y][x]) continue;
      if (mapper(xyLoc{(int16_t)x, (int16_t)y}) != -1) continue;
      flag[y][x] = true;
      int latt = x, lont = y, mask = 0 ;
      string hexcolor = mask2hexcolor(mask);
      output << lats << "," << lons << "," << latt << "," << lont << ","
             << pch << "," <<  hexcolor << "," << mask << endl;
    }
  }

  void print_square(int lats, int lons, int len, vector<vector<bool>>& flag, ofstream& output,
      int pch=12) {
    for (int y=max(lons-(len-1)/2,0); y<=min(lons+(len-1)/2, height); y++)
    for (int x=max(lats-(len-1)/2,0); x<=min(lats+(len-1)/2, height); x++) {
      if (flag[y][x]) continue;
      flag[y][x] = true;
      int latt = x, lont = y;
      string hexcolor = mask2hexcolor(warthog::HMASK);
      output << lats << "," << lons << "," << latt << "," << lont << ","
            << pch << "," << hexcolor << "," << warthog::HMASK << endl;
    }
  }

  void print_centroids(const vector<int>& centroids, const Mapper& mapper,
      vector<vector<bool>>& flag, ofstream& output, int pch=8) {
    for (int i: centroids) {
      xyLoc loc = mapper(i);
      if (flag[loc.y][loc.x]) continue;
      int latt = loc.x, lont = loc.y;
      string hexcolor = mask2hexcolor(warthog::HMASK);
      output << 0 << "," << 0 << "," << latt << "," << lont << ","
             << pch << "," << hexcolor << "," << 0 << endl;
    }
  }

  void print_rects(int lats, int lons, vector<vector<bool>>& flag,
      const vector<RectInfo>& rects, ofstream& output,
      int pch = 15) {
    for (const auto& it: rects) {
      int r = it.y(width);
      int c = it.x(width);
      for (int x=0; x<it.L; x++)
      for (int y=0; y<it.U; y++) {
        if (flag[r-y][c-x]) continue;
        flag[r-y][c-x] = true;
        int latt = c-x, lont = r-y;
        string hexcolor = mask2hexcolor(it.mask);
        output << lats << "," << lons << "," << latt << "," << lont << ","
               << pch << "," << hexcolor << "," << it.mask << endl;
      }
    }
  }

  void print_entries(int lats, int lons, 
      vector<vector<bool>>& flag,
      const vector<int>& entries,
      const Mapper& mapper, ofstream& output, int pch=21) {

    for (int i: entries) {
      int id = i >> 4;
      int move = i & 0xF;
      xyLoc loc = mapper(id);
      int latt = loc.x, lont = loc.y;
      if (flag[lont][latt]) continue;
      flag[lont][latt] = true;
      string hexcolor = mask2hexcolor(move);
      pch = move;
      output << lats << "," << lons << "," << latt << "," << lont << ","
             << pch << "," <<  hexcolor << "," << move << endl;
    }
  }

  TEST_CASE("pure", "[.fmoves-visual]") {
    xyLoc s;
    int cnt = 0, hLevel;
    ifstream file(default_testcase_path + "pure.in");

    while (file >> mpath >> hLevel >> s.x >> s.y) {
      ofstream output("visual-" + to_string(cnt) + ".out");
      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      AdjGraph g(extract_graph(mapper));
      Dijkstra dij(g, mapper);
      vector<int> sides;
      vector<RectInfo> rects;
      const auto& fmoves = dij.run(mapper(s), hLevel, rects, sides);

      string header = "lats,lons,latt,lont,pch,hex,mask";
      // In R plot, pch=0: square, pch=15: solid square
      output << header << endl;
      int lats = s.x, lons = s.y, pch=0;
      vector<vector<bool>> flag(height+1, vector<bool>(width+1, false));
      print_obstacle(lats, lons, mapper, flag, output);
      print_fmoves(lats,lons, mapper, flag, fmoves, output);
      cnt++;
    }
  }

  TEST_CASE("inv-pure", "[.fmoves-visual]") {
    xyLoc s;
    int cnt = 0, hLevel;
    ifstream file(default_testcase_path + "pure.in");

    while (file >> mpath >> hLevel >> s.x >> s.y) {
      ofstream output("visual-inv-" + to_string(cnt) + ".out");
      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      AdjGraph g(extract_graph(mapper));
      Dijkstra dij(g, mapper);
      vector<int> sides;
      vector<RectInfo> rects;
      dij.run(mapper(s), hLevel, rects, sides);
      const auto& fmoves = dij.get_inv_allowed();

      string header = "lats,lons,latt,lont,pch,hex,mask";
      // In R plot, pch=0: square, pch=15: solid square
      output << header << endl;
      int lats = s.x, lons = s.y, pch=0;
      vector<vector<bool>> flag(height+1, vector<bool>(width+1, false));
      print_obstacle(lats, lons, mapper, flag, output);
      print_fmoves(lats,lons, mapper, flag, fmoves, output);
      cnt++;
    }
  }

  TEST_CASE("rect", "[.fmoves-visual]") {
    xyLoc s;
    int cnt = 0, hLevel = 0;
    ifstream file(default_testcase_path + "rect.in");
    while (file >> mpath >> hLevel >> s.x >> s.y) {
      int lats = s.x, lons = s.y, pch=0;
      ofstream output("rect-" + to_string(cnt) + ".out");
      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      AdjGraph g(extract_graph(mapper));
      Dijkstra dij(g, mapper);
      const auto& fmoves = dij.run(mapper(s), hLevel);
      RectWildcard rw(mapper, s, fmoves);
      vector<RectInfo> rects = rw.computeRects();

      /*
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
      */

      auto cmp = [&](const RectInfo& a, const RectInfo& b) {
        return a.L * a.U > b.L * b.U;
      };
      sort(rects.begin(), rects.end(), cmp);
      vector<vector<bool>> flag(height+1, vector<bool>(width+1, false));

      string header = "lats,lons,latt,lont,pch,hex,mask";
      output << header << endl;
      print_obstacle(lats, lons, mapper, flag, output);
      print_rects(lats, lons, flag, rects, output);
      print_fmoves(lats, lons, mapper, flag, fmoves, output);
      cnt++;
    }
  }


TEST_CASE("rect-used", "[.fmoves-visual]") {
  xyLoc s;
  int cnt = 0, hLevel;
  ifstream file(default_testcase_path + "rect-used.in");
  while (file >> mpath >> hLevel >> s.x >> s.y) {
    int lats = s.x, lons = s.y, pch=0;
    ofstream output("rect-used-" + to_string(cnt) + ".out");
    LoadMap(mpath.c_str(), mapData, width, height);
    Mapper mapper(mapData, width, height);
    NodeOrdering order = compute_real_dfs_order(extract_graph(mapper));
    mapper.reorder(order);
    vector<int> row_ordering = order.get_old_array();
    AdjGraph g(extract_graph(mapper));
    Dijkstra dij(g, mapper);
    vector<RectInfo> rects;
    vector<int> sides;
    const auto& fmoves = dij.run(mapper(s), hLevel, rects, sides);
    CPD cpd;
    vector<RectInfo> used = cpd.append_row(mapper(s), fmoves, mapper, rects, row_ordering, sides.back());
    vector<vector<bool>> flag(height+1, vector<bool>(width+1, false));
    cerr << "#used: " << used.size() << endl;
    cerr << "#fmoves: " << fmoves.size() << ", #cpd: " << cpd.entry_count() << endl;
    string header = "lats,lons,latt,lont,pch,hex,mask";
    output << header << endl;
    print_obstacle(lats, lons, mapper, flag, output);
    print_square(lats, lons, sides.back(), flag, output);
    print_rects(lats, lons, flag, used, output);
    print_fmoves(lats, lons, mapper, flag, fmoves, output, pch=20);
    cnt++;
  }
}

TEST_CASE("rect-cpd-row", "[.cpd-row-visual]") {
  ifstream file(default_testcase_path + "rect-cpd-row.in");
  int hLevel = 0, cnt = 0, pch;
  xyLoc s;
  while (file >> mpath >> hLevel >> s.x >> s.y) {
    int lats = s.x, lons = s.y, pch = 0;
    ofstream output("rect-cpd-row" + to_string(cnt) + ".out");
    LoadMap(mpath.c_str(), mapData, width, height);
    Mapper mapper(mapData, width, height);
    NodeOrdering order = compute_real_dfs_order(extract_graph(mapper));
    mapper.reorder(order);
    vector<int> row_ordering = order.get_old_array();
    AdjGraph g(extract_graph(mapper));
    Dijkstra dij(g, mapper);
    vector<RectInfo> rects;
    vector<int> sides;
    const auto& fmoves = dij.run(mapper(s), hLevel, rects, sides);
    rects.clear();
    rects.push_back({0, 0, 0, 0});
    CPD cpd;
    vector<RectInfo> used = cpd.append_row(mapper(s), fmoves, mapper, rects, row_ordering, sides.back());
    vector<vector<bool>> flag(height, vector<bool>(width, false));
    cerr << "#used: " << used.size() << endl;
    cerr << "#fmoves: " << fmoves.size() << ", #cpd: " << cpd.entry_count() << endl;
    string header = "lats,lons,latt,lont,pch,hex,mask";
    output << header << endl;
    print_entries(lats, lons, flag, cpd.get_entry(), mapper, output);
    print_obstacle(lats, lons, mapper, flag, output);
    print_rects(lats, lons, flag, used, output);
    print_fmoves(lats, lons, mapper, flag, fmoves, output, pch=20);
    cnt++;
  }
}

TEST_CASE("visual-split", "[.cpd-row-visual]") {
  ifstream file(default_testcase_path + "visual-split.in");
  string type;
  int hLevel;
  xyLoc s;
  while (file >> mpath >> type >> hLevel >> s.x >> s.y) {
    int lats = s.x, lons = s.y;
    ofstream output(getMapName(mpath) + "-split-" + type + ".out");
    LoadMap(mpath.c_str(), mapData, width, height);
    Mapper mapper(mapData, width, height);
    NodeOrdering order = compute_split_dfs_order(extract_graph(mapper));
    //NodeOrdering order = compute_cut_order(extract_graph(mapper), prefer_zero_cut(balanced_min_cut));
    mapper.reorder(order);
    AdjGraph g(extract_graph(mapper));
    Dijkstra dij(g, mapper);
    vector<int> sides;
    dij.run(mapper(s), hLevel, sides);
    int raw_hcnt = 0, hrun = 0;
    CPD cpd;
    if (type == "inv") {
      for (auto i: dij.get_inv_allowed()) if (i & warthog::HMASK) raw_hcnt++;
      cpd.append_row(mapper(s), dij.get_inv_allowed(), mapper, 0);
      for (auto i: cpd.get_entry())
        if ( (1<<(i&0xF) == warthog::HMASK) )
          hrun++;
    }
    else {
      for (auto i: dij.get_allowed()) if (i & warthog::HMASK) raw_hcnt++;
      cpd.append_row(mapper(s), dij.get_allowed(), mapper, 0);
      for (auto i: cpd.get_entry())
        if ( (1<<(i&0xF) == warthog::HMASK) )
          hrun++;
    }
    cerr << "#raw_h: " << raw_hcnt << ", #hrun:" << hrun << ", hlevel: " << hLevel << endl;
    vector<vector<bool>> flag(height, vector<bool>(width, false));
    cerr << "#size: " << dij.get_allowed().size() << ", #cpd: " << cpd.entry_count() << endl;
    string header = "lats,lons,latt,lont,pch,hex,mask";
    output << header << endl;
    print_entries(lats, lons, flag, cpd.get_entry(), mapper, output, 8);
    print_obstacle(lats, lons, mapper, flag, output, 15);
  }
}

TEST_CASE("visual-extra", "[.cpd-row-visual]") {
  ifstream file(default_testcase_path + "visual-extra.in");
  string type;
  int hLevel;
  xyLoc s;
  while (file >> mpath >> type >> hLevel >> s.x >> s.y) {
    int lats = s.x, lons = s.y;
    ofstream output(getMapName(mpath) + "-extra-" + type + ".out");
    LoadMap(mpath.c_str(), mapData, width, height);
    Mapper mapper(mapData, width, height);
    //NodeOrdering order = compute_real_dfs_order(extract_graph(mapper));
    NodeOrdering order = compute_cut_order(extract_graph(mapper), prefer_zero_cut(balanced_min_cut));
    mapper.reorder(order);
    AdjGraph g(extract_graph(mapper));
    Dijkstra dij(g, mapper);
    dij.run_extra(mapper(s), hLevel);
    int raw_hcnt = 0, hrun = 0, extra_cnt = 0;
    CPD cpd;
    if (type == "inv") {
      for (auto i: dij.get_inv_allowed()) if (i & warthog::HMASK) raw_hcnt++;
      for (int i=0; i<mapper.node_count(); i++) if (i != mapper(s)) {
        int mask = dij.get_inv_allowed()[i];
        while (mask) {
          int move = warthog::m2i.at(warthog::lowb(mask));
          if (!(mapper.get_neighbor(i) & (1<<move))) extra_cnt++;
          mask -= warthog::lowb(mask);
        }
      }
      cpd.append_row(mapper(s), dij.get_inv_allowed(), mapper, 0);
      for (auto i: cpd.get_entry())
        if ( (1<<(i&0xF) == warthog::HMASK) )
          hrun++;
    }
    else {
      for (auto i: dij.get_allowed()) if (i & warthog::HMASK) raw_hcnt++;
      cpd.append_row(mapper(s), dij.get_allowed(), mapper, 0);
      for (auto i: cpd.get_entry())
        if ( (1<<(i&0xF) == warthog::HMASK) )
          hrun++;
    }
    cerr << "#raw_h: " << raw_hcnt << ", #hrun:" << hrun << ", hlevel: " << hLevel << endl;
    vector<vector<bool>> flag(height, vector<bool>(width, false));
    cerr << "#size: " << dij.get_allowed().size() << ", #cpd: " << cpd.entry_count() << endl;
    cerr << "#extra_cnt: " << extra_cnt << endl;
    string header = "lats,lons,latt,lont,pch,hex,mask";
    output << header << endl;
    print_entries(lats, lons, flag, cpd.get_entry(), mapper, output, 8);
    print_obstacle(lats, lons, mapper, flag, output, 15);
  }
}

TEST_CASE("visual-centroid", "[.cpd-centroid-visual]") {
  ifstream file(default_testcase_path + "visual-centroid.in");
  while (file >> mpath) {
    ofstream output(getMapName(mpath) + "-centroids.out") ;
    LoadMap(mpath.c_str(), mapData, width, height);
    Mapper mapper(mapData, width, height);
    vector<int> cents = compute_centroid(mapper, 7);
    vector<vector<bool>> flag(height, vector<bool>(width, false));

    string header = "lats,lons,latt,lont,pch,hex,mask";
    output << header << endl;
    print_obstacle(0, 0, mapper, flag, output, 15);
    print_centroids(cents, mapper, flag, output);
  }
}

TEST_CASE("inspect-runs", "[.cpd]") {
  ifstream file(default_testcase_path + "inspect-runs.in");
  int hLevel = 0;
  string type, index_path;
  while (file >> mpath >> index_path >> type >> hLevel) {
    ofstream output(getMapName(mpath) + "-" + to_string(hLevel) + "-" + type + "-inspect.out");
    LoadMap(mpath.c_str(), mapData, width, height);
    Index data;
    if (type == "rect")
      data = LoadRectWildCard(mapData, width, height, index_path.c_str());
    else if (type == "inv")
      data = LoadInvCPD(mapData, width, height, index_path.c_str());
    else 
      data = LoadVanillaCPD(mapData, width, height, index_path.c_str());
    string header = "row,runs,hrun,hcnt,map";
    output << header << endl;
    for (int i=0; i<data.mapper.node_count(); i++) {
      int runs = data.cpd.get_begin()[i+1] - data.cpd.get_begin()[i];
      int hcnt = data.cpd.get_heuristic_cnt(i);
      int hrun = data.cpd.get_heuristic_run(i);
      output << i << "," << runs << "," << hrun << "," << hcnt<< "," << getMapName(mpath) << endl;
    }
  }
}

TEST_CASE("inspect-row", "[.cpd]") {
  ifstream file(default_testcase_path + "inspect-row.in");
  int hLevel = 0;
  xyLoc s;
  string type, index_path, outpath;
  while (file >> mpath >> index_path >> type >> hLevel >> s.x >> s.y) {
    ofstream output("visual-row-" + getIndexName(index_path) + ".out");
    LoadMap(mpath.c_str(), mapData, width, height);
    Index data;
    if (type == "rect")
      data = LoadRectWildCard(mapData, width, height, index_path.c_str());
    else if (type == "inv")
      data = LoadInvCPD(mapData, width, height, index_path.c_str());
    else 
      data = LoadVanillaCPD(mapData, width, height, index_path.c_str());

    vector<vector<bool>> flag(height, vector<bool>(width, false));
    string header = "lats,lons,latt,lont,pch,hex,mask";
    auto entry = data.cpd.get_entry();
    auto begin = data.cpd.get_begin();
    int id = data.mapper(s);
    cerr << "#cpd: " << begin[id+1] - begin[id] << endl;
    output << header << endl;
    print_entries(s.x, s.y, flag, vector<int>(entry.begin()+begin[id], entry.begin()+begin[id+1]), data.mapper, output, 8);
    print_obstacle(s.x, s.y, data.mapper, flag, output);
  }
}

}
