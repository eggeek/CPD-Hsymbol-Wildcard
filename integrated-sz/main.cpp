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
  std::vector<double> time;// = std::vector<double>(3);
  std::vector<double> time20moves;// = std::vector<double>(3);
  std::vector<double> srcTime;// = std::vector<double>(3);
  std::vector<double> srcTime20moves;// = std::vector<double>(3);
  double pathcost;
  std::vector<xyLoc> path;

  string to_string() {
    std::ostringstream res;
    sort(time.begin(), time.end());
    sort(time20moves.begin(), time20moves.end());
    sort(srcTime.begin(), srcTime.end());
    sort(srcTime20moves.begin(), srcTime20moves.end());
    // get the mean
    //res << time[time.size() / 2] << ",";
    //res << time20moves[time20moves.size() / 2] << ",";
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
  printf("\t\t1: H symbol with level1 heuristic function\n");
  printf("\t\t2: H symbol with level2 heuristic function\n");
  printf("\t\t3: H symbol with level3 heuristic function\n");
}

void GetExperimentsSRCTime(void* ref, ScenarioLoader& scen, std::vector<Stats>& exps, int hLevel, bool pure) {
  double (*GetPathFunc)(void*, xyLoc, xyLoc, int, int);
  if (pure) GetPathFunc = GetPathCostSRCPurely;
  else GetPathFunc = GetPathCostSRC;

  for (int x=0; x<scen.GetNumExperiments(); x++) {
    double dist = scen.GetNthExperiment(x).GetDistance();
    xyLoc s, g;
    s.x = scen.GetNthExperiment(x).GetStartX();
    s.y = scen.GetNthExperiment(x).GetStartY();
    g.x = scen.GetNthExperiment(x).GetGoalX();
    g.y = scen.GetNthExperiment(x).GetGoalY();
    auto stime = std::chrono::steady_clock::now();
      auto pathcost = GetPathFunc(ref, s, g, hLevel, -1);
    auto etime = std::chrono::steady_clock::now();
    auto tcost = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();
    exps[x].srcTime.push_back(tcost);
    if (fabs(pathcost - dist) > warthog::EPS) {
      printf("experiment: %d, expect: %.5f, actual: %.5f\n", x, dist, pathcost);
      assert(false);
    }
  }
}

void GetExperimentsSRCTime20Moves(void* ref, ScenarioLoader& scen, std::vector<Stats>& exps, int hLevel, bool pure) {
  double (*GetPathFunc)(void*, xyLoc, xyLoc, int, int);
  if (pure) GetPathFunc = GetPathCostSRCPurely;
  else GetPathFunc = GetPathCostSRC;

  for (int x=0; x<scen.GetNumExperiments(); x++) {
    xyLoc s, g;
    s.x = scen.GetNthExperiment(x).GetStartX();
    s.y = scen.GetNthExperiment(x).GetStartY();
    g.x = scen.GetNthExperiment(x).GetGoalX();
    g.y = scen.GetNthExperiment(x).GetGoalY();
    auto stime = std::chrono::steady_clock::now();
      GetPathFunc(ref, s, g, hLevel, 20);
    auto etime = std::chrono::steady_clock::now();
    double tcost = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();
    exps[x].srcTime20moves.push_back(tcost);
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
  bool pure = false; // whether run `SRC` purely
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
    else if (strcmp(argv[i], "-pure") == 0) {
      pure = true;
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
  if (pure)
    sprintf(filename, "./index_data/%s.map-%s-pure-%d", getMapName(mpath).c_str() , GetName(), hLevel);
  else
    sprintf(filename, "./index_data/%s.map-%s-%d", getMapName(mpath).c_str() , GetName(), hLevel);

  if (pre) {
    if (pure)
      PreprocessMapPurely(mapData, width, height, filename, hLevel);
    else
      PreprocessMap(mapData, width, height, filename, hLevel);
  }
  
  if (!run)
    return 0;

  if (pure)
    outfname = "outputs/" + getMapName(filename) + "-pure-" + std::to_string(hLevel) + ".txt";
  else
    outfname = "outputs/" + getMapName(filename) + "-" + std::to_string(hLevel) + ".txt";


  void *reference;
  if (pure)
    reference = PrepareForSearchPurely(mapData, width, height, filename);
  else
    reference = PrepareForSearch(mapData, width, height, filename);

  ScenarioLoader scen(spath.c_str());

  std::vector<Stats> experimentStats;
  int total_exp = scen.GetNumExperiments();
  experimentStats.resize(total_exp);
  int repeat = 10;

  for (int i=0; i<repeat; i++) {
    GetExperimentsSRCTime(reference, scen, experimentStats, hLevel, pure);
    GetExperimentsSRCTime20Moves(reference, scen, experimentStats, hLevel, pure);
  }

  std::ofstream out;
  //string header = "map,scenid,total,20move,total-src,20move-src,distance,path_size,hLevel";
  string header = "map,scenid,total-src,20move-src,distance,path_size,hLevel,pure";
  out.open (outfname.c_str());
  out << header << std::endl;
  string mapname = getMapName(string(filename));
  for (unsigned int x = 0; x < experimentStats.size(); x++) {
    out << mapname << "," << x << "," <<  experimentStats[x].to_string() << "," << hLevel << pure << std::endl;
  }
  out.close();
  return 0;
}
