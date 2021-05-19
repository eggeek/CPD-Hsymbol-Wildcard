#include <random>
#include <set>
#include <string.h>
#include "RoadNetworkLoader.h"
#include "dijkstra.h"
#include "Preprocessing.h"
#include "Query.h"
using namespace std;
double r;
string path;
string index_path;
int p2p_num, hlevel = 0, sid = -1;

string MapName(string dir) {
  auto pos = dir.find_last_of('/');
  return dir.substr(++pos);
}

void genSubGr() {
  string mname = MapName(path);
  string grpath = path + "/" + mname + ".gr";
  string copath = path + "/" + mname + ".co";
  vector<xyLoc> coord;
  ListGraph listg = RoadNetwork::LoadSub(grpath, copath, coord, r);
  AdjGraph adj(listg);
  RoadNetwork::print(adj);
}

void genSubCo() {
  string mname = MapName(path);
  string grpath = path + "/" + mname + ".gr";
  string copath = path + "/" + mname + ".co";
  vector<xyLoc> coord;
  ListGraph listg = RoadNetwork::LoadSub(grpath, copath, coord, r);
  RoadNetwork::print(coord);
}

void genScenario() {
  string mname = MapName(path);
  string grpath = path + "/" + mname + ".gr";
  string copath = path + "/" + mname + ".co";
  vector<xyLoc> coord;
  ListGraph listg = RoadNetwork::Load(grpath, copath, coord);
  random_device rd;
  mt19937 eng(rd());

  AdjGraph adj(listg);
  Mapper mapper(listg, coord);
  NodeOrdering order = compute_real_dfs_order(listg);
  mapper.reorder(order);
  Dijkstra dij(mapper.g, mapper);
  string header = "s,t,dist,map";
  cout << header << endl;
  set<pair<int, int>> pairs;
  if (sid == -1) {
    for (int i=0; i<p2p_num; i++) {
      int u, v;
      while (true) {
        uniform_int_distribution<int> genu(0, listg.node_count()-1);
        u = genu(eng);
        if (u-1 < 0) continue;
        uniform_int_distribution<int> genv(0, u-1);
        v = genv(eng);
        if (pairs.find({u, v}) != pairs.end()) continue;
        pairs.insert({u, v});
        break;
      }
      int posu = mapper.getPos(u);
      int posv = mapper.getPos(v);
      dij.run(posu, 0);
      long long dist = dij.distance(posv);
      if (dist == numeric_limits<long long>::max()) dist = -1;
      cout << u << "," << v << "," << dist << "," << mname << endl;
    }
  } else {
    int u = sid;
    for (int i=0; i<p2p_num; i++) {
      int v;
      while (true) {
        uniform_int_distribution<int> genv(0, listg.node_count()-1);
        v = genv(eng);
        if (pairs.find({u, v}) != pairs.end()) continue;
        pairs.insert({u, v});
        break;
      }
      int posu = mapper.getPos(u);
      int posv = mapper.getPos(v);
      dij.run(posu, 0);
      long long dist = dij.distance(posv);
      if (dist == numeric_limits<long long>::max()) dist = -1;
      cout << u << "," << v << "," << dist << "," << mname << endl;
    }
  }
}


