#include <deque>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <omp.h>
#include "preprocessing.h"
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

void PreprocessRectWildcard(vector<bool>& bits, int width, int height, const char* filename, int hLevel=1) {
  Mapper mapper(bits, width, height);
  printf("width = %d, height = %d, node_count = %d\n", width, height, mapper.node_count());

  printf("Computing node order\n");
  NodeOrdering order = compute_real_dfs_order(extract_graph(mapper));
  mapper.reorder(order);
  printf("Computing Row Order\n");
  AdjGraph g(extract_graph(mapper));
  vector<int> row_ordering;
  row_ordering = order.get_old_array();

  printf("Computing first-move matrix, hLevel: %d\n", hLevel);
  CPD cpd;
  map<int, vector<RectInfo>> used;
  vector<int> square_sides;
  {
    {
      Dijkstra dij(g, mapper);
      vector<RectInfo> rects;
      vector<unsigned short> tmpfmoves;
      CPD tmpcpd;
      warthog::timer t;
      t.start();
      tmpfmoves = dij.run(0, hLevel, rects, square_sides);
      tmpcpd.append_row(0, tmpfmoves, mapper, rects, row_ordering, square_sides.back());
      t.stop();
      square_sides.clear();
      double tots = t.elapsed_time_micro()*g.node_count() / 1000000;
      printf("Estimated sequential running time : %fmin\n", tots / 60.0);
    }

    printf("Using %d threads\n", omp_get_max_threads());
    vector<CPD>thread_cpd(omp_get_max_threads());
    vector<vector<RectInfo>> thread_rects(omp_get_max_threads());
    vector<map<int,vector<RectInfo>>> thread_used(omp_get_max_threads());
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
      Mapper thread_mapper = mapper;
      Dijkstra thread_dij(thread_adj_g, thread_mapper);

      for(int source_node=node_begin; source_node < node_end; ++source_node){
        vector<unsigned short> allowed = thread_dij.run(source_node, hLevel, thread_rects[thread_id],
            thread_square_side[thread_id]);
        vector<RectInfo> tmp_used = thread_cpd[thread_id].append_row(source_node, allowed, thread_mapper,
            thread_rects[thread_id], row_ordering, thread_square_side[thread_id].back());
        thread_used[thread_id][source_node].insert(thread_used[thread_id][source_node].end(), tmp_used.begin(), tmp_used.end());
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
    for (auto& x: thread_square_side)
      square_sides.insert(square_sides.end(), x.begin(), x.end());
    for(auto&x: thread_used) {
      for (const auto& it: x) {
        used[it.first].insert(used[it.first].end(), it.second.begin(), it.second.end());
      }
    }
  }

  printf("Saving data to %s\n", filename);
  printf("begin size: %d, entry size: %d\n", cpd.entry_count(), cpd.get_entry_size());
  FILE*f = fopen(filename, "wb");
  save_vector(f, square_sides);
  RectWildcardIndex rwobj(used);
  rwobj.save(f);
  order.save(f);
  cpd.save(f);
  save_vector(f, row_ordering);
  fclose(f);
  printf("Done\n");
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
  printf("begin size: %d, entry size: %d\n", cpd.entry_count(), cpd.get_entry_size());
  FILE*f = fopen(filename, "wb");
  save_vector(f, square_sides);
  order.save(f);
  cpd.save(f);
  save_vector(f, row_ordering);
  fclose(f);
  printf("Done\n");

}
