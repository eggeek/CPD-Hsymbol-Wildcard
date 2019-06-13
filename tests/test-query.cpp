#include <vector>
#include <iomanip>
#include <fstream>
#include "catch.hpp"
#include "ScenarioLoader.h"
#include "preprocessing.h"
#include "Hsymbol.h"
#include "loader.h"
using namespace std;

namespace TEST_PREPROCESS {
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
    const int16_t* dx = warthog::dx;
    const int16_t* dy = warthog::dy;
    double cost = 0.0;

    while (curs != curt) {
      if (data.row_ordering[curs] >= data.row_ordering[curt]) {
      }
      else {
      
      }
    }
    return 0;
  }

  TEST_CASE("rw-query", "[.run]") {
    int cnt = 0, hLevel = 0; 
    ifstream file(default_testcase_path + "rw-query.in");
    while (file >> mpath >> indexpath >> spath >> hLevel) {
      string output = "rw-query" + to_string(cnt) + ".out";
      LoadMap(mpath.c_str(), mapData, width, height);
      Index state = LoadRectWildCard(mapData, width, height, indexpath.c_str());
      ScenarioLoader scens(spath.c_str());
      for (int i=0; i<scens.GetNumExperiments(); i++) {
        double dist = scens.GetNthExperiment(i).GetDistance();
        xyLoc s, g;
        s.x = scens.GetNthExperiment(i).GetStartX();
        s.y = scens.GetNthExperiment(i).GetStartY();
        g.x = scens.GetNthExperiment(i).GetGoalX();
        g.y = scens.GetNthExperiment(i).GetGoalY();
      }
    }
  }
}
