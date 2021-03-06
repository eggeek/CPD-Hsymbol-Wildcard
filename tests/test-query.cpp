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
#include "focal.h"
using namespace std;

namespace TEST_QUERY{
  const string default_testcase_path = "./tests/input/test-query/";
  string mpath, spath, indexpath;
  int height, width;
  vector<bool> mapData;

  TEST_CASE("rw-query", "[.run-rw]") {
    ifstream file(default_testcase_path + "rw-query.in");
    while (file >> mpath >> indexpath >> spath) {
      cerr << "Testing: " << indexpath << endl;
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
        Extracter e;
        e.reset(data.graph.node_count());
        c.pathcost = GetRectWildCardCost(data, s, g, data.p.hLevel, c, e);
        REQUIRE(fabs(c.pathcost - dist) <= warthog::EPS);
      }
    }
  }

  TEST_CASE("inv-query", "[.run]") {
    ifstream file(default_testcase_path + "inv-query.in");
    while (file >> mpath >> indexpath >> spath) {
      cerr << "Testing: " << indexpath << endl;
      string output = "inv-query-" + getIndexName(indexpath) + ".out";
      LoadMap(mpath.c_str(), mapData, width, height);
      Index data = LoadIndexData(mapData, width, height, indexpath.c_str());
      ScenarioLoader scens(spath.c_str());

      Extracter e;
      e.init(data.graph.node_count());
      for (int i=0; i<scens.GetNumExperiments(); i++) {
        double dist = scens.GetNthExperiment(i).GetDistance();
        xyLoc s, g;
        s.x = scens.GetNthExperiment(i).GetStartX();
        s.y = scens.GetNthExperiment(i).GetStartY();
        g.x = scens.GetNthExperiment(i).GetGoalX();
        g.y = scens.GetNthExperiment(i).GetGoalY();
        Counter c = Counter{0, 0, 0};
        e.reset(i);
        c.pathcost = GetInvCPDCost(data, s, g, data.p.hLevel, c, e);
        REQUIRE(c.pathcost - dist <= warthog::EPS);
        REQUIRE(c.pathcost - dist > -warthog::EPS);
      }
    }
  }

  TEST_CASE("inv-centroid-query", "[.run]") {
    ifstream file(default_testcase_path + "inv-centroid-query.in");
    while (file >> mpath >> indexpath >> spath) {
      cerr << "Testing: " << indexpath << endl;
      string output = "inv-centroid-" + getIndexName(indexpath) + ".out";
      LoadMap(mpath.c_str(), mapData, width, height);
      Index data = LoadIndexData(mapData, width, height, indexpath.c_str());
      ScenarioLoader scens(spath.c_str());
      Extracter e1, e2;
      e1.init(data.graph.node_count());
      e2.init(data.graph.node_count());
      for (int i=0; i<scens.GetNumExperiments(); i++) {
        double dist = scens.GetNthExperiment(i).GetDistance();
        xyLoc s, g;
        s.x = scens.GetNthExperiment(i).GetStartX();
        s.y = scens.GetNthExperiment(i).GetStartY();
        g.x = scens.GetNthExperiment(i).GetGoalX();
        g.y = scens.GetNthExperiment(i).GetGoalY();
        Counter c = Counter{0, 0, 0};
        e1.reset(i), e2.reset(i);
        c.pathcost = GetInvCentroidCost(data, s, g, data.p.hLevel, c, e1, e2);
        int l = data.p.centroid;
        double ub = 2.0 * l;
        REQUIRE(c.pathcost - dist <= warthog::EPS + ub);
        REQUIRE(c.pathcost - dist >= -warthog::EPS);
      }
    }
  }

  TEST_CASE("forward-centroid-query", "[.run]") {
    ifstream file(default_testcase_path + "forward-centroid-query.in");
    while (file >> mpath >> indexpath >> spath) {
      cerr << "Testing: " << indexpath << endl;
      string output = "forward-centroid-" + getIndexName(indexpath) + ".out";
      LoadMap(mpath.c_str(), mapData, width, height);
      Index data = LoadIndexData(mapData, width, height, indexpath.c_str());
      ScenarioLoader scens(spath.c_str());
      Extracter e1, e2;
      e1.init(data.graph.node_count());
      e2.init(data.graph.node_count());
      for (int i=0; i<scens.GetNumExperiments(); i++) {
        double dist = scens.GetNthExperiment(i).GetDistance();
        xyLoc s, g;
        s.x = scens.GetNthExperiment(i).GetStartX();
        s.y = scens.GetNthExperiment(i).GetStartY();
        g.x = scens.GetNthExperiment(i).GetGoalX();
        g.y = scens.GetNthExperiment(i).GetGoalY();
        Counter c = Counter{0, 0, 0};
        e1.reset(i), e2.reset(i);
        c.pathcost = GetForwardCentroidCost(data, s, g, data.p.hLevel, c, e1, e2);
        int l = data.p.centroid;
        double ub = 2.0 * l;
        REQUIRE(c.pathcost - dist <= warthog::EPS + ub);
        REQUIRE(c.pathcost - dist >= -warthog::EPS);
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
      Extracter e;
      e.init(data.graph.node_count());
      for (int i=0; i<scens.GetNumExperiments(); i++) {
        double dist = scens.GetNthExperiment(i).GetDistance();
        xyLoc s, g;
        s.x = scens.GetNthExperiment(i).GetStartX();
        s.y = scens.GetNthExperiment(i).GetStartY();
        g.x = scens.GetNthExperiment(i).GetGoalX();
        g.y = scens.GetNthExperiment(i).GetGoalY();
        Counter c = Counter{0, 0, 0};
        e.reset(i);
        c.pathcost = GetPathCostSRC(data, s, g, data.p.hLevel, c, e);
        REQUIRE(c.pathcost - dist <= warthog::EPS);
        REQUIRE(c.pathcost - dist >= -warthog::EPS);
      }
    }
  }

  TEST_CASE("focal-search", "[.run]") {
    ifstream file(default_testcase_path + "focal-search.in");
    int L = 0;
    while (file >> mpath >> spath >> L) {
      cerr << "Testing: " << mpath.c_str() << endl;
      string output = "focal-search-" + getMapName(mpath) + ".out";
      LoadMap(mpath.c_str(), mapData, width, height);
      Mapper mapper(mapData, width, height);
      ScenarioLoader scens(spath.c_str());
      AdjGraph g = extract_graph(mapper);
      Focal fs(g, mapper);
      for (int i=0; i<scens.GetNumExperiments(); i++) {
        double dist = scens.GetNthExperiment(i).GetDistance();
        xyLoc s, g;
        s.x = scens.GetNthExperiment(i).GetStartX();
        s.y = scens.GetNthExperiment(i).GetStartY();
        g.x = scens.GetNthExperiment(i).GetGoalX();
        g.y = scens.GetNthExperiment(i).GetGoalY();

        Counter c = Counter{0, 0, 0};
        fs.reset();
        c.pathcost = (double)fs.run(mapper(s), mapper(g), L);
        c.steps = fs.extract_path(mapper(s), mapper(g));
        double diff = c.pathcost - dist;
        REQUIRE(diff <= warthog::EPS + (double)L);
        REQUIRE(diff >= -warthog::EPS);
      }
    }
  }
}
