#include <stdio.h>
#include <stdint.h>
#include <numeric>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <chrono>
#include <unistd.h>
#include "ScenarioLoader.h"
#include "Entry.h"
#include "jpsp_oracle.h"
#include "constants.h"
#include "timer.h"

class Stats {
public:
  std::vector<double> time = std::vector<double>(3);
  std::vector<double> time20moves = std::vector<double>(3);
  std::vector<double> srcTime = std::vector<double>(3);
  std::vector<double> srcTime20moves = std::vector<double>(3);
  double pathcost;
  std::vector<xyLoc> path;

  string to_string() {
    std::ostringstream res;
    sort(time.begin(), time.end());
    sort(time20moves.begin(), time20moves.end());
    sort(srcTime.begin(), srcTime.end());
    sort(srcTime20moves.begin(), srcTime20moves.end());
    // get the mean
    res << time[time.size() / 2] << ",";
    res << time20moves[time20moves.size() / 2] << ",";
    res << srcTime[srcTime.size() / 2] << ",";
    res << srcTime20moves[srcTime20moves.size() / 2] << ",";
    res << pathcost << "," << path.size();
    return res.str();
  }
};

string getMapName(string filename) {
  auto pos = filename.find_last_of('/');
  const string mapfile = filename.substr(pos + 1);
  auto suff = mapfile.find('.');
  return mapfile.substr(0, suff);
}

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

void GetExperimentsSRCTime(void* ref, ScenarioLoader& scen, std::vector<Stats>& exps, int hLevel) {
  warthog::timer t;
  for (int x=0; x<scen.GetNumExperiments(); x++) {
    double dist = scen.GetNthExperiment(x).GetDistance();
    xyLoc s, g;
    s.x = scen.GetNthExperiment(x).GetStartX();
    s.y = scen.GetNthExperiment(x).GetStartY();
    g.x = scen.GetNthExperiment(x).GetGoalX();
    g.y = scen.GetNthExperiment(x).GetGoalY();
    auto stime = std::chrono::steady_clock::now();
      double pathcost = GetPathCostSRC(ref, s, g, hLevel);
    auto etime = std::chrono::steady_clock::now();
    double tcost = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();
    //t.start();
    //  double pathcost = GetPathCostSRC(ref, s, g);
    //t.stop();
    //double tcost = t.elapsed_time_nano();
    exps[x].srcTime.push_back(tcost);
    if (fabs(pathcost - dist) > warthog::EPS) {
      printf("experiment: %d, expect: %.5f, actual: %.5f\n", x, dist, pathcost);
      assert(false);
    }
  }
}

void GetExperimentsSRCTime20Moves(void* ref, ScenarioLoader& scen, std::vector<Stats>& exps, int hLevel) {
  warthog::timer t;
  for (int x=0; x<scen.GetNumExperiments(); x++) {
    xyLoc s, g;
    s.x = scen.GetNthExperiment(x).GetStartX();
    s.y = scen.GetNthExperiment(x).GetStartY();
    g.x = scen.GetNthExperiment(x).GetGoalX();
    g.y = scen.GetNthExperiment(x).GetGoalY();
    auto stime = std::chrono::steady_clock::now();
      GetPathCostSRC(ref, s, g, hLevel, 20);
    auto etime = std::chrono::steady_clock::now();
    double tcost = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();
    //t.start();
    // GetPathCostSRC(ref, s, g, 20);
    //t.stop();
    //double tcost = t.elapsed_time_nano();
    exps[x].srcTime20moves.push_back(tcost);
  }
}



void GetExperimentsTime(void* ref, warthog::jpsp_oracle& oracle,
    ScenarioLoader& scen, std::vector<Stats>& exps, int hLevel) {
  warthog::timer t;
  for (int x=0; x<scen.GetNumExperiments(); x++) {
    double dist = scen.GetNthExperiment(x).GetDistance();
    xyLoc s, g;
    s.x = scen.GetNthExperiment(x).GetStartX();
    s.y = scen.GetNthExperiment(x).GetStartY();
    g.x = scen.GetNthExperiment(x).GetGoalX();
    g.y = scen.GetNthExperiment(x).GetGoalY();
    auto stime = std::chrono::steady_clock::now();
      double pathcost = GetPathCost(ref, s, g, oracle, hLevel);
    auto etime = std::chrono::steady_clock::now();
    double tcost = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();
    //t.start();
    //  double pathcost = GetPathCost(ref, s, g, oracle);
    //t.stop();
    //double tcost = t.elapsed_time_nano();
    exps[x].time.push_back(tcost);
    if (fabs(pathcost - dist) > warthog::EPS) {
      printf("experiment: %d, expect: %.5f, actual: %.5f\n", x, dist, pathcost);
      assert(false);
    }
    exps[x].pathcost = pathcost;
  }
}

