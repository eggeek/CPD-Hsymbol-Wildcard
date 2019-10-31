#include <deque>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <omp.h>
#include <cstdio>
#include <chrono>
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


double evaluate_tcost(Dijkstra& dij, int source, int hLevel) {
  auto stime = std::chrono::steady_clock::now();
  dij.run(source, hLevel);
  auto etime = std::chrono::steady_clock::now();
  double tcost = std::chrono::duration_cast<std::chrono::nanoseconds>(etime - stime).count();
  return tcost / 1000000000.0;
}


void reportProgress(int& progress, int tot) {
  ++progress;
  if(progress % 100 == 0) {
    double ratio = (double)progress / tot * 100.0;
    cout << "Progress: [" << progress << "/" << tot << "] "
         << setprecision(3) << ratio << "% \r";
    cout.flush();
  }
}


void PreprocessFwd(Mapper& mapper, NodeOrdering& order, AdjGraph& g, const Parameters& p) {
  CPDBASE cpd;
  vector<int> square_sides;

  printf("Using %d threads\n", omp_get_max_threads());
  vector<CPD_CENTROID>thread_cpd(omp_get_max_threads());
  vector<vector<int>> thread_square_side(omp_get_max_threads());

  int progress = 0;

  #pragma omp parallel
  {
    const int thread_count = omp_get_num_threads();
    const int tid = omp_get_thread_num();
    const int node_count = g.node_count();

    int node_begin = (node_count*tid) / thread_count;
    int node_end = (node_count*(tid+1)) / thread_count;

    AdjGraph thread_adj_g(g);
    Dijkstra thread_dij(thread_adj_g, mapper);
    Mapper thread_mapper = mapper;

    for(int source_node=node_begin; source_node < node_end; source_node++) {
      thread_dij.run(source_node, p.hLevel, thread_square_side[tid]);
      thread_cpd[tid].append_row(source_node, thread_dij.get_allowed(), thread_mapper, thread_square_side[tid].back());
      #pragma omp critical 
      reportProgress(progress, g.node_count());
    }
  }

  for(auto&x:thread_cpd)
    cpd.append_rows(x);
  for(auto&x:thread_square_side)
    square_sides.insert(square_sides.end(), x.begin(), x.end());

  printf("Saving data to %s\n", p.filename.c_str());
  printf("begin size: %d, entry size: %d\n", cpd.entry_count(), cpd.get_entry_size());
  FILE*f = fopen(p.filename.c_str(), "wb");
  p.save(f);
  save_vector(f, square_sides);
  order.save(f);
  cpd.save(f);
  fclose(f);
  printf("Done\n");
}


void PreprocessFwdCentroid(Mapper& mapper, NodeOrdering& order, vector<int>& cents, AdjGraph& g, const Parameters& p) {
  CPD_CENTROID cpd;
  vector<int> square_sides;
  vector<int> cents_square_sides;

  printf("Using %d threads\n", omp_get_max_threads());

  vector<CPD_CENTROID> cents_cpdT(omp_get_max_threads());
  vector<vector<int>>  cents_square_sideT(omp_get_max_threads());

  vector<int> cents_rank(mapper.node_count(), -1);
  for (int i=0; i<(int)cents.size(); i++) cents_rank[cents[i]] = i;

  int progress = 0;

  vector<CPD_CENTROID>thread_cpd(omp_get_max_threads());
  vector<vector<int>> thread_square_side(omp_get_max_threads());

  printf("Computing rest of CPD\n");
  progress = 0;
  #pragma omp parallel
  {
    const int thread_count = omp_get_num_threads();
    const int tid = omp_get_thread_num();
    const int node_count = g.node_count();

    int node_begin = (node_count*tid) / thread_count;
    int node_end = (node_count*(tid+1)) / thread_count;

    AdjGraph thread_adj_g(g);
    Dijkstra thread_dij(thread_adj_g, mapper);
    const Mapper& thread_mapper = mapper;

    for(int source_node=node_begin; source_node < node_end; source_node++) {
      thread_dij.run(source_node, p.hLevel, thread_square_side[tid]);
      vector<unsigned short> fmoves = thread_dij.get_allowed();
      thread_cpd[tid].append_row_forward(source_node, fmoves, thread_mapper, thread_square_side[tid].back());
      #pragma omp critical 
      reportProgress(progress, g.node_count());
    }
  }

  for(auto&x:thread_cpd)
    cpd.append_rows(x);
  for(auto&x:thread_square_side)
    square_sides.insert(square_sides.end(), x.begin(), x.end());

  printf("Saving data to %s\n", p.filename.c_str());
  printf("begin size: %d, entry size: %d\n", cpd.entry_count(), cpd.get_entry_size());
  FILE*f = fopen(p.filename.c_str(), "wb");
  p.save(f);
  save_vector(f, square_sides);
  save_vector(f, mapper.get_fa());
  order.save(f);
  cpd.save(f);
  fclose(f);
  printf("Done\n");
}


