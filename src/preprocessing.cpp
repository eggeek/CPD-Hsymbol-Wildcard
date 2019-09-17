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
#include "order.h"
#include "adj_graph.h"
#include "dijkstra.h"
#include "constants.h"
#include "balanced_min_cut.h"
#include "prefer_zero_cut.h"
#include "centroid.h"
#include "cpd_base.h"
#include "cpd_rect.h"
#include "cpd_centroid.h"
#include <cstdio>

using namespace std;

NodeOrdering compute_ordering(const Mapper& mapper, const Parameters& p) {
  NodeOrdering order;
  if (p.otype == "DFS")
    order = compute_real_dfs_order(extract_graph(mapper));
  else if (p.otype == "CUT")
    order = compute_cut_order(extract_graph(mapper), prefer_zero_cut(balanced_min_cut));
  else if (p.otype == "SPLIT")
    order = compute_split_dfs_order(extract_graph(mapper));
  else if (p.otype == "FRACTAL") {
    vector<xyLoc> nodes(mapper.node_count());
      for (int i=0; i<mapper.node_count(); i++) nodes[i] = mapper(i);
    order = compute_fractal_order(nodes);
  }
  return order;
}

void PreprocessMap(std::vector<bool> &bits, int width, int height, const Parameters& p)
{
  using CPD = CPDBASE;
  using CPD_INV = CPDBASE;
  Mapper mapper(bits, width, height);
  printf("width = %d, height = %d, node_count = %d\n", width, height, mapper.node_count());

  printf("Computing node order\n");
  NodeOrdering order = compute_ordering(mapper, p);
  mapper.reorder(order);

  printf("Computing Row Order\n");
  AdjGraph g(extract_graph(mapper));
  vector<int> row_ordering;
  row_ordering = order.get_old_array();

  printf("Computing first-move matrix, hLevel: %d\n", p.hLevel);
  vector<int> square_sides;
  CPD cpd;
  CPD_INV inv_cpd;
  {
    {
      Dijkstra dij(g, mapper);
      warthog::timer t;
      t.start();
      dij.run(0, p.hLevel);
      t.stop();
      double tots = t.elapsed_time_micro()*g.node_count() / 1000000;
      printf("Estimated sequential running time : %fmin\n", tots / 60.0);
    }

    printf("Using %d threads\n", omp_get_max_threads());
    vector<CPD>thread_cpd(omp_get_max_threads());
    vector<CPD_INV>thread_cpd_inv(omp_get_max_threads());
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
        thread_dij.run(source_node, p.hLevel, thread_square_side[thread_id]);
        thread_cpd[thread_id].append_row(source_node, thread_dij.get_allowed(), thread_mapper, *(thread_square_side[thread_id].end()-1));
        thread_cpd_inv[thread_id].append_row(source_node, thread_dij.get_inv_allowed(), thread_mapper, 0);
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
    for (auto&x: thread_cpd_inv)
      inv_cpd.append_rows(x);
    for(auto&x:thread_square_side)
      square_sides.insert(square_sides.end(), x.begin(), x.end());
  }

  printf("Saving data to %s\n", p.filename.c_str());
  printf("begin size: %d, entry size: %d\n", cpd.entry_count(), cpd.get_entry_size());
  FILE*f = fopen(p.filename.c_str(), "wb");
  Parameters p1 = {p.otype, p.itype, p.filename, p.hLevel, p.centroid};
  p1.save(f);
  save_vector(f, square_sides);
  order.save(f);
  cpd.save(f);
  fclose(f);
  printf("Done\n");

  string fname = p.filename + "-inv";
  printf("Saving data to %s\n", fname.c_str());
  printf("begin size: %d, entry size: %d\n", inv_cpd.entry_count(), inv_cpd.get_entry_size());
  f = fopen(fname.c_str(), "wb");
  Parameters p2 = {p.otype, "inv", p.filename, p.hLevel, p.centroid};
  p2.save(f);
  order.save(f);
  inv_cpd.save(f);
  fclose(f);
  printf("Done\n");
}

