#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <assert.h>
#include <numeric>
#include "focal.h"
#include "loader.h"
#include "ScenarioLoader.h"
#include "mapper.h"
#include "query.h"
using namespace std;

class Stats {
public:
  std::vector<double> srcTime;// = std::vector<double>(3);
  Counter c;
  string to_string(double expect) {
    std::ostringstream res;
    sort(srcTime.begin(), srcTime.end());
    res << std::accumulate(srcTime.begin(), srcTime.end(), 0.0) / 5.0 << "," << c.pathcost << "," << expect << "," << c.steps;
    return res.str();
  }
};

string mpath, spath;
int width, height, L;
vector<bool> mapData;
ScenarioLoader scens;
Mapper mapper;
AdjGraph g;
vector<Stats> exps;

void runExperiment() {
  Focal focal(g, mapper);
  for (int i=0; i<scens.GetNumExperiments(); i++) {
    double dist = scens.GetNthExperiment(i).GetDistance();
    xyLoc s, g;
    s.x = scens.GetNthExperiment(i).GetStartX();
    s.y = scens.GetNthExperiment(i).GetStartY();
    g.x = scens.GetNthExperiment(i).GetGoalX();
    g.y = scens.GetNthExperiment(i).GetGoalY();
    exps[i].c = Counter{0, 0, 0};

    focal.reset();
    auto stime = std::chrono::steady_clock::now();
      exps[i].c.pathcost = focal.run(mapper(s), mapper(g), L);
      exps[i].c.steps = focal.extract_path(mapper(s), mapper(g));
    auto etime = std::chrono::steady_clock::now();
    double tcost = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();
    exps[i].srcTime.push_back(tcost);
    if (fabs(exps[i].c.pathcost - dist) > warthog::EPS + (double)L) {
      printf("experiment: %d, expect: %.5f, actual: %.5f\n", i, dist, exps[i].c.pathcost);
      assert(false);
    }
  }
}


int main(int argc, char ** argv) {
  if (argc != 4) {
    cerr << "usage: ./exe <mpath> <spath> L" << endl;
    exit(1);
  }
  mpath = string(argv[1]);
  spath = string(argv[2]);
  L = atoi(argv[3]);
  LoadMap(mpath.c_str(), mapData, width, height);
  scens = ScenarioLoader(spath.c_str());
  exps.resize(scens.GetNumExperiments());
  mapper = Mapper(mapData, width, height);
  g = extract_graph(mapper);

  int repeat = 5;
  for (int i=0; i<repeat; i++)
    runExperiment();

  std::ofstream out;  
  out.open("outputs/focal/" + getMapName(mpath) + "-" + to_string(L) + ".txt", std::ios::app);
  string header = "map,scenid,tcost,distance,expect,steps,r";
  out << header << endl;
  for (int i=0; i<(int)exps.size(); i++) {
    double expect = scens.GetNthExperiment(i).GetDistance();
    out << getMapName(mpath) << "," << i << "," << exps[i].to_string(expect) << "," << L << endl;
  }
  return 0;
}
