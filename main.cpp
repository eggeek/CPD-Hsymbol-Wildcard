#include <boost/program_options.hpp>
#include <numeric>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <chrono>
#include <unistd.h>
#include "ScenarioLoader.h"
#include "preprocessing.h"
#include "query.h"
#include "loader.h"
#include "jpsp_oracle.h"
#include "constants.h"
#include "timer.h"
#include "rect_wildcard.h"

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

const char *GetName()
{
  #ifndef USE_CUT_ORDER
  return "DFS-SRC-RLE";
  #else
  return "METIS-CUT-SRC-RLE";
  #endif
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

void GetExperimentsSRCTime(const Index& ref, ScenarioLoader& scen, std::vector<Stats>& exps, int hLevel) {
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

void GetExperimentsSRCTime20Moves(const Index& ref, ScenarioLoader& scen, std::vector<Stats>& exps, int hLevel) {
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

int main(int argc, char **argv) {
  // process command line
  string mpath, spath;
  int pre=0, run=0, hLevel=1;

  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,H", "help message")
    ("full,F", "preprocess map then run scenario")
    ("preprocess,P", "preprocess map")
    ("run,R", "run scenario without preprocessing")
    ("hLevel,L", po::value<int>(&hLevel)->default_value(1), "Level of heuristic")
    ("map,M", po::value<string>(&mpath)->required(), "path of map")
    ("scen,S", po::value<string>(&spath)->required(), "path of scenario")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  po::notify(vm);
  if (vm.count("full"))       pre = run = true;
  if (vm.count("preprocess")) pre = true;
  if (vm.count("run"))        run = true;
  // done

  char filename[255];
  string outfname;
  std::vector<xyLoc> thePath;
  std::vector<bool> mapData;
  int width, height;

  LoadMap(mpath.c_str(), mapData, width, height);
  sprintf(filename, "./index_data/%s.map-%s-%d", getMapName(mpath).c_str() , GetName(), hLevel);

  if (pre)
    PreprocessMap(mapData, width, height, filename, hLevel);
  
  if (!run)
    return 0;
  outfname = "outputs/" + getMapName(filename) + "-" + std::to_string(hLevel) + ".txt";


  const Index& reference = PrepareForSearch(mapData, width, height, filename);

  ScenarioLoader scen(spath.c_str());

  std::vector<Stats> experimentStats;
  int total_exp = scen.GetNumExperiments();
  experimentStats.resize(total_exp);
  int repeat = 10;

  for (int i=0; i<repeat; i++) {
    GetExperimentsSRCTime(reference, scen, experimentStats, hLevel);
    GetExperimentsSRCTime20Moves(reference, scen, experimentStats, hLevel);
  }

  std::ofstream out;
  //string header = "map,scenid,total,20move,total-src,20move-src,distance,path_size,hLevel";
  string header = "map,scenid,total-src,20move-src,distance,path_size,hLevel";
  out.open (outfname.c_str(), std::ios::app);
  out << header << std::endl;
  string mapname = getMapName(string(filename));
  for (unsigned int x = 0; x < experimentStats.size(); x++) {
    out << mapname << "," << x << "," <<  experimentStats[x].to_string() << "," << hLevel << std::endl;
  }
  out.close();
  return 0;
}
