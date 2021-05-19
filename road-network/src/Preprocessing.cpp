#include <deque>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include "Preprocessing.h"
#include "timer.h"
#include "list_graph.h"
#include "cpd.h"
#include "order.h"
#include "adj_graph.h"
#include "dijkstra.h"
#include "constants.h"
#include "sector_wildcard.h"

using namespace std;

#ifdef USE_PARALLELISM
#include <omp.h>
#endif

void PreprocessMapPurely(
  int order_code, const ListGraph& listg,
  const vector<xyLoc>& coord, const char* filename,
  int hLevel, long long quant
) {
  Mapper mapper(listg, coord);

  printf("Computing node order\n");
  NodeOrdering order;
  if (order_code == warthog::FRACTAL)
    order = compute_fractal_order(coord);
  else
    order = compute_real_dfs_order(listg);
  mapper.reorder(order);
  mapper.quant = quant;


  CPD cpd;
  {
    AdjGraph& g = mapper.g;
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
    #endif
  }

  int maxd = 0;
  for (int i=0; i<mapper.g.node_count(); i++) maxd = max(maxd, mapper.g.out_deg(i));

  printf("Saving data to %s\n", filename);
  printf("begin size: %d, entry size: %d\n", cpd.get_begin_size(), cpd.get_entry_size());
  printf("Max Degree: %d\n", maxd);
  FILE*f = fopen(filename, "wb");
  order.save(f);
  cpd.save(f);
  fclose(f);
  printf("Done\n");

 
}

void PreprocessMapWithSectorWildCard(
  int order_code, const ListGraph& listg, 
  const vector<xyLoc>& coord, const char *filename, int hLevel,
  bool ext, long long quant
) {
  Mapper mapper(listg, coord);

  printf("Computing node order: %s\n", order_code == warthog::FRACTAL?"fractal": "dfs");
  NodeOrdering order;
  if (order_code == warthog::FRACTAL)
    order = compute_fractal_order(coord);
  else
    order = compute_real_dfs_order(listg);
  mapper.reorder(order);
  mapper.quant = quant;
  vector<Sectors> sectors(mapper.g.node_count());

  CPD cpd;
  {
    AdjGraph& g = mapper.g;
    {
      Dijkstra dij(g, mapper);
      warthog::timer t;
      t.start();
      Sectors tmp;
      dij.run(0, hLevel, tmp);
      t.stop();
      double tots = t.elapsed_time_micro()*g.node_count() / 1000000;
      printf("Estimated sequential running time : %fmin\n", tots / 60.0);
    }

    #ifndef USE_PARALLELISM
    Dijkstra dij(g, mapper);
    for(int source_node=0; source_node < g.node_count(); ++source_node){
      if(source_node % (g.node_count()/10) == 0)
        printf("%d of %d done\n", source_node, g.node_count());

      const auto&allowed = dij.run(source_node, hLevel, sectors[source_node], ext);
      cpd.append_row(source_node, allowed, sectors[source_node], mapper);
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
        vector<unsigned short> allowed = thread_dij.run(source_node,
            hLevel, sectors[source_node], ext);
        thread_cpd[thread_id].append_row(source_node, allowed, sectors[source_node], mapper);
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
    #endif
  }

  int maxd = 0;
  for (int i=0; i<mapper.g.node_count(); i++) maxd = max(maxd, mapper.g.out_deg(i));

  printf("Saving data to %s\n", filename);
  printf("begin size: %d, entry size: %d\n", cpd.get_begin_size(), cpd.get_entry_size());
  printf("Max Degree: %d\n", maxd);
  FILE*f = fopen(filename, "wb");
  order.save(f);
  cpd.save(f);
  // sector wildcard
  for (int i=0; i<mapper.g.node_count(); i++) {
    sectors[i].save(f);
  }
  fclose(f);
  printf("Done\n");

}


void *PrepareForSearchPurely(
  const ListGraph& listg,
  const vector<xyLoc>& coord,
  const char *filename
)
{
  cerr << "Loading preprocessing data" << endl;
  State*state = new State;
  
  state->mapper = Mapper(listg, coord);

  FILE*f = fopen(filename, "rb");
  NodeOrdering order;
  order.load(f);
  state->cpd.load(f);
  fclose(f);

  state->mapper.reorder(order);
  state->graph = state->mapper.g;
  state->current_node = -1;

  cerr << "Loading done" << endl;
  return state;
}

void* PrepareForSearchSectorWildCard(
  const ListGraph& listg,
  const vector<xyLoc>& coord,
  const char* filename
)
{

  cerr << "Loading preprocessing data" << endl;
  State*state = new State;
  
  state->mapper = Mapper(listg, coord);

  FILE*f = fopen(filename, "rb");
  NodeOrdering order;
  order.load(f);
  state->cpd.load(f);
  state->sectors.resize(listg.node_count());
  for (int i=0; i<listg.node_count(); i++) {
    state->sectors[i].load(f);
  }
  fclose(f);

  state->mapper.reorder(order);
  state->graph = state->mapper.g;
  state->current_node = -1;

  cerr << "Loading done" << endl;
  return state;
}
