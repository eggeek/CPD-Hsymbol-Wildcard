#include <numeric>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <unistd.h>
#include "ScenarioLoader.h"
#include "RoadNetworkLoader.h"
#include "Preprocessing.h"
#include "Query.h"
#include "jpsp_oracle.h"
#include "constants.h"
#include "timer.h"

using namespace std;

const char *GetName()
{
  #ifndef USE_CUT_ORDER
  return "DFS-SRC-RLE";
  #else
  return "METIS-CUT-SRC-RLE";
  #endif
}

string getMapName(string filename) {
  auto pos = filename.find_last_of('/');
  const string mapfile = filename.substr(pos + 1);
  auto suff = mapfile.find('.');
  return mapfile.substr(0, suff);
}

string getFileName(string path) {
  auto pos = path.find_last_of('/');
  return path.substr(pos+1);
}

string rmFileSuffix(string filename) {
  auto pos = filename.find_last_of('.');
  return filename.substr(0, pos);
}

bool exists(const string& fname) {
  ifstream f(fname.c_str());
  return f.good();
}

void argHelp(char **argv) {
  printf("Invalid Arguments\nUsage %s <flags> <map> <scenario>\n", argv[0]);
  printf("Flags:\n");
  printf("\t-full : Preprocess map and run scenario\n");
  printf("\t-pre : Preprocess map\n");
  printf("\t-run : Run scenario without preprocessing\n");
  printf("\t-dfs : dfs ordering");
  printf("\t-fractal: fractal ordering");
  printf("\t[hLevel] : int (defualt=1)\n");
  printf("\t\t0: no H symbol\n");
  printf("\t\t1: H symbol with basic heuristic function\n");
  printf("\t\t2: H symbol with improved heuristic function\n");
}

int main(int argc, char **argv) {
  string filename;
  string outfname;
  string mpath;
  string scen_path;
  string index_path;
  bool pre = false;
  bool run = false;
  bool dfs = true;
  bool fractal = false;
  bool sector = false;
  bool ext_sector  = false;
  long long quat = 1;
  int hLevel = 1;

  mpath = argv[argc-1];
  for (int i=1; i<argc-1; i++) {
    if (strcmp(argv[i], "-full") == 0) {
      pre = run = true;
    }
    else if (strcmp(argv[i], "-pre") == 0) {
      pre = true; run = false;
    }
    else if (strcmp(argv[i], "-run") == 0) {
      run = true; pre = false;
    }
    else if (strcmp(argv[i], "-h") == 0) {
      hLevel = atoi(argv[i+1]);
      i++;
    }
    else if (strcmp(argv[i], "-dfs") == 0) {
      dfs = true, fractal = false;
    }
    else if (strcmp(argv[i], "-fractal") == 0) {
      dfs = false, fractal = true;
    }
    else if (strcmp(argv[i], "-sector") == 0) {
      sector = true;
    }
    else if (strcmp(argv[i], "-ext-sector") == 0) {
      sector = true, ext_sector = true;
    }
    else if (strcmp(argv[i], "-quant") == 0) {
      quat = atoll(argv[i+1]);
      i++;
    }
    else if (strcmp(argv[i], "-i") == 0) {
      index_path = string(argv[i+1]);
      i++;
    }
    else if (strcmp(argv[i], "-s") == 0) {
      scen_path = string(argv[i+1]);
      i++;
    }
    else {
      printf("unknow arg: %s", argv[i]);
      argHelp(argv);
      exit(0);
    }
  }
  string mname = getMapName(mpath);
  string grpath = mpath + "/" + mname + ".gr";
  string copath = mpath + "/" + mname + ".co";

  auto gen_name = [&](string suffix=".cpd") {
    string res = "./index_data/" + mname + 
      "-" + GetName() + 
      "-" + to_string(hLevel);
    if (sector) res += "-sector";
    if (ext_sector) res +="-ext";
    if (quat != 1) res += "-q" + to_string(quat);
    return res + suffix;
  };
  filename = gen_name();

  assert(exists(grpath));
  vector<xyLoc> coord;
  ListGraph listg;
  if (exists(copath)) {
    listg = RoadNetwork::Load(grpath, copath, coord);
  }
  else {
    listg = RoadNetwork::Load(grpath);
    coord.resize(listg.node_count());
  }
  if (pre) {
    int order_code = warthog::DFS;
    if (fractal) order_code = warthog::FRACTAL;
    if (sector)
      PreprocessMapWithSectorWildCard(order_code, listg, coord, filename.c_str(), hLevel, ext_sector, quat);
    else
      PreprocessMapPurely(order_code, listg, coord, filename.c_str(), hLevel, quat);
  }

  if (!run)
    return 0;
  assert(index_path != "");
  assert(scen_path != "");
  outfname = "outputs/" + rmFileSuffix(getFileName(index_path)) + ".csv";
  vector<RoadNetwork::Scen> scens = RoadNetwork::LoadScenarios(scen_path);
  void* ref;
  if (sector)
    ref = PrepareForSearchSectorWildCard(listg, coord, index_path.c_str());
  else
    ref = PrepareForSearchPurely(listg, coord, index_path.c_str());
  
  ofstream out;
  out.open(outfname.c_str(), ios::app);
  string header = "map,s,t,expect,cost,diff,path_size";
  out << header << endl;
  vector<int> path;
  for (const auto& it: scens) {
    long long dist;
    path.clear();
    if (sector)
      dist = GetPathWithSectorWildCard(ref, it.s, it.t, path, hLevel, -1);
    else
      dist = GetPath(ref, it.s, it.t, path, hLevel, -1);
    long long diff = dist - it.dist;
    out << mname << "," << it.s << "," << it.t << "," << it.dist << ","
        << dist << "," << diff << "," <<  path.size() << endl;
  }
  return 0;
}
