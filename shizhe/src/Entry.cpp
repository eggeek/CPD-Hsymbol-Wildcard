#include <deque>
#include <vector>
#include <algorithm>
#include <assert.h>
#include "Entry.h"
#include "timer.h"
#include "list_graph.h"
#include "mapper.h"
#include "cpd.h"
#include "order.h"
#include "adj_graph.h"
#include "dijkstra.h"
#include "constants.h"
#include <cstdio>

using namespace std;

#ifdef USE_PARALLELISM
#include <omp.h>
#endif

const char *GetName()
{
  #ifndef USE_CUT_ORDER
  return "DFS-SRC-RLE";
  #else
  return "METIS-CUT-SRC-RLE";
  #endif
}

void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename, int hLevel=1)
{
  Mapper mapper(bits, width, height);
  printf("width = %d, height = %d, node_count = %d\n", width, height, mapper.node_count());

  printf("Computing node order\n");
  NodeOrdering order = compute_real_dfs_order(extract_graph(mapper));
  mapper.reorder(order);


  printf("Computing first-move matrix, hLevel: %d\n", hLevel);

  CPD cpd;
  {
    AdjGraph g(extract_graph(mapper));
    
    {
      Dijkstra dij(g, mapper);
      warthog::timer t;
      t.start();
      dij.run(0, hLevel);
      t.stop();
      double tots = t.elapsed_time_micro()*g.node_count() / 1000000;
      printf("Estimated sequential running time : %fmin\n", tots / 60.0);
    }

    #ifndef USE_PARALLELISM
    Dijkstra dij(g, mapper);
    for(int source_node=0; source_node < g.node_count(); ++source_node){
      if(source_node % (g.node_count()/10) == 0)
        printf("%d of %d done\n", source_node, g.node_count());

      const auto&allowed = dij.run(source_node, hLevel);
      cpd.append_row(source_node, allowed);
    }
    #else
    printf("Using %d threads\n", omp_get_max_threads());
    vector<CPD>thread_cpd(omp_get_max_threads());

    int progress = 0;

    #pragma omp parallel
    {
      const int thread_count = omp_get_num_threads();
      const int thread_id = omp_get_thread_num();
      const int node_count = g.node_count();

      int node_begin = (node_count*thread_id) / thread_count;
      int node_end = (node_count*(thread_id+1)) / thread_count;

      AdjGraph thread_adj_g(g);
      Dijkstra thread_dij(thread_adj_g, mapper);
      for(int source_node=node_begin; source_node < node_end; ++source_node){
        thread_cpd[thread_id].append_row(source_node, thread_dij.run(source_node, hLevel));
        #pragma omp critical 
        {
          ++progress;
          if(progress % (g.node_count()/10) == 0){
            printf("%d of %d done\n", progress, g.node_count());
            fflush(stdout);
          }
        }
      }
    }

    for(auto&x:thread_cpd)
      cpd.append_rows(x);
    #endif
  }

  printf("Saving data to %s\n", filename);
  printf("begin size: %d, entry size: %d\n", cpd.get_begin_size(), cpd.get_entry_size());
  FILE*f = fopen(filename, "wb");
  order.save(f);
  cpd.save(f);
  fclose(f);
  printf("Done\n");

}

struct State{
  CPD cpd;
  Mapper mapper;
  AdjGraph graph;
  int current_node;
  int target_node;
};

void *PrepareForSearch(std::vector<bool> &bits, int w, int h, const char *filename)
{
  printf("Loading preprocessing data\n");
  State*state = new State;
  
  state->mapper = Mapper(bits, w, h);

  FILE*f = fopen(filename, "rb");
  NodeOrdering order;
  order.load(f);
  state->cpd.load(f);
  fclose(f);

  state->mapper.reorder(order);

  state->graph = AdjGraph(extract_graph(state->mapper));
  state->current_node = -1;

  printf("Loading done\n");


  return state;
}

double GetPathCostSRC(void *data, xyLoc s, xyLoc t, int hLevel, int limit) {
  State* state = static_cast<State*>(data);
  int (*heuristic_func)(int, int, const Mapper&);
  if (hLevel == 1)
    heuristic_func = Hsymbol::get_heuristic_move1;
  else if (hLevel == 2)
    heuristic_func = Hsymbol::get_heuristic_move2;

  int current_source = state->mapper(s);
  int current_target = state->mapper(t);
  const int16_t* dx = warthog::dx;
  const int16_t* dy = warthog::dy;
  double cost = 0.0;
  int steps = 0;
  while (current_source != current_target) {
    int move = state->cpd.get_first_move(current_source, current_target);
    // no path exist
    if (move == 0xF) break;
    if ((1 << move) == warthog::HMASK) {
      move = H::decode(current_source, current_target, state->mapper, heuristic_func);
    }
    cost += warthog::doublew[move];
    s.x += dx[move];
    s.y += dy[move];
    steps ++;
    if (limit != -1 && limit <= steps)
      break;
    current_source = state->mapper(s);
  }
  return cost;
}

double GetPath(void *data, xyLoc s, xyLoc t, std::vector<xyLoc> &path, warthog::jpsp_oracle& oracle,
    int hLevel, int limit)//, int &callCPD)
{
  State*state = static_cast<State*>(data);
  int current_source = state->mapper(s);
  int current_target = state->mapper(t);
  const int16_t* dx = warthog::dx;
  const int16_t* dy = warthog::dy;
  double cost = 0.0;
  int move = state->cpd.get_first_move(current_source, current_target);
  if ((1 << move) == warthog::HMASK) {
    move = H::decode(current_source, current_target, state->mapper, hLevel);
  }

  if(move != 0xF && current_source != current_target){
    oracle.set_goal_location(t.x,t.y);
    auto direction = (warthog::jps::direction)(1 << move);
    int number_step_to_turn = oracle.next_jump_point(s.x, s.y, direction);

    path.push_back(s);

    for(;;){

      for(int i = 0; i < number_step_to_turn; i++){
        s.x += dx[move];
        s.y += dy[move];
        cost += warthog::doublew[move];
        current_source = state->mapper(s);
        path.push_back(s);
        if(current_source == current_target)
          break;
        if (limit != -1 && (int)path.size() >= limit)
          break;
      }

      if(current_source == current_target)
        break;

      move = state->cpd.get_first_move(current_source, current_target);
      // decode the heuristic move
      if ((1 << move) == warthog::HMASK) {
        move = H::decode(current_source, current_target, state->mapper, hLevel);
      }
      direction = (warthog::jps::direction)(1 << move);
      number_step_to_turn = oracle.next_jump_point(s.x, s.y, direction);

    }
  }
  return cost;
}

void LoadMap(const char *fname, std::vector<bool> &map, int &width, int &height)
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