void genCPDSectorCoord() {
  string mname = MapName(path);
  string grpath = path + "/" + mname + ".gr";
  string copath = path + "/" + mname + ".co";
  vector<xyLoc> coord;
  ListGraph listg = RoadNetwork::Load(grpath, copath, coord);

  void* data = PrepareForSearchSectorWildCard(listg, coord, index_path.c_str());
  State* state = static_cast<State*>(data);
  cerr << "Entry size: " << state->cpd.get_entry_size() << endl;
  int maxs = 0;
  if (sid == -1) {
    for (int j=0; j<state->mapper.g.node_count(); j++) {
      if (state->cpd.get_row_length(j) > maxs) {
        maxs = state->cpd.get_row_length(j);
        sid = j;
      }
    }
  }
  else maxs = state->cpd.get_row_length(sid);
  Sectors tmp = state->sectors[sid];
  cerr << "sid: " << sid << endl;
  cerr << "row length: " << maxs << endl;
  vector<int> ids = state->cpd.get_ids(sid);
  set<int> id_set = set<int>(ids.begin(), ids.end());
  string header = "s,t,lats,lons,latt,lont,eid,h";
  printf("%s\n", header.c_str());
  xyLoc sloc = state->mapper(sid);
  for (int id=0; id<(int)listg.node_count(); id++) if (id != sid) {
    xyLoc tloc = state->mapper(id);
    double lats = (double)sloc.y / 1000000.0;
    double lons = (double)sloc.x / 1000000.0;
    double latt = (double)tloc.y / 1000000.0;
    double lont = (double)tloc.x / 1000000.0;
    int eid, d;
    d = Sectors::find_sector(state->sectors[sid], sloc, tloc);
    if (d != -1)
      eid = 8;
    //else if (id_set.find(id) == id_set.end())
    //  continue;
    else eid = state->cpd.get_first_move(sid, id);
    int h = 0;
    printf("%d,%d,%.6lf,%.6lf,%.6lf,%.6lf,%d,%d\n", sid, id, lats, lons, latt, lont, eid, h);
  }
}

void genCPDCoord() {
  string mname = MapName(path);
  string grpath = path + "/" + mname + ".gr";
  string copath = path + "/" + mname + ".co";
  vector<xyLoc> coord;
  ListGraph listg = RoadNetwork::Load(grpath, copath, coord);

  void* data = PrepareForSearchPurely(listg, coord, index_path.c_str());
  State* state = static_cast<State*>(data);
  cerr << "Entry size: " << state->cpd.get_entry_size() << endl;
  int maxs = 0;
  if (sid == -1) {
    for (int j=0; j<state->mapper.g.node_count(); j++) {
      if (state->cpd.get_row_length(j) > maxs) {
        maxs = state->cpd.get_row_length(j);
        sid = j;
      }
    }
  }
  else maxs = state->cpd.get_row_length(sid);
  cerr << "sid: " << sid << endl;
  cerr << "row length: " << maxs << endl;
  vector<int> ids = state->cpd.get_ids(sid);
  string header = "s,t,lats,lons,latt,lont,eid,h";
  printf("%s\n", header.c_str());
  for (int id: ids) {
    double lats = (double)state->mapper(sid).y / 1000000.0;
    double lons = (double)state->mapper(sid).x / 1000000.0;
    double latt = (double)state->mapper(id).y / 1000000.0;
    double lont = (double)state->mapper(id).x / 1000000.0;
    int eid = state->cpd.get_first_move(sid, id);
    int h = 0;
    printf("%d,%d,%.6lf,%.6lf,%.6lf,%.6lf,%d,%d\n", sid, id, lats, lons, latt, lont, eid, h);
  }
} 

void genCPDInfo() {
  string mname = MapName(path);
  string grpath = path + "/" + mname + ".gr";
  string copath = path + "/" + mname + ".co";
  vector<xyLoc> coord;
  ListGraph listg = RoadNetwork::Load(grpath, copath, coord);

  void* data = PrepareForSearchPurely(listg, coord, index_path.c_str());
  State* state = static_cast<State*>(data);
  cerr << "Entry size: " << state->cpd.get_entry_size() << endl;
  string header = "s,t,lats,lons,latt,lont,eid,h";
  printf("%s\n", header.c_str());
  int maxs = 0;
  if (sid == -1) {
    for (int j=0; j<state->mapper.g.node_count(); j++) {
      if (state->cpd.get_row_length(j) > maxs) {
        maxs = state->cpd.get_row_length(j);
        sid = j;
      }
    }
  }
  else maxs = state->cpd.get_row_length(sid);
  cerr << "sid: " << sid << endl;
  cerr << "row length: " << maxs << endl;
  int i = sid, cnth = 0;
  for (int j=0; j<state->mapper.g.node_count(); j++) if (j != i) {
    int move = state->cpd.get_first_move(i, j);
    if (move == 0xF) continue;
    bool h = false;

    if ((1 << move) == warthog::HMASK) {
      h = true;
      move = H::decode(i, j, state->mapper, hlevel);
    }
    else if (hlevel) {
        int hmove = warthog::INVALID_MOVE;
        switch (hlevel) {
          case 1:
            hmove = Hsymbol::get_angle_heuristic(i, j, state->mapper);
            break;
          case 2:
            hmove = Hsymbol::get_distance_heuristic(i, j, state->mapper);
            break;
          default:
            break;
        }
        if (hmove == move) h = true;
    }
    cnth += (int)h;
    double lats = (double)state->mapper(i).y / 1000000.0;
    double lons = (double)state->mapper(i).x / 1000000.0;
    double latt = (double)state->mapper(j).y / 1000000.0;
    double lont = (double)state->mapper(j).x / 1000000.0;
    int eid = move;
    printf("%d,%d,%.6lf,%.6lf,%.6lf,%.6lf,%d,%d\n", i, j, lats, lons, latt, lont, eid, h);
  }
  cerr << "num of h symbol: " << cnth / 2 << endl;
}

