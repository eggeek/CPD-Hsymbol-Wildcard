#include <vector>
#include <iomanip>
#include <sstream>
#include "catch.hpp"
#include "dijkstra.h"
#include "jps.h"
#include "order.h"
#include "loader.h"
#include "centroid.h"
#include "balanced_min_cut.h"
#include "prefer_zero_cut.h"
#include "cpd_base.h"
#include "cpd_rect.h"
using namespace std;

namespace GEN_VISUAL {
  const string default_testcase_path = "./tests/input/test-visual/";
  const string default_map_path = "./tests/maps/";
  const string header = "rowid,x,y,order,cid,mask";
  string mpath;
  int height, width;
  vector<bool> mapData;

  void print_obstacle(const Mapper& mapper, vector<vector<bool>>& flag, ofstream& output) {
    for (int y=0; y<height; y++)
    for (int x=0; x<width; x++) {
      if (flag[y][x]) continue;
      if (mapper(xyLoc{(int16_t)x, (int16_t)y}) != -1) continue;
      flag[y][x] = true;
      output << -1 << "," << x << "," << y << "," << -1 << ",-1,0" << endl;
    }
  }

  void print_entries(int rowid, vector<vector<bool>>& flag,
      const vector<int>& entries,
      const Mapper& mapper, ofstream& output) {

    for (int i: entries) {
      int id = i >> 4;
      int move = i & 0xF;
      xyLoc loc = mapper(id);
      int tx = loc.x, ty = loc.y;
      if (flag[ty][tx]) continue;
      flag[ty][tx] = true;
      int cid = mapper.get_fa().empty()? -1: mapper.get_fa()[i];
      output << rowid << "," << tx << "," << ty << "," << i << "," << cid << "," << move << endl;
    }
  }

  void print_centroids(int rowid, vector<int>& cents, const Mapper& mapper, 
      vector<vector<bool>>& flag, ofstream& output) {
    for (int i: cents) {
      xyLoc loc = mapper(i);
      if (flag[loc.y][loc.x]) continue;
      flag[loc.y][loc.x] = true;
      int cid = mapper.get_fa()[i];
      output << rowid << "," << loc.x << "," << loc.y << "," << i << "," << cid << "," << -1 << endl;
    }
  }

  void print_fmoves(int rowid, const Mapper& mapper, 
      vector<vector<bool>>& flag, 
      const vector<unsigned short>& fmoves,
      ofstream& output) {
    for (int i=0; i<mapper.node_count(); i++) {
      xyLoc loc = mapper(i);
      if (flag[loc.y][loc.x]) continue;
      flag[loc.y][loc.x] = true;
      int cid = mapper.get_fa().empty()?-1: mapper.get_fa()[i];
      output << rowid << "," << loc.x << "," << loc.y << "," << i << "," << cid << "," << fmoves[i] << endl;
    }
  }

  TEST_CASE("visual-fmoves", "[.fmoves-visual]") {
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
      const auto& fmoves = dij.run(mapper(s), hLevel, sides);
      output << header << endl;
      vector<vector<bool>> flag(height+1, vector<bool>(width+1, false));
      print_obstacle(mapper, flag, output);
      print_fmoves(mapper(s), mapper, flag, fmoves, output);
      cnt++;
    }
  }

  TEST_CASE("visual-centroid", "[.centroid-visual]") {
    ifstream file(default_testcase_path + "visual-centroid.in");
    int sizec;
    while (file >> mpath >> sizec) {
      ofstream output(getMapName(mpath) + "-centroids.out") ;
      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      vector<int> cents = compute_centroid(mapper, sizec);
      vector<vector<bool>> flag(height, vector<bool>(width, false));
      output << header << endl;
      print_obstacle(mapper, flag, output);
      print_centroids(0, cents, mapper, flag, output);
      for (int i=0; i<mapper.node_count(); i++) {
        xyLoc loc = mapper(i);
        output << "0," << loc.x << "," << loc.y << "," << i << "," << mapper.get_fa()[i] << ",0" << endl;
      }
    }
  }

TEST_CASE("inspect-runs", "[.cpd-statistic]") {
  ifstream file(default_testcase_path + "inspect-runs.in");
  string index_path;
  while (file >> mpath >> index_path) {
    LoadMap(mpath.c_str(), mapData, width, height);
    Index data = LoadIndexData(mapData, width, height, index_path.c_str());
    ofstream output(
        getMapName(mpath) + "-" + 
        (data.p.centroid?"subopt": "opt") + "-" + 
        data.p.itype + "-inspect.out");
    string header = "row,runs,hrun,hcnt,crun,ccnt,cx,cy,x,y,map";
    output << header << endl;
    for (int i=0; i<data.cpd.node_count(); i++) {
      int runs = data.cpd.get_begin()[i+1] - data.cpd.get_begin()[i];
      int hcnt = data.cpd.get_heuristic_cnt(i);
      int hrun = data.cpd.get_heuristic_run(i);
      int crun = data.cpd.get_centroid_run(i);
      int ccnt = data.cpd.get_centroid_cnt(i);
      int cid = data.mapper.get_fa()[i];
      xyLoc loc = data.mapper(i);
      xyLoc c = data.mapper(cid);
      output << i << "," << runs << "," << hrun << "," << hcnt<< "," 
             << crun << "," << ccnt << "," << c.x << "," << c.y << "," 
             << loc.x << "," << loc.y << "," << getMapName(mpath) << endl;
    }
  }
}

