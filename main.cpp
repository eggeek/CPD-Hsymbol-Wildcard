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

namespace po = boost::program_options;
po::variables_map vm;
string type;

class Stats {
public:
  //std::vector<double> time;// = std::vector<double>(3);
  //std::vector<double> time20moves;// = std::vector<double>(3);
  std::vector<double> srcTime;// = std::vector<double>(3);
  //std::vector<double> srcTime20moves;// = std::vector<double>(3);
  Counter c;
  //std::vector<xyLoc> path;

  static string header() {
    return "map,scenid,tcost,distance,steps,access,hLevel";
  }
  string to_string() {
    std::ostringstream res;
    //sort(time.begin(), time.end());
    //sort(time20moves.begin(), time20moves.end());
    sort(srcTime.begin(), srcTime.end());
    //sort(srcTime20moves.begin(), srcTime20moves.end());
    // get the mean
    //res << time[time.size() / 2] << ",";
    //res << time20moves[time20moves.size() / 2] << ",";
    res << srcTime[srcTime.size() / 2] << ",";
    //res << srcTime20moves[srcTime20moves.size() / 2] << ",";
    res << c.pathcost << "," << c.steps << "," << c.access_cnt;
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
  printf("\t\t1: H symbol with level1 heuristic function\n");
  printf("\t\t2: H symbol with level2 heuristic function\n");
  printf("\t\t3: H symbol with level3 heuristic function\n");
}

void GetExperimentsSRCTime(const Index& ref, ScenarioLoader& scen, std::vector<Stats>& exps, int hLevel) {
  warthog::timer t;
  double (*runner)(const Index& data, xyLoc s, xyLoc t, int hLevel, Counter& c, int limit);
  if (type == "vanilla")
    runner = GetPathCostSRC;
  else if (type == "rect")
    runner = GetRectWildCardCost;
  else if (type == "inv")
    runner = GetInvCPDCost;

  for (int x=0; x<scen.GetNumExperiments(); x++) {
    double dist = scen.GetNthExperiment(x).GetDistance();
    xyLoc s, g;
    s.x = scen.GetNthExperiment(x).GetStartX();
    s.y = scen.GetNthExperiment(x).GetStartY();
    g.x = scen.GetNthExperiment(x).GetGoalX();
    g.y = scen.GetNthExperiment(x).GetGoalY();
    exps[x].c = Counter{0, 0, 0};
    auto stime = std::chrono::steady_clock::now();
      exps[x].c.pathcost = runner(ref, s, g, hLevel, exps[x].c, -1);
    auto etime = std::chrono::steady_clock::now();
    double tcost = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();
    exps[x].srcTime.push_back(tcost);
    if (fabs(exps[x].c.pathcost - dist) > warthog::EPS) {
      printf("experiment: %d, expect: %.5f, actual: %.5f\n", x, dist, exps[x].c.pathcost);
      assert(false);
    }
  }
}

int main(int argc, char **argv) {
  // process command line
  string mpath, spath, indexpath, outfname, otype;
  int pre=0, run=0, hLevel=1;

  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,H", "help message")
    ("full,F", "preprocess map then run scenario")
    ("preprocess,P", "preprocess map")
    ("run,R", "run scenario without preprocessing")
    ("hLevel,L", po::value<int>(&hLevel)->default_value(0), "Level of heuristic")
    ("map,M", po::value<string>(&mpath)->required(), "path of map")
    ("scen,S", po::value<string>(&spath), "path of scenario")
    ("index,I", po::value<string>(&indexpath), "path of index")
    ("type,T", po::value<string>(&type)->default_value("vanilla"), "type of cpd")
    ("output,O", po::value<string>(&outfname), "output path")
    ("order", po::value<string>(&otype)->default_value("DFS"), "type of ordering")
  ;

  po::store(po::parse_command_line(argc, argv, desc), vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  po::notify(vm);
  if (vm.count("full"))       pre = run = true;
  if (vm.count("preprocess")) pre = true;
  if (vm.count("run"))        run = true;
  if (run) {
    if (!vm.count("index") || !vm.count("scen")) {
      std::cout << "require index & scen path" << endl;
    }
  }
  // done

  char filename[255];
  std::vector<xyLoc> thePath;
  std::vector<bool> mapData;
  int width, height;

  LoadMap(mpath.c_str(), mapData, width, height);
  sprintf(filename, "./index_data/%s.map-%s-%d", getMapName(mpath).c_str() , otype.c_str(), hLevel);

  Parameters p{otype, type, filename, hLevel};
  if (pre)
    PreprocessMap(mapData, width, height, p);
  
  if (!run)
    return 0;

  if (!vm.count("output"))
    outfname = "outputs/" + getMapName(filename) + "-" + std::to_string(hLevel) + ".txt";

  Index reference;
  if (type == "vanilla")
    reference = LoadVanillaCPD(mapData, width, height, indexpath.c_str());
  else if (type == "rect")
    reference = LoadRectWildCard(mapData, width, height, indexpath.c_str());
  else if (type == "inv")
    reference = LoadInvCPD(mapData, width, height, indexpath.c_str());

  ScenarioLoader scen(spath.c_str());

  std::vector<Stats> experimentStats;
  int total_exp = scen.GetNumExperiments();
  experimentStats.resize(total_exp);
  int repeat = 10;

  for (int i=0; i<repeat; i++) {
    GetExperimentsSRCTime(reference, scen, experimentStats, hLevel);
  }

  std::ofstream out;
  string header = Stats::header();
  out.open (outfname.c_str(), std::ios::app);
  out << header << std::endl;
  string mapname = getMapName(string(filename));
  for (unsigned int x = 0; x < experimentStats.size(); x++) {
    out << mapname << "," << x << "," <<  experimentStats[x].to_string() << "," << hLevel << std::endl;
  }
  out.close();
  return 0;
}