void genCPDDesc() {
  string mname = MapName(path);
  string grpath = path + "/" + mname + ".gr";
  string copath = path + "/" + mname + ".co";
  vector<xyLoc> coord;
  ListGraph listg = RoadNetwork::Load(grpath, copath, coord);

  void* data = PrepareForSearchPurely(listg, coord, index_path.c_str());
  State* state = static_cast<State*>(data);
  cerr << "Entry size: " << state->cpd.get_entry_size() << endl;
  string header = "id,lat,lon,cnt";
  map<int, int> cnt;
  for (int i=0; i<state->graph.node_count(); i++) {
    map<int, int> row_cnt = state->cpd.get_statistic(i);
    for (const auto& it: row_cnt)
      cnt[it.first] += it.second;
  }
  printf("%s\n", header.c_str());
  for (const auto& it: cnt) {
    xyLoc loc = state->mapper(it.first);
    double lat = (double)loc.y / 1000000.0;
    double lon = (double)loc.x / 1000000.0;
    printf("%d,%.6lf,%.6lf,%d\n", it.first, lat, lon, it.second);
  }
}

void genCoordOrderCSV(bool fractal) {
  string mname = MapName(path);
  string grpath = path + "/" + mname + ".gr";
  string copath = path + "/" + mname + ".co";
  vector<xyLoc> coord;
  ListGraph listg = RoadNetwork::Load(grpath, copath, coord);
  Mapper mapper(listg, coord);
  NodeOrdering order;
  if (!fractal) order = compute_real_dfs_order(listg);
  else order = compute_fractal_order(coord);
  mapper.reorder(order);
  string header = "rank,lat,lon,map";
  printf("%s\n", header.c_str());
  for (int i=0; i<mapper.g.node_count(); i++) {
    double lon = (double)mapper(i).x / 1000000.0;
    double lat = (double)mapper(i).y / 1000000.0;
    printf("%d,%.6lf,%.6lf,%s\n", i, lat, lon, mname.c_str());
  }
}

void genFirstMove() {
  string mname = MapName(path);
  string grpath = path + "/" + mname + ".gr";
  string copath = path + "/" + mname + ".co";
  vector<xyLoc> coord;
  ListGraph listg = RoadNetwork::Load(grpath, copath, coord);

  void* data = PrepareForSearchPurely(listg, coord, index_path.c_str());
  State* state = static_cast<State*>(data);
  cerr << "Entry size: " << state->cpd.get_entry_size() << endl;
  int maxs = 0;
  if (sid == -1) {
    for (int j=0; j<state->mapper.g.node_count(); j++) {
      if (state->cpd.get_row_length(j) > maxs) {
        maxs = state->cpd.get_row_length(j);
        sid = j;
      }
    }
  }
  else maxs = state->cpd.get_row_length(sid);
  cerr << "sid: " << sid << endl;
  cerr << "row length: " << maxs << endl;
  string header = "s,t,lats,lons,latt,lont,eid,h";
  printf("%s\n", header.c_str());
  for (int id=0; id<(int)listg.node_count(); id++) if (id != sid) {
    double lats = (double)state->mapper(sid).y / 1000000.0;
    double lons = (double)state->mapper(sid).x / 1000000.0;
    double latt = (double)state->mapper(id).y / 1000000.0;
    double lont = (double)state->mapper(id).x / 1000000.0;
    int eid = state->cpd.get_first_move(sid, id);
    int h = 0;
    printf("%d,%d,%.6lf,%.6lf,%.6lf,%.6lf,%d,%d\n", sid, id, lats, lons, latt, lont, eid, h);
  }
}

