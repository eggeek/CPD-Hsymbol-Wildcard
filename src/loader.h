#pragma once
#include <iostream>
#include <string>
#include <stdexcept>
#include "mapper.h"
#include "preprocessing.h"
using namespace std;

inline string getIndexName(string filename) {
  auto pos = filename.find_last_of('/');
  return filename.substr(pos + 1);
}

inline string getMapName(string filename) {
  auto pos = filename.find_last_of('/');
  const string mapfile = filename.substr(pos + 1);
  auto suff = mapfile.find('.');
  return mapfile.substr(0, suff);
}

inline void LoadMap(const char *fname, std::vector<bool> &map, int &width, int &height)
{
  FILE *f;
  f = fopen(fname, "r");
  if (f)
    {
    fscanf(f, "type octile\nheight %d\nwidth %d\nmap\n", &height, &width);
    map.resize(height*width);
    for (int y = 0; y < height; y++)
    {
      for (int x = 0; x < width; x++)
      {
        char c;
        do {
          fscanf(f, "%c", &c);
        } while (isspace(c));
        map[y*width+x] = (c == '.' || c == 'G' || c == 'S');
      }
    }
    fclose(f);
    }
}

inline void LoadFwdCPD(Index& data, FILE* f) {
  data.square_sides = load_vector<int>(f);
  NodeOrdering order;
  order.load(f);
  data.cpd.load(f);
  data.mapper.reorder(order);
  data.graph = AdjGraph(extract_graph(data.mapper));
}

inline void LoadFwdCsymbolCPD(Index& data, FILE* f) {
  vector<int> centroids;
  data.square_sides = load_vector<int>(f);
  centroids = load_vector<int>(f);
  NodeOrdering order;
  order.load(f);
  data.cpd.load(f);
  data.mapper.reorder(order);
  data.mapper.set_centroids(centroids);
  data.graph = AdjGraph(extract_graph(data.mapper));
}

inline void LoadFwdCentroidsCPD(Index& data, FILE* f) {
  vector<int> centroids;
  data.square_sides = load_vector<int>(f);
  centroids = load_vector<int>(f);
  NodeOrdering order;
  order.load(f);
  data.cpd.load(f);
  data.mapper.reorder(order);
  data.mapper.set_centroids(centroids);
  data.graph = AdjGraph(extract_graph(data.mapper));
}

inline void LoadRectWildCard(Index& data, FILE* f) {
  data.square_sides = load_vector<int>(f);
  data.rwobj.load(f);
  NodeOrdering order;
  order.load(f);
  data.cpd.load(f);
  data.row_ordering = load_vector<int>(f);
  data.mapper.reorder(order);
  data.graph = AdjGraph(extract_graph(data.mapper));
}

inline void LoadInvCPD(Index& data, FILE* f) {
  //data.square_sides = load_vector<int>(f);
  NodeOrdering order;
  order.load(f);
  data.cpd.load(f);
  data.mapper.reorder(order);
  data.graph = AdjGraph(extract_graph(data.mapper));
}


inline void LoadInvCentroidsCPD(Index& data, FILE* f) {
  vector<int> centroids;
  //data.square_sides = load_vector<int>(f);
  centroids = load_vector<int>(f);
  NodeOrdering order;
  order.load(f);
  data.cpd.load(f);

  data.mapper.reorder(order);
  data.mapper.set_centroids(centroids);
  data.graph = AdjGraph(extract_graph(data.mapper));
}

inline Index LoadIndexData(vector<bool>& bits, int w, int h, const char* fname) {
  cerr << "Loading preprocessing data" << endl;
  Index data;
  data.mapper = Mapper(bits, w, h);
  FILE* f = fopen(fname, "rb");
  data.p.load(f);

  if (data.p.itype == "fwd") {
    if (data.p.centroid) LoadFwdCentroidsCPD(data, f);
    else if (data.p.csymbol) LoadFwdCsymbolCPD(data, f);
    else LoadFwdCPD(data, f);
  }
  else if (data.p.itype == "inv") {
    if (data.p.centroid) LoadInvCentroidsCPD(data, f);
    else LoadInvCPD(data, f);
  }
  else if (data.p.itype == "rect") {
    if (data.p.centroid) LoadRectWildCard(data, f);
    else {
      throw runtime_error("Rect centroid is unimplemented.");
    }
  }
  else {
    throw runtime_error("unknown index type: \"" + data.p.itype + "\"");
  }

  fclose(f);
  cerr << "Loading done" << endl;
  return data;
}
