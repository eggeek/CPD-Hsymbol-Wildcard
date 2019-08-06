#include <vector>
#include <iomanip>
#include <fstream>
#include "catch.hpp"
#include "ScenarioLoader.h"
#include "preprocessing.h"
#include "Hsymbol.h"
#include "loader.h"
#include "query.h"
#include "dijkstra.h"
using namespace std;

namespace TEST_QUERY{
  const string default_testcase_path = "./tests/input/test-query/";
  string mpath, spath, indexpath;
  int height, width;
  vector<bool> mapData;

  TEST_CASE("rw-query", "[.run]") {
    ifstream file(default_testcase_path + "rw-query.in");
    while (file >> mpath >> indexpath >> spath) {
      string output = "rw-query" + getIndexName(indexpath) + ".out";
      LoadMap(mpath.c_str(), mapData, width, height);
      Index data = LoadIndexData(mapData, width, height, indexpath.c_str());
      ScenarioLoader scens(spath.c_str());
      for (int i=0; i<scens.GetNumExperiments(); i++) {
        double dist = scens.GetNthExperiment(i).GetDistance();
        xyLoc s, g;
        s.x = scens.GetNthExperiment(i).GetStartX();
        s.y = scens.GetNthExperiment(i).GetStartY();
        g.x = scens.GetNthExperiment(i).GetGoalX();
        g.y = scens.GetNthExperiment(i).GetGoalY();
        Counter c = Counter{0, 0, 0};
        c.pathcost = GetRectWildCardCost(data, s, g, data.p.hLevel, c);
        REQUIRE(fabs(c.pathcost - dist) <= warthog::EPS);
      }
    }
  }

  TEST_CASE("inv-query", "[.run]") {
    ifstream file(default_testcase_path + "inv-query.in");
    while (file >> mpath >> indexpath >> spath) {
      string output = "inv-query-" + getIndexName(indexpath) + ".out";
      LoadMap(mpath.c_str(), mapData, width, height);
      Index data = LoadIndexData(mapData, width, height, indexpath.c_str());
      ScenarioLoader scens(spath.c_str());
      for (int i=0; i<scens.GetNumExperiments(); i++) {
        double dist = scens.GetNthExperiment(i).GetDistance();
        xyLoc s, g;
        s.x = scens.GetNthExperiment(i).GetStartX();
        s.y = scens.GetNthExperiment(i).GetStartY();
        g.x = scens.GetNthExperiment(i).GetGoalX();
        g.y = scens.GetNthExperiment(i).GetGoalY();
        Counter c = Counter{0, 0, 0};
        c.pathcost = GetInvCPDCost(data, s, g, data.p.hLevel, c);
        REQUIRE(fabs(c.pathcost - dist) <= warthog::EPS);
      }
    }
  }

  TEST_CASE("inv-centroid-query", "[.run]") {
    ifstream file(default_testcase_path + "inv-centroid-query.in");
    while (file >> mpath >> indexpath >> spath) {
      string output = "inv-centroid-" + getIndexName(indexpath) + ".out";
      LoadMap(mpath.c_str(), mapData, width, height);
      Index data = LoadIndexData(mapData, width, height, indexpath.c_str());
      ScenarioLoader scens(spath.c_str());
      for (int i=0; i<scens.GetNumExperiments(); i++) {
        double dist = scens.GetNthExperiment(i).GetDistance();
        xyLoc s, g;
        s.x = scens.GetNthExperiment(i).GetStartX();
        s.y = scens.GetNthExperiment(i).GetStartY();
        g.x = scens.GetNthExperiment(i).GetGoalX();
        g.y = scens.GetNthExperiment(i).GetGoalY();
        Counter c = Counter{0, 0, 0};
        c.pathcost = GetInvCentroidCost(data, s, g, data.p.hLevel, c);
        int l = data.p.centroid;
        double ub = 2.0 * l;
        REQUIRE(fabs(c.pathcost - dist) <= warthog::EPS + ub);
      }
    }
  }

  TEST_CASE("forward-centroid-query", "[.run]") {
    ifstream file(default_testcase_path + "forward-centroid-query.in");
    while (file >> mpath >> indexpath >> spath) {
      string output = "forward-centroid-" + getIndexName(indexpath) + ".out";
      LoadMap(mpath.c_str(), mapData, width, height);
      Index data = LoadIndexData(mapData, width, height, indexpath.c_str());
      ScenarioLoader scens(spath.c_str());
      for (int i=0; i<scens.GetNumExperiments(); i++) {
        double dist = scens.GetNthExperiment(i).GetDistance();
        xyLoc s, g;
        s.x = scens.GetNthExperiment(i).GetStartX();
        s.y = scens.GetNthExperiment(i).GetStartY();
        g.x = scens.GetNthExperiment(i).GetGoalX();
        g.y = scens.GetNthExperiment(i).GetGoalY();
        Counter c = Counter{0, 0, 0};
        c.pathcost = GetForwardCentroidCost(data, s, g, data.p.hLevel, c);
        int l = data.p.centroid;
        double ub = 2.0 * l;
        REQUIRE(fabs(c.pathcost - dist) <= warthog::EPS + ub);
      }
    }
  }

  TEST_CASE("normal-query", "[.run]") {
    using CPD = CPDBASE;
    ifstream file(default_testcase_path + "normal-query.in");
    while (file >> mpath >> indexpath >> spath) {
      cerr << "Testing: " << indexpath << endl;
      string output = "normal-query-" + getIndexName(indexpath) + ".out";
      LoadMap(mpath.c_str(), mapData, width, height);
      Index data = LoadIndexData(mapData, width, height, indexpath.c_str());
      CPD cpd;
      ScenarioLoader scens(spath.c_str());
      for (int i=0; i<scens.GetNumExperiments(); i++) {
        double dist = scens.GetNthExperiment(i).GetDistance();
        xyLoc s, g;
        s.x = scens.GetNthExperiment(i).GetStartX();
        s.y = scens.GetNthExperiment(i).GetStartY();
        g.x = scens.GetNthExperiment(i).GetGoalX();
        g.y = scens.GetNthExperiment(i).GetGoalY();
        Counter c = Counter{0, 0, 0};
        c.pathcost = GetPathCostSRC(data, s, g, data.p.hLevel, c);
        REQUIRE(fabs(c.pathcost - dist) <= warthog::EPS);
      }
    }
  }
}
