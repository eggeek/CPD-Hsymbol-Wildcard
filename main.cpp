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

class Stats {
public:
  std::vector<double> srcTime;// = std::vector<double>(3);
  Counter c;

  static string header() {
    return "map,scenid,tcost,distance,steps,access,hLevel";
  }
  string to_string() {
    std::ostringstream res;
    sort(srcTime.begin(), srcTime.end());
    res << srcTime[srcTime.size() / 2] << ",";
    res << c.pathcost << "," << c.steps << "," << c.access_cnt;
    return res.str();
  }
};

class SubOptStats {
public:
  std::vector<double> srcTime;
  Counter c;
  static string header() {
    return "map,scenid,tcost,distance,expect,steps,access,hLevel";
  }

  string to_string(double expect) {
    std::ostringstream res;
    sort(srcTime.begin(), srcTime.end());
    res << srcTime[srcTime.size() / 2] << ",";
    res << c.pathcost << "," << expect << "," << c.steps << "," << c.access_cnt;
    return res.str();
  }
};

void GetExperimentsSRCTime(const Index& ref, ScenarioLoader& scen, std::vector<Stats>& exps, int hLevel) {
  warthog::timer t;
  double (*runner)(const Index& data, xyLoc s, xyLoc t, int hLevel, Counter& c, int limit);
  if (ref.p.itype == "vanilla")
    runner = GetPathCostSRC;
  else if (ref.p.itype == "rect")
    runner = GetRectWildCardCost;
  else if (ref.p.itype == "inv")
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

void GetSubOptExperimentsSRCTime(const Index& ref, ScenarioLoader& scen, std::vector<SubOptStats>& exps, const Parameters& p) {
  warthog::timer t;
  double (*runner)(const Index& data, xyLoc s, xyLoc t, int hLevel, Counter& c, int limit);
  if (ref.p.itype == "vanilla")
    runner = GetForwardCentroidCost;
  else if (ref.p.itype == "rect")
    runner = GetRectWildCardCost;
  else if (ref.p.itype == "inv")
    runner = GetInvCentroidCost;

  for (int x=0; x<scen.GetNumExperiments(); x++) {
    double dist = scen.GetNthExperiment(x).GetDistance();
    xyLoc s, g;
    s.x = scen.GetNthExperiment(x).GetStartX();
    s.y = scen.GetNthExperiment(x).GetStartY();
    g.x = scen.GetNthExperiment(x).GetGoalX();
    g.y = scen.GetNthExperiment(x).GetGoalY();
    exps[x].c = Counter{0, 0, 0};
    auto stime = std::chrono::steady_clock::now();
      exps[x].c.pathcost = runner(ref, s, g, p.hLevel, exps[x].c, -1);
    auto etime = std::chrono::steady_clock::now();
    double tcost = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();
    exps[x].srcTime.push_back(tcost);
    if (fabs(exps[x].c.pathcost - dist) > warthog::EPS + 2*p.centroid) {
      printf("experiment: %d, expect: %.5f, actual: %.5f\n", x, dist, exps[x].c.pathcost);
      assert(false);
    }
  }
}

void OptimalExperiments(int repeat, const Index& data,
    const string& outfname,
    const string& spath,
    const string& filename) {
  ScenarioLoader scen(spath.c_str());
  std::vector<Stats> experimentStats;
  int total_exp = scen.GetNumExperiments();
  experimentStats.resize(total_exp);
  for (int i=0; i<repeat; i++) {
    GetExperimentsSRCTime(data, scen, experimentStats, data.p.hLevel);
  }

  std::ofstream out;
  string header = Stats::header();
  out.open (outfname.c_str(), std::ios::app);
  out << header << std::endl;
  string mapname = getMapName(string(filename));
  for (unsigned int x = 0; x < experimentStats.size(); x++) {
    out << mapname << "," << x << "," <<  experimentStats[x].to_string() << "," << data.p.hLevel << std::endl;
  }
  out.close();
}

void SubOptExperiments(int repeat, const Index& data,
    const string& outfname,
    const string& spath,
    const string& filename) {

  ScenarioLoader scen(spath.c_str());
  std::vector<SubOptStats> experimentStats;
  int total_exp = scen.GetNumExperiments();
  experimentStats.resize(total_exp);
  for (int i=0; i<repeat; i++) {
    GetSubOptExperimentsSRCTime(data, scen, experimentStats, data.p);
  }

  std::ofstream out;
  string header = SubOptStats::header();
  out.open (outfname.c_str(), std::ios::app);
  out << header << std::endl;
  string mapname = getMapName(string(filename));
  for (unsigned int x = 0; x < experimentStats.size(); x++) {
    double expect = scen.GetNthExperiment(x).GetDistance();
    out << mapname << "," << x << "," <<  experimentStats[x].to_string(expect) << "," << data.p.hLevel << std::endl;
  }
  out.close();
}

int main(int argc, char **argv) {
  // process command line
  string mpath, spath, indexpath, outfname, otype, itype;
  int pre=0, run=0, hLevel=1, centroid=0;

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
    ("type,T", po::value<string>(&itype)->default_value("vanilla"), "type of cpd")
    ("output,O", po::value<string>(&outfname), "output path")
    ("order", po::value<string>(&otype)->default_value("DFS"), "type of ordering")
    ("centroid,C", po::value<int>(&centroid)->default_value(0), "using centroid cpd")
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
  sprintf(filename, "./index_data/%s.map-%s-%d-%s", getMapName(mpath).c_str() , otype.c_str(),
      hLevel, centroid?"subopt":"opt");

  Parameters p{otype, itype, filename, hLevel, centroid};
  if (pre) {
    if (!centroid)
      PreprocessMap(mapData, width, height, p);
    else
      PreprocessCentroid(mapData, width, height, p);
  }
  
  if (!run)
    return 0;

  Index data;
  data = LoadIndexData(mapData, width, height, indexpath.c_str());

  if (!vm.count("output"))
    outfname = "outputs/" + getMapName(filename) + "-" + 
               std::to_string(data.p.hLevel) + 
               (data.p.centroid?"-subopt":"-opt") + ".txt";

  int repeat = 10;
  if (!centroid)
    OptimalExperiments(repeat, data, outfname, spath, filename);
  else
    SubOptExperiments(repeat, data, outfname, spath, filename);
  return 0;
}
