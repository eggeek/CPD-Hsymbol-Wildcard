#include <stdio.h>
#include <stdint.h>
#include <numeric>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <chrono>
#include <unistd.h>
#include "ScenarioLoader.h"
#include "Timer.h"
#include "Entry.h"
#include "jpsp_oracle.h"
#include "constants.h"

string getMapName(string filename) {
  auto pos = filename.find_last_of('/');
  const string mapfile = filename.substr(pos + 1);
  auto suff = mapfile.find('.');
  return mapfile.substr(0, suff);
}

class Stats {
public:
  double time, cost, twentyMoveTime;
  std::vector<xyLoc> path;

  string to_string() {
    std::ostringstream res;
    res << time << "," << twentyMoveTime << "," << cost << "," << path.size();
    return res.str();
  }
};

void argHelp(char **argv) {
  printf("Invalid Arguments\nUsage %s <flags> <map> <scenario>\n", argv[0]);
  printf("Flags:\n");
  printf("\t-full : Preprocess map and run scenario\n");
  printf("\t-pre : Preprocess map\n");
  printf("\t-run : Run scenario without preprocessing\n");
  printf("\t[hLevel] : int (defualt=1)\n");
  printf("\t\t0: no H symbol\n");
  printf("\t\t1: H symbol with basic heuristic function\n");
  printf("\t\t2: H symbol with improved heuristic function\n");
}

int main(int argc, char **argv)
{
  char filename[255];
  string outfname;
  string mpath, spath;
  std::vector<xyLoc> thePath;
  std::vector<bool> mapData;
  int width, height;
  bool pre = false;
  bool run = false;
  int hLevel = 1;

  if (argc < 4) {
    argHelp(argv);
    exit(0);
  }
  mpath = argv[argc-2];
  spath = argv[argc-1];
  for (int i=1; i<argc-2; i++) {
    if (strcmp(argv[i], "-full") == 0)
    {
      pre = run = true;
    }
    else if (strcmp(argv[i], "-pre") == 0)
    {
      pre = true; run = false;
    }
    else if (strcmp(argv[i], "-run") == 0)
    {
      run = true; pre = false;
    }
    else if (isdigit(argv[i][0])) {
      hLevel = atoi(argv[i]);
    }
    else {
      argHelp(argv);
      exit(0);
    }
  }

  LoadMap(mpath.c_str(), mapData, width, height);
  sprintf(filename, "./index_data/%s.map-%s-%d", getMapName(mpath).c_str() , GetName(), hLevel);

  if (pre)
  {
    PreprocessMap(mapData, width, height, filename, hLevel);
  }
  
  if (!run)
  {
    return 0;
  }
  outfname = "outputs/" + getMapName(filename) + "-" + std::to_string(hLevel) + ".txt";


  void *reference = PrepareForSearch(mapData, width, height, filename);
  warthog::gridmap gm(mpath.c_str());
  warthog::jpsp_oracle oracle(&gm);
  std::cerr << "Sanity Check: "<< (oracle.sanity_check() ? "pass" : "fail") << "\n";

  ScenarioLoader scen(spath.c_str());

  Timer t;
  std::vector<Stats> experimentStats;
  int total_exp = scen.GetNumExperiments();
  experimentStats.resize(total_exp);
  for (int x = 0; x < total_exp; x++) {
    thePath.resize(0);
    double dist = scen.GetNthExperiment(x).GetDistance();
    xyLoc s, g;
    s.x = scen.GetNthExperiment(x).GetStartX();
    s.y = scen.GetNthExperiment(x).GetStartY();
    g.x = scen.GetNthExperiment(x).GetGoalX();
    g.y = scen.GetNthExperiment(x).GetGoalY();

    auto stime = std::chrono::steady_clock::now();
      double pcost = GetPath(reference, s, g, thePath, oracle);
    auto etime = std::chrono::steady_clock::now();
    usleep(1000);

    experimentStats[x].time = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();
    experimentStats[x].cost = pcost;
    for (auto i : thePath)
      experimentStats[x].path.push_back(i);

    thePath.resize(0);
    stime = std::chrono::steady_clock::now();
      GetPath(reference, s, g, thePath, oracle, 20);
    etime = std::chrono::steady_clock::now();
    usleep(1000);

    experimentStats[x].twentyMoveTime = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();

    if (fabs(dist - pcost) > warthog::EPS) {
      printf("experiment: %d, expect: %.5f, actual: %.5f\n", x, dist, pcost);
      assert(false);
    }
  }

  std::ofstream out;
  string header = "map,scenid,time_total,time_20move,distance,path_size,hLevel";
  out.open (outfname.c_str(), std::ios::app);
  out << header << std::endl;
  string mapname = getMapName(string(filename));
  for (unsigned int x = 0; x < experimentStats.size(); x++)
  {
    out << mapname << "," << x << "," <<  experimentStats[x].to_string() << "," << hLevel << std::endl;
  }
  out.close();
  return 0;
}
