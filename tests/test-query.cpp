#include <vector>
#include <iomanip>
#include <fstream>
#include "catch.hpp"
#include "ScenarioLoader.h"
#include "preprocessing.h"
#include "Hsymbol.h"
#include "loader.h"
#include "dijkstra.h"
using namespace std;

namespace TEST_QUERY{
  const string default_testcase_path = "./tests/input/test-query/";
  string mpath, spath, indexpath;
  int height, width;
  vector<bool> mapData;

  double getpath(const Index& data, xyLoc s, xyLoc t, int hLevel) {
    int (*heuristic_func)(int, int, const Mapper&);
    if (hLevel == 1)
      heuristic_func = Hsymbol::get_heuristic_move1;
    else if (hLevel == 2)
      heuristic_func = Hsymbol::get_heuristic_move2;
    else if (hLevel == 3)
      heuristic_func = Hsymbol::get_heuristic_move3;

    int curs = data.mapper(s);
    int curt = data.mapper(t);
    int move;
    const int16_t* dx = warthog::dx;
    const int16_t* dy = warthog::dy;
    double cost = 0.0;

    auto next_move = [&](int current_source, int current_target)
    {
        xyLoc cs = data.mapper.operator()(current_source);
        xyLoc ct = data.mapper.operator()(current_target);
        int vx = signbit(ct.x - cs.x);
        int vy = signbit(ct.y - cs.y);
        return warthog::v2i[1+vx][1+vy];
    };

    auto is_in_square = [&](int current_source, int current_target)
    {
        int side = data.square_sides[current_source];
        xyLoc loc_source = data.mapper.operator()(current_source);
        xyLoc loc_x = data.mapper.operator()(current_target);
        int dx = iabs(loc_source.x - loc_x.x);
        int dy = iabs(loc_source.y- loc_x.y);
        if(( (dx<<1) <= (side-1)) && ( (dy<<1) <= (side-1)))
        {
          return true;
        }
        return false;
    };

    auto to_next_pos = [&](xyLoc& source, xyLoc& target, int& sid, int& tid) {

      if (is_in_square(sid, tid)) {
        move = next_move(sid, tid);
        if ((1<<move) == warthog::HMASK) {
          move = Hsymbol::decode(sid, tid, data.mapper, heuristic_func);
        }
      }
      else {
        const RectInfo* rect = data.rwobj.get_rects(sid, data.mapper, target);

        Dijkstra dij(data.graph, data.mapper);
        CPD cpd;
        vector<RectInfo> rects;
        vector<int> sides;
        vector<unsigned short> fmoves = dij.run(data.mapper(source), hLevel, rects, sides);
        vector<RectInfo> used = cpd.append_row(sid, fmoves, data.mapper, rects, data.row_ordering, sides.back());
        if (rect != NULL) {
          bool flag = false;
          for (const auto& it: used) {
            if (it.mask == rect->mask && it.pos == rect->pos && it.U == rect->U && it.L == rect->L) {
              flag = true;
              break;
            }
          }
          if (flag != true) {
            rect = data.rwobj.get_rects(sid, data.mapper, target);
          }
          REQUIRE(flag == true);
        }

        if (rect == NULL) {
          move = data.cpd.get_first_move(sid, tid);
          int move2 = cpd.get_first_move(0, tid);
          REQUIRE(move == move2);
        }
        else {
          move = rect->mask? warthog::m2i.at(warthog::lowb(rect->mask)): warthog::NOMOVE;
        }
        if (move == warthog::NOMOVE) return;
        if ((1<<move) == warthog::HMASK) {
          move = Hsymbol::decode(sid, tid, data.mapper, heuristic_func);
        }
        REQUIRE((fmoves[tid] & (1<<move)) > 0);
      }

      cost += warthog::doublew[move];
      source.x += dx[move];
      source.y += dy[move];
      sid = data.mapper(source);
    };

    while (curs != curt) {
      if (data.row_ordering[curs] >= data.row_ordering[curt]) {
        to_next_pos(s, t, curs, curt);
        if (move == warthog::NOMOVE) break;
      }
      else {
        to_next_pos(t, s, curt, curs); 
        if (move == warthog::NOMOVE) break;
      }
    }
    return cost;
  }

  TEST_CASE("rw-query", "[.run]") {
    int cnt = 0, hLevel = 0; 
    ifstream file(default_testcase_path + "rw-query.in");
    while (file >> mpath >> indexpath >> spath >> hLevel) {
      string output = "rw-query" + to_string(cnt) + ".out";
      LoadMap(mpath.c_str(), mapData, width, height);
      Index data = LoadRectWildCard(mapData, width, height, indexpath.c_str());
      ScenarioLoader scens(spath.c_str());
      for (int i=0; i<scens.GetNumExperiments(); i++) {
        double dist = scens.GetNthExperiment(i).GetDistance();
        xyLoc s, g;
        s.x = scens.GetNthExperiment(i).GetStartX();
        s.y = scens.GetNthExperiment(i).GetStartY();
        g.x = scens.GetNthExperiment(i).GetGoalX();
        g.y = scens.GetNthExperiment(i).GetGoalY();
        double pathcost = getpath(data, s, g, hLevel);
        REQUIRE(fabs(pathcost - dist) <= warthog::EPS);
      }
    }
  }
}