void GetExperimentsTime20Moves(void* ref, warthog::jpsp_oracle& oracle,
    ScenarioLoader& scen, std::vector<Stats>& exps, int hLevel) {
  warthog::timer t;
  for (int x=0; x<scen.GetNumExperiments(); x++) {
    xyLoc s, g;
    s.x = scen.GetNthExperiment(x).GetStartX();
    s.y = scen.GetNthExperiment(x).GetStartY();
    g.x = scen.GetNthExperiment(x).GetGoalX();
    g.y = scen.GetNthExperiment(x).GetGoalY();
    auto stime = std::chrono::steady_clock::now();
      GetPathCost(ref, s, g, oracle, hLevel, 20);
    auto etime = std::chrono::steady_clock::now();
    double tcost = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();
    //t.start();
    //  GetPathCost(ref, s, g, oracle, 20);
    //t.stop(); 
    //double tcost = t.elapsed_time_nano();
    exps[x].time20moves.push_back(tcost);
  }
}

void GetExperimentsPath(void* ref, warthog::jpsp_oracle& oracle,
    ScenarioLoader& scen, std::vector<Stats>& exps, int hLevel) {
  std::vector<xyLoc> thePath;
  for (int x=0; x<scen.GetNumExperiments(); x++) {
    thePath.clear();
    xyLoc s, g;
    s.x = scen.GetNthExperiment(x).GetStartX();
    s.y = scen.GetNthExperiment(x).GetStartY();
    g.x = scen.GetNthExperiment(x).GetGoalX();
    g.y = scen.GetNthExperiment(x).GetGoalY();
    GetPath(ref, s, g, thePath, oracle, hLevel);
    exps[x].path = std::vector<xyLoc>(thePath.begin(), thePath.end());
  }
}

int main(int argc, char **argv) {
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
    if (strcmp(argv[i], "-full") == 0) {
      pre = run = true;
    }
    else if (strcmp(argv[i], "-pre") == 0) {
      pre = true; run = false;
    }
    else if (strcmp(argv[i], "-run") == 0) {
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
    PreprocessMap(mapData, width, height, filename, hLevel);
  
  if (!run)
    return 0;
  outfname = "outputs/" + getMapName(filename) + "-" + std::to_string(hLevel) + ".txt";


  void *reference = PrepareForSearch(mapData, width, height, filename);
  warthog::gridmap gm(mpath.c_str());
  warthog::jpsp_oracle oracle(&gm);
  std::cerr << "Sanity Check: "<< (oracle.sanity_check() ? "pass" : "fail") << "\n";

  ScenarioLoader scen(spath.c_str());

  std::vector<Stats> experimentStats;
  int total_exp = scen.GetNumExperiments();
  experimentStats.resize(total_exp);
  int repeat = 10;

  for (int i=0; i<repeat; i++) {
    GetExperimentsTime(reference, oracle, scen, experimentStats, hLevel);
    GetExperimentsTime20Moves(reference, oracle, scen, experimentStats, hLevel);
    GetExperimentsSRCTime(reference, scen, experimentStats, hLevel);
    GetExperimentsSRCTime20Moves(reference, scen, experimentStats, hLevel);
    GetExperimentsPath(reference, oracle, scen, experimentStats, hLevel);
  }

  std::ofstream out;
  string header = "map,scenid,total,20move,total-src,20move-src,distance,path_size,hLevel";
  out.open (outfname.c_str(), std::ios::app);
  out << header << std::endl;
  string mapname = getMapName(string(filename));
  for (unsigned int x = 0; x < experimentStats.size(); x++) {
    out << mapname << "," << x << "," <<  experimentStats[x].to_string() << "," << hLevel << std::endl;
  }
  out.close();
  return 0;
}