void PreprocessRev(Mapper& mapper, NodeOrdering& order, AdjGraph& g, const Parameters& p) {
  CPDBASE cpd;
  Dijkstra temp_dij(g, mapper);
  vector<CPDBASE>thread_cpd(omp_get_max_threads());
  int progress = 0;
  #pragma omp parallel
  {
    const int thread_count = omp_get_num_threads();
    const int tid = omp_get_thread_num();
    const int node_count = g.node_count();

    int node_begin = (node_count*tid) / thread_count;
    int node_end = (node_count*(tid+1)) / thread_count;

    AdjGraph thread_adj_g(g);
    Dijkstra thread_dij(thread_adj_g, mapper);
    Mapper thread_mapper = mapper;

    for(int source_node=node_begin; source_node < node_end; source_node++){
      thread_dij.run_extra(source_node, p.hLevel);
      thread_cpd[tid].append_row(source_node, thread_dij.get_inv_allowed(), thread_mapper, 0);
      #pragma omp critical 
      reportProgress(progress, g.node_count());
    }
  }
  for (auto& x: thread_cpd)
    cpd.append_rows(x);

  string fname = p.filename + "-inv";
  printf("Saving data to %s\n", fname.c_str());
  printf("begin size: %d, entry size: %d\n", cpd.entry_count(), cpd.get_entry_size());
  FILE* f = fopen(fname.c_str(), "wb");
  p.save(f);
  order.save(f);
  cpd.save(f);
  fclose(f);
  printf("Done\n");
}

void PreprocessRevCentroid(Mapper& mapper, NodeOrdering& order, vector<int>& cents, AdjGraph& g,
    const Parameters& p) {
  CPD_CENTROID cpd;

  printf("Using %d threads\n", omp_get_max_threads());
  vector<CPD_CENTROID>thread_cpd(omp_get_max_threads());

  int progress = 0;

  #pragma omp parallel
  {
    const int thread_count = omp_get_num_threads();
    const int tid = omp_get_thread_num();
    const int node_count = g.node_count();

    int node_begin = (node_count*tid) / thread_count;
    int node_end = (node_count*(tid+1)) / thread_count;

    AdjGraph thread_adj_g(g);
    Dijkstra thread_dij(thread_adj_g, mapper);
    Mapper thread_mapper = mapper;

    for(int source_node=node_begin; source_node < node_end; source_node++){
      if (mapper.get_fa()[source_node] == source_node) {
        thread_dij.run_extra(source_node, p.hLevel);
        thread_cpd[tid].append_row(source_node, thread_dij.get_inv_allowed(), thread_mapper, 0);
        #pragma omp critical 
        reportProgress(progress, cents.size());
      }
    }
  }

  for (auto&x: thread_cpd)
    cpd.append_rows(x);

  FILE*f;
  string fname = p.filename + "-inv";
  printf("Saving data to %s\n", fname.c_str());
  printf("begin size: %d, entry size: %d\n", cpd.entry_count(), cpd.get_entry_size());
  f = fopen(fname.c_str(), "wb");
  p.save(f);
  save_vector(f, mapper.get_fa());
  order.save(f);
  cpd.save(f);
  fclose(f);
  printf("Done\n");
}

void add_centroid_symbol(int source, vector<unsigned short>& fmoves,
    const Mapper& mapper, const vector<int>& cents_rank, const CPDBASE& cpd,
    const vector<int>& square_sides) {
  for (int i=0; i<mapper.node_count(); i++) {
    int cid = cents_rank[mapper.get_fa()[source]];
    int side = square_sides[cid];
    int m;
    if (cpd.is_in_square(i, side, mapper.get_fa()[source], mapper))
      m = warthog::m2i.at(warthog::HMASK);
    else 
      m = cpd.get_first_move(cid, i);
    if (fmoves[i] & (1<<m)) 
      fmoves[i] |= warthog::CENTMASK;
  }
}

