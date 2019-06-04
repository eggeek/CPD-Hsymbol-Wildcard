#include <deque>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <iomanip>
#include "Entry.h"
#include "timer.h"
#include "list_graph.h"
#include "mapper.h"
#include "cpd.h"
#include "order.h"
#include "adj_graph.h"
#include "dijkstra.h"
#include "constants.h"
#include "balanced_min_cut.h"
#include "prefer_zero_cut.h"
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
  #ifndef USE_CUT_ORDER
  NodeOrdering order = compute_real_dfs_order(extract_graph(mapper));
  #else
  NodeOrdering order = compute_cut_order(extract_graph(mapper), prefer_zero_cut(balanced_min_cut));
  #endif
  mapper.reorder(order);

  printf("Computing Row Order\n");
  AdjGraph g(extract_graph(mapper));
  vector<int> row_ordering;
  row_ordering = order.get_old_array();

  printf("Computing first-move matrix, hLevel: %d\n", hLevel);
  vector<int> square_sides;
  CPD cpd;
  {
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

      const auto&allowed = dij.run(source_node, hLevel, square_sides);
      cpd.append_row(source_node, allowed, row_ordering, mapper, square_sides[source_node]);
    }
    #else
    printf("Using %d threads\n", omp_get_max_threads());
    vector<CPD>thread_cpd(omp_get_max_threads());
    vector<vector<int>> thread_square_side(omp_get_max_threads());

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
      Mapper thread_mapper = mapper;

      for(int source_node=node_begin; source_node < node_end; ++source_node){
        vector<unsigned short> allowed = thread_dij.run(source_node, hLevel, thread_square_side[thread_id]);
        thread_cpd[thread_id].append_row(source_node, allowed, row_ordering, thread_mapper, *(thread_square_side[thread_id].end()-1));
        #pragma omp critical 
        {
          ++progress;
          if(progress % 100 == 0) {
            double ratio = (double)progress / g.node_count() * 100.0;
            cout << "Progress: [" << progress << "/" << g.node_count() << "] "
                 << setprecision(3) << ratio << "% \r";
            cout.flush();
          }
        }
      }
    }

    for(auto&x:thread_cpd)
      cpd.append_rows(x);
    for(auto&x:thread_square_side)
      square_sides.insert(square_sides.end(), x.begin(), x.end());
    #endif
  }

  printf("Saving data to %s\n", filename);
  printf("begin size: %d, entry size: %d\n", cpd.get_begin_size(), cpd.get_entry_size());
  FILE*f = fopen(filename, "wb");
  save_vector(f, square_sides);
  order.save(f);
  cpd.save(f);
  save_vector(f, row_ordering);
  fclose(f);
  printf("Done\n");

}

struct State{
  CPD cpd;
  Mapper mapper;
  AdjGraph graph;
  vector<int> row_ordering;
  vector<int> square_sides;
  vector<unsigned char> reverse_moves;
  int current_node;
  int target_node;
};

void *PrepareForSearch(std::vector<bool> &bits, int w, int h, const char *filename)
{
  printf("Loading preprocessing data\n");
  State*state = new State;
  
  state->mapper = Mapper(bits, w, h);

  FILE*f = fopen(filename, "rb");
  state->square_sides = load_vector<int>(f);
  NodeOrdering order;
  order.load(f);
  state->cpd.load(f);
  state->row_ordering = load_vector<int>(f);
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
  else if (hLevel == 3)
    heuristic_func = Hsymbol::get_heuristic_move3;

  int current_source = state->mapper(s);
  int current_target = state->mapper(t);
  const int16_t* dx = warthog::dx;
  const int16_t* dy = warthog::dy;
  double cost = 0.0;
  int steps = 0;

  auto is_in_square = [&](int current_source, int current_target)
  {
      int side = state->square_sides[current_source];
      xyLoc loc_source = state->mapper.operator()(current_source);
      xyLoc loc_x = state->mapper.operator()(current_target);
      int dx = iabs(loc_source.x - loc_x.x);
      int dy = iabs(loc_source.y- loc_x.y);
      if(( (dx<<1) <= (side-1)) && ( (dy<<1) <= (side-1)))
      {
        return true;
      }
      return false;
  };

  auto next_move = [&](int current_source, int current_target)
  {
      xyLoc cs = state->mapper.operator()(current_source);
      xyLoc ct = state->mapper.operator()(current_target);
      int vx = signbit(ct.x - cs.x);
      int vy = signbit(ct.y - cs.y);
      return warthog::v2i[1+vx][1+vy];
  };

  while (current_source != current_target) {
    int move;
    if(state->row_ordering[current_source] >= state->row_ordering[current_target])
    {
      if(is_in_square(current_source, current_target))
      {
        move = next_move(current_source, current_target);
      }
      else
      {
        move = state->cpd.get_first_move(current_source, current_target);
      }
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
    else
    {
      if(is_in_square(current_target, current_source))
      {
        move = next_move(current_target, current_source);
      }
      else
      {
        move = state->cpd.get_first_move(current_target, current_source);
      }
      if (move == 0xF) break;
      if ((1 << move) == warthog::HMASK) {
        move = H::decode(current_target, current_source, state->mapper, heuristic_func);
      }
      cost += warthog::doublew[move];
      t.x += dx[move];
      t.y += dy[move];
      steps ++;
      if (limit != -1 && limit <= steps)
        break;
      current_target = state->mapper(t);
    }

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