void genCPDSingleRowDesc(bool fractal, bool ext) {
  string mname = MapName(path);
  string grpath = path + "/" + mname + ".gr";
  string copath = path + "/" + mname + ".co";
  vector<xyLoc> coord;
  ListGraph listg = RoadNetwork::Load(grpath, copath, coord);
  Mapper mapper(listg, coord);
  NodeOrdering order;
  if (fractal)
    order = compute_fractal_order(coord);
  else
    order = compute_real_dfs_order(listg);
  mapper.reorder(order);
  Dijkstra dij(mapper.g, mapper);
  int hLevel = 1;
  Sectors sectors;
  auto allowed = dij.run(sid, hLevel, sectors, ext);
  CPD cpd;
  cpd.append_row(sid, allowed, sectors, mapper);
  vector<int> ids = cpd.get_ids(0, true);
  string header = "s,t,lats,lons,latt,lont,symbol,len";
  xyLoc s = mapper(sid);
  double lats = s.y / 1000000.0;
  double lons = s.x / 1000000.0;

  printf("%s\n", header.c_str());
  for (int i=0; i<(int)ids.size(); i++) {
    int id = ids[i];
    xyLoc t = mapper(id);
    double latt = t.y / 1000000.0;
    double lont = t.x / 1000000.0;
    int symbol = cpd.get_first_move(0, id);
    int len = cpd.get_run_size(0, i, listg.node_count());
    printf("%d,%d,%.6lf,%.6lf,%.6lf,%.6lf,%d,%d\n", sid, id, lats, lons, latt, lont, symbol, len);
  }
}

void genCPDSingleRow(bool fractal, bool ext) {
  string mname = MapName(path);
  string grpath = path + "/" + mname + ".gr";
  string copath = path + "/" + mname + ".co";
  vector<xyLoc> coord;
  ListGraph listg = RoadNetwork::Load(grpath, copath, coord);
  Mapper mapper(listg, coord);
  NodeOrdering order;
  if (fractal)
    order = compute_fractal_order(coord);
  else
    order = compute_real_dfs_order(listg);
  mapper.reorder(order);
  Dijkstra dij(mapper.g, mapper);
  Sectors sectors;
  auto allowed = dij.run(sid, hlevel, sectors, ext);
  CPD cpd;
  cpd.append_row(sid, allowed, sectors, mapper);
  int cnth = 0, cntsh = 0, cntss = 0, cntw = 0, cntd = 0;
  int entry_h = cpd.get_mask_entry(0, warthog::HMASK);
  int entry_sh = cpd.get_mask_entry(0, warthog::SHMASK);
  int entry_ss = cpd.get_mask_entry(0, warthog::SSMASK);

  string header = "s,t,lats,lons,latt,lont,color,desc";
  string rgb = "rgb";
  int pos[] = {16, 8, 0};
  printf("%s\n", header.c_str());
  xyLoc sloc = mapper(sid);
  double lats = sloc.y / 1000000.0;
  double lons = sloc.x / 1000000.0;
  for (int id=0; id<(int)listg.node_count(); id++) if (id != sid) {
    xyLoc tloc = mapper(id);
    double latt = tloc.y / 1000000.0;
    double lont = tloc.x / 1000000.0;
    int mask = allowed[id];
    int color = 0;
    string col_desc = "";
    bool in_ext_h = false;
    bool in_ext_s = false;
    bool in_sector = false;
    if (mask & warthog::HMASK) {
      mask ^= warthog::HMASK;
      cnth++;
    }
    while (mask) {
      int lowbit = warthog::lowb(mask);
      int d = warthog::m2i.at(lowbit);
      color += 100 << pos[d % 3] ;
      mask -= lowbit;
      col_desc += rgb[d % 3];
    }

    long double rad = Geo::angle_ccw({tloc.x - sloc.x, tloc.y - sloc.y});
    int d = Sectors::find_closest_sector(sectors, rad);
    if (sectors.is_in_sector(d, rad))
      in_sector = true, cntw++;
    else if (allowed[id] & warthog::SHMASK)
      in_ext_h = true, cntsh++;
    else if (allowed[id] & warthog::SSMASK)
      in_ext_s = true, cntss++;
    else cntd ++;

    if (Sectors::find_sector(sectors, sloc, tloc) != -1)
      color = 0x000000;
    else if (in_ext_h) color = 0x444444;
    else if (in_ext_s) color = 0x888888;
    printf("%d,%d,%.6lf,%.6lf,%.6lf,%.6lf,%d,%s\n", sid, id, lats, lons, latt, lont, color, col_desc.c_str());
  }

  cerr << "row length: " << cpd.get_row_length(0) << endl;
  cerr << "cnth: " << cnth << ", cntsh: " << cntsh << ", cntss: " << cntss << ", cntw: " << cntw << ", cntd: " << cntd << endl;
  cerr << "entry_h: " << entry_h << ", entry_sh: " << entry_sh << ", entry_ss: " << entry_ss << endl;
}