TEST_CASE("inspect-row", "[.cpd-inspect]") {
  ifstream file(default_testcase_path + "inspect-row.in");
  xyLoc s;
  string index_path, outpath;
  while (file >> mpath >> index_path >> s.x >> s.y) {
    ofstream output("visual-row-" + getIndexName(index_path) + ".out");
    LoadMap(mpath.c_str(), mapData, width, height);
    Index data = LoadIndexData(mapData, width, height, index_path.c_str());
    int sid = data.mapper(s);
    vector<vector<bool>> flag(height, vector<bool>(width, false));
    auto entry = data.cpd.get_entry();
    auto begin = data.cpd.get_begin();
    int id = data.mapper(s);
    cerr << "#cpd: " << begin[id+1] - begin[id] << endl;
    output << header << endl;
    print_entries(sid, flag, vector<int>(entry.begin()+begin[id], entry.begin()+begin[id+1]), data.mapper, output);
    for (int i=0; i<data.mapper.node_count(); i++) {
      xyLoc loc = data.mapper(i);
      int move = data.cpd.get_first_move(sid, i);
      //int hmove = Hsymbol::get_heuristic_move(sid, i, data.mapper, 3);
      int mask = 1<<move;
      int cid = data.p.centroid? data.mapper.get_fa()[i]: -1;
      output << sid << "," << loc.x << "," << loc.y << "," << i << ","
             << cid << "," << mask << endl;
    }
    print_obstacle(data.mapper, flag, output);
  }
}

TEST_CASE("inspect-rev-centroid-row", "[.cpd-inspect]") {
  ifstream file(default_testcase_path + "inspect-rev-centroid-row.in");
  xyLoc s;
  string index_path, outpath;
  while (file >> mpath >> index_path >> s.x >> s.y) {
    ofstream output("visual-row-" + getIndexName(index_path) + ".out");
    LoadMap(mpath.c_str(), mapData, width, height);
    Index data = LoadIndexData(mapData, width, height, index_path.c_str());
    int sid = data.mapper(s);
    int cid = data.mapper.get_fa()[sid];
    vector<vector<bool>> flag(height, vector<bool>(width, false));
    auto entry = data.cpd.get_entry();
    auto begin = data.cpd.get_begin();
    int rid = data.mapper.get_centroid_rank(cid);
    cerr << "#cpd: " << begin[rid+1] - begin[rid] << endl;
    output << header << endl;
    for (int i=0; i<data.mapper.node_count(); i++) {
      xyLoc loc = data.mapper(i);
      if (abs(loc.x - s.x) + abs(loc.y - s.y) > 5 * data.p.centroid)
        continue;
      int move;
      if (i == data.mapper.get_fa()[i]) {
        move = data.cpd.get_first_move(data.mapper.get_centroid_rank(i), sid);
      }
      else move = data.cpd.get_first_move(data.mapper.get_centroid_rank(data.mapper.get_fa()[i]), sid);
      //int hmove = Hsymbol::get_heuristic_move(sid, i, data.mapper, 3);
      int mask = 1<<move;
      cid = data.mapper.get_fa()[i];
      output << sid << "," << loc.x << "," << loc.y << "," << i << ","
             << cid << "," << mask << endl;
    }
    print_obstacle(data.mapper, flag, output);
  }
}

TEST_CASE("inspect-fwd-centroid-symbol") {
  ifstream file(default_testcase_path + "inspect-fwd-centroid-symbol.in");
  xyLoc s;
  int centroid;
  string outpath;
  while (file >> mpath >> centroid >> s.x >> s.y) {
    ofstream output("fwd-centroid-symbol-" + getMapName(mpath) + "-c" + to_string(centroid) + ".out");
    LoadMap(mpath.c_str(), mapData, width, height);
    Mapper mapper(mapData, width, height);
    printf("computing centroid.\n");
    vector<int> cents = compute_centroid(mapper, centroid);
    AdjGraph g(extract_graph(mapper));
    int sid = mapper(s);
    int cid = mapper.get_fa()[sid];
    vector<vector<bool>> flag(height+1, vector<bool>(width+1, false));


    output << header << endl;

    Dijkstra dij(g, mapper);
    vector<unsigned short> fc, fs;

    dij.run(cid, 3);
    fc = dij.get_allowed();

    dij.run(sid, 3);
    fs = dij.get_allowed();

    for (int i=0; i<g.node_count(); i++) if (i != sid && i != cid) {
      int mask = fs[i] & fc[i];
      xyLoc loc = mapper(i);
      if (mask)
        output << sid << "," << loc.x << "," << loc.y << "," << i << "," << cid << "," << mask << endl;
    }
    print_obstacle(mapper, flag, output);
  }
}

}
