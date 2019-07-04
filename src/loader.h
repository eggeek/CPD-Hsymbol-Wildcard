#pragma once
#include <iostream>
#include <string>
#include "mapper.h"
#include "preprocessing.h"
using namespace std;

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

inline Index PrepareForSearch(std::vector<bool> &bits, int w, int h, const char *filename)
{
  printf("Loading preprocessing data\n");
  Index state;
  state.mapper = Mapper(bits, w, h);
  FILE*f = fopen(filename, "rb");
  state.square_sides = load_vector<int>(f);
  NodeOrdering order;
  order.load(f);
  state.cpd.load(f);
  state.row_ordering = load_vector<int>(f);
  fclose(f);
  state.mapper.reorder(order);
  state.graph = AdjGraph(extract_graph(state.mapper));
  printf("Loading done\n");
  return state;
}

inline Index LoadVanillaCPD(vector<bool>& bits, int w, int h, const char* fname) {
  printf("Loading preprocessing data\n");
  Index state;
  state.mapper = Mapper(bits, w, h);
  FILE*f = fopen(fname, "rb");
  state.square_sides = load_vector<int>(f);
  NodeOrdering order;
  order.load(f);
  state.cpd.load(f);
  fclose(f);
  state.mapper.reorder(order);
  state.graph = AdjGraph(extract_graph(state.mapper));
  printf("Loading done\n");
  return state;
}

inline Index LoadRectWildCard(vector<bool>& bits, int w, int h, const char* fname) {
  printf("Loading preprocessing data\n");
  Index state;
  state.mapper = Mapper(bits, w, h);

  FILE* f = fopen(fname, "rb");
  state.square_sides = load_vector<int>(f);
  state.rwobj.load(f);
  NodeOrdering order;
  order.load(f);
  state.cpd.load(f);
  state.row_ordering = load_vector<int>(f);
  fclose(f);

  state.mapper.reorder(order);
  state.graph = AdjGraph(extract_graph(state.mapper));
  printf("Loading done\n");
  return state;
}

inline Index LoadInvCPD(vector<bool>& bits, int w, int h, const char* fname) {
  printf("Loading preprocessing data\n");
  Index data;
  data.mapper = Mapper(bits, w, h);
  FILE* f = fopen(fname, "rb");
  //data.square_sides = load_vector<int>(f);
  NodeOrdering order;
  order.load(f);
  data.cpd.load(f);
  fclose(f);
  data.mapper.reorder(order);
  data.graph = AdjGraph(extract_graph(data.mapper));
  printf("Loading done\n");
  return data;
}