int main(int argv, char* args[]) {
  bool co = false, gr = false, cnth = false, order = false, cpdcoord = false,
       desc = false, info = false, sector = false, ext_sector = false, fmove = false, fractal = false,
       row = false;
  p2p_num = 0;
  sid = 0;
  index_path = "";
  for (int i=1; i<argv; i++) {
    if (strcmp(args[i], "-r") == 0) {
      r = atof(args[i+1]);
    }
    if (strcmp(args[i], "-p") == 0) {
      path = string(args[i+1]);
    }
    if (strcmp(args[i], "-co") == 0) {
      co = true; 
    }
    if (strcmp(args[i], "-gr") == 0) {
      gr = true;
    }
    if (strcmp(args[i], "-p2p") == 0) {
      p2p_num = atoi(args[i+1]);
      i++;
    }
    if (strcmp(args[i], "-i") == 0) {
      index_path = string(args[i+1]);
    }
    if (strcmp(args[i], "-h") == 0) {
      hlevel = atoi(args[i+1]);
      i++;
    }
    if (strcmp(args[i], "-s") == 0) {
      sid = atoi(args[i+1]);
      i++;
    }
    if (strcmp(args[i], "-cnt") == 0) {
      cnth = true;
    }
    if (strcmp(args[i], "-order") == 0) {
      order = true;
    }
    if (strcmp(args[i], "-cpdcoord") == 0) {
      cpdcoord = true;
    }
    if (strcmp(args[i], "-desc") == 0) {
      desc = true;
    }
    if (strcmp(args[i], "-info") == 0) {
      info = true;
    }
    if (strcmp(args[i], "-sector") == 0) {
      sector = true;
    }
    if (strcmp(args[i], "-ext-sector") == 0) {
      sector = true, ext_sector = true;
    }
    if (strcmp(args[i], "-fmove") == 0) {
      fmove = true;
    }
    if (strcmp(args[i], "-fractal") == 0) {
      fractal = true;
    }
    if (strcmp(args[i], "-row") == 0) {
      row = true;
    }
  }
  if (gr) genSubGr();
  else if (co) genSubCo();
  else if (p2p_num) genScenario();
  else if (info && index_path != "") genCPDInfo();
  else if (order) genCoordOrderCSV(fractal);
  else if (cpdcoord && index_path != "") {
    if (sector)
      genCPDSectorCoord();
    else
      genCPDCoord();
  }
  else if (desc && index_path != "") {
    genCPDDesc();
  }
  else if (fmove && index_path != "") {
    genFirstMove();
  }
  else if (row) {
    if (!desc)
      genCPDSingleRow(fractal, ext_sector);
    else
      genCPDSingleRowDesc(fractal, ext_sector);
  }
}