void PreprocessFwdCsymbol(Mapper& mapper, const NodeOrdering& order, 
    vector<int>& cents, AdjGraph& g, const Parameters& p) {
  using CPD = CPDBASE;
  CPD cents_cpd;
  CPD cpd;
  vector<int> square_sides;
  vector<int> cents_square_sides;
  vector<CPD> cents_cpdT(omp_get_max_threads());
  vector<vector<int>>  cents_square_sideT(omp_get_max_threads());

  vector<int> cents_rank(mapper.node_count(), -1);
  for (int i=0; i<(int)cents.size(); i++) cents_rank[cents[i]] = i;
  
  int progress = 0;

  printf("Computing CPD of Centroids.\n");
  #pragma omp parallel
  {
    const int thread_count = omp_get_num_threads();
    const int tid = omp_get_thread_num();
    const int node_count = cents.size();

    int node_begin = (node_count*tid) / thread_count;
    int node_end = (node_count*(tid+1)) / thread_count;

    AdjGraph thread_adj_g(g);
    Dijkstra thread_dij(thread_adj_g, mapper);
    const Mapper& thread_mapper = mapper;

    for (int i=node_begin; i<node_end; i++) {
      thread_dij.run(cents[i], p.hLevel, cents_square_sideT[tid]);
      cents_cpdT[tid].append_row(cents[i], thread_dij.get_allowed(), thread_mapper, cents_square_sideT[tid].back());
      #pragma omp critical
      reportProgress(progress, cents.size());
    }
  }
  for (auto& x: cents_cpdT) cents_cpd.append_rows(x);
  for (auto& x: cents_square_sideT) cents_square_sides.insert(cents_square_sides.end(), x.begin(), x.end());

  vector<CPD> thread_cpd(omp_get_max_threads());
  vector<vector<int>> thread_square_side(omp_get_max_threads());

  printf("Computing rest of CPD\n");
  progress = 0;
  #pragma omp parallel
  {
    const int thread_count = omp_get_num_threads();
    const int tid = omp_get_thread_num();
    const int node_count = g.node_count();

    int node_begin = (node_count*tid) / thread_count;
    int node_end = (node_count*(tid+1)) / thread_count;
    AdjGraph thread_adj_g(g);
    Dijkstra thread_dij(thread_adj_g, mapper);
    const Mapper& thread_mapper = mapper;
    for (int source_node=node_begin; source_node < node_end; source_node++) {
      if (thread_mapper.get_fa()[source_node] == source_node) {
        assert(cents_rank[source_node] != -1);
        vector<int> compressed_row = cents_cpd.get_ith_compressed_row(cents_rank[source_node]);
        thread_square_side[tid].push_back(cents_square_sides[cents_rank[source_node]]);
        thread_cpd[tid].append_compressed_cpd_row(compressed_row);
      }
      else {
        thread_dij.run(source_node, p.hLevel, thread_square_side[tid]);
        vector<unsigned short> fmoves = thread_dij.get_allowed();
        add_centroid_symbol(source_node, fmoves, thread_mapper, cents_rank, cents_cpd, cents_square_sides);
        thread_cpd[tid].append_row(source_node, fmoves, thread_mapper, thread_square_side[tid].back());
      }
      #pragma omp critical
      reportProgress(progress, g.node_count());
    }
  }

  for (auto& x: thread_cpd) cpd.append_rows(x);
  for (auto& x: thread_square_side) square_sides.insert(square_sides.end(), x.begin(), x.end());

  printf("Saving data to %s\n", p.filename.c_str());
  printf("begin size: %d, entry size: %d\n", cpd.entry_count(), cpd.get_entry_size());
  FILE*f = fopen(p.filename.c_str(), "wb");
  p.save(f);
  save_vector(f, square_sides);
  save_vector(f, mapper.get_fa());
  order.save(f);
  cpd.save(f);
  fclose(f);
  printf("Done\n");
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

  vector<int> cents;
  if (p.centroid || p.csymbol) {
    printf("Computing centroids (centroid: %d, csymbol: %d)\n", p.centroid, p.csymbol);
    cents = compute_centroid(mapper, p.centroid | p.csymbol);
  }

  AdjGraph g(extract_graph(mapper));

  Dijkstra temp_dij(g, mapper);
  double tots = evaluate_tcost(temp_dij, 0, p.hLevel);
  if (p.centroid || p.csymbol) tots *= cents.size();
  else tots *= g.node_count();
  printf("Estimated sequential running time : %fmin\n", tots / 60.0);

  printf("Computing first-move matrix, hLevel: %d\n", p.hLevel);
  if (p.itype == "fwd") {
    vector<int> square_sides;
    if (p.centroid) PreprocessFwdCentroid(mapper, order, cents, g, p);
    else if (p.csymbol) PreprocessFwdCsymbol(mapper, order, cents, g, p);
    else PreprocessFwd(mapper, order, g, p);
  }
  else if (p.itype == "inv") {
    if (p.centroid) PreprocessRevCentroid(mapper, order, cents, g, p);
    else PreprocessRev(mapper, order, g, p);
  }
  else { assert(false); exit(1); }
}