void PreprocessCentroid(vector<bool>& bits, int width, int height, const Parameters& p) {
  using CPD = CPD_CENTROID;
  using CPD_INV = CPD_CENTROID;
  Mapper mapper(bits, width, height);

  printf("Computing node order\n");
  NodeOrdering order = compute_ordering(mapper, p);
  mapper.reorder(order);

  printf("Computing centroids\n");
  vector<int> cents = compute_centroid(mapper, p.centroid);

  printf("Computing Row Order\n");
  AdjGraph g(extract_graph(mapper));
  vector<int> row_ordering;
  row_ordering = order.get_old_array();

  printf("width = %d, height = %d, node_count = %d\n", width, height, (int)cents.size());
  printf("Computing first-move matrix, hLevel: %d\n", p.hLevel);
  vector<int> square_sides;
  CPD cpd;
  CPD_INV inv_cpd;
  {
    {
      Dijkstra dij(g, mapper);
      warthog::timer t;
      t.start();
      dij.run(0, p.hLevel);
      t.stop();
      double tots = t.elapsed_time_micro()*cents.size() / 1000000.0;
      printf("Estimated sequential running time : %fmin\n", tots / 60.0);
    }

    printf("Using %d threads\n", omp_get_max_threads());
    vector<CPD>thread_cpd(omp_get_max_threads());
    vector<CPD_INV>thread_cpd_inv(omp_get_max_threads());
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

      for(int source_node=node_begin; source_node < node_end; source_node++){
        thread_dij.run(source_node, p.hLevel, thread_square_side[thread_id]);
        thread_cpd[thread_id].append_row_forward(source_node, thread_dij.get_allowed(), thread_mapper, *(thread_square_side[thread_id].end()-1));
        if (mapper.get_fa()[source_node] == source_node) {
          thread_cpd_inv[thread_id].append_row(source_node, thread_dij.get_inv_allowed(), thread_mapper, 0);
        }
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
    for (auto&x: thread_cpd_inv)
      inv_cpd.append_rows(x);
    for(auto&x:thread_square_side)
      square_sides.insert(square_sides.end(), x.begin(), x.end());
  }

  printf("Saving data to %s\n", p.filename.c_str());
  printf("begin size: %d, entry size: %d\n", cpd.entry_count(), cpd.get_entry_size());
  FILE*f = fopen(p.filename.c_str(), "wb");
  Parameters p1 = {p.otype, p.itype, p.filename, p.hLevel, p.centroid};
  p1.save(f);
  save_vector(f, square_sides);
  save_vector(f, mapper.get_fa());
  order.save(f);
  cpd.save(f);
  fclose(f);
  printf("Done\n");

  string fname = p.filename + "-inv";
  printf("Saving data to %s\n", fname.c_str());
  printf("begin size: %d, entry size: %d\n", inv_cpd.entry_count(), inv_cpd.get_entry_size());
  f = fopen(fname.c_str(), "wb");
  Parameters p2 = {p.otype, "inv", p.filename, p.hLevel, p.centroid};
  p2.save(f);
  save_vector(f, mapper.get_fa());
  order.save(f);
  inv_cpd.save(f);
  fclose(f);
  printf("Done\n");
}


void PreprocessRevCentroid(vector<bool>& bits, int width, int height, const Parameters& p) {
  using CPD_INV = CPD_CENTROID;
  Mapper mapper(bits, width, height);

  printf("Computing node order\n");
  NodeOrdering order = compute_ordering(mapper, p);

  mapper.reorder(order);
  printf("Computing centroids\n");
  vector<int> cents = compute_centroid(mapper, p.centroid);

  printf("Computing Row Order\n");
  AdjGraph g(extract_graph(mapper));
  vector<int> row_ordering;
  row_ordering = order.get_old_array();

  printf("width = %d, height = %d, node_count = %d\n", width, height, (int)cents.size());
  printf("Computing first-move matrix, hLevel: %d\n", p.hLevel);
  vector<int> square_sides;
  CPD_INV inv_cpd;
  {
    {
      Dijkstra dij(g, mapper);
      warthog::timer t;
      t.start();
      dij.run(0, p.hLevel);
      t.stop();
      double tots = t.elapsed_time_micro()*cents.size() / 1000000.0;
      printf("Estimated sequential running time : %fmin\n", tots / 60.0);
    }

    printf("Using %d threads\n", omp_get_max_threads());
    vector<CPD_INV>thread_cpd_inv(omp_get_max_threads());
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

      for(int source_node=node_begin; source_node < node_end; source_node++){
        if (mapper.get_fa()[source_node] == source_node) {
          thread_dij.run(source_node, p.hLevel, thread_square_side[thread_id]);
          thread_cpd_inv[thread_id].append_row(source_node, thread_dij.get_inv_allowed(), thread_mapper, 0);
        }
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

    for (auto&x: thread_cpd_inv)
      inv_cpd.append_rows(x);
  }

  FILE*f;
  string fname = p.filename + "-inv";
  printf("Saving data to %s\n", fname.c_str());
  printf("begin size: %d, entry size: %d\n", inv_cpd.entry_count(), inv_cpd.get_entry_size());
  f = fopen(fname.c_str(), "wb");
  Parameters p2 = {p.otype, "inv", p.filename, p.hLevel, p.centroid};
  p2.save(f);
  save_vector(f, mapper.get_fa());
  order.save(f);
  inv_cpd.save(f);
  fclose(f);
  printf("Done\n");
}
