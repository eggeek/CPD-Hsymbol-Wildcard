#pragma once
#include "adj_graph.h"
#include "FastMapHeuristic.h"
#include "heap.h"


const int kFMMemoryPerNode = 10;

const bool kFMEightNeighbor = false;
const bool kFMDiagonalRule = false;

const bool kFMReduceEdgeLengths = true;
const bool kFMUseL1Distance = true;

const int kFMNumRestarts = 10;
const int kFMNumFlips = 10;
const double kFMThreshold = 0.5;

const int kFMMaxDimensions = kFMMemoryPerNode;
const int kDHNumPivots = kFMMemoryPerNode;
const int kNonNode = numeric_limits<int>::max();
const double INF = numeric_limits<double>::max();
const double EPS = 0.001;

class FastMapGenerator {

public:
  FastMapGenerator(bool reduced_edge_len = true, bool l1_dist = true): 
    g(NULL), fmh(NULL), use_reduced_edge_len(reduced_edge_len), use_l1_dist(l1_dist) {};

  FastMapHeuristic* CalcHeuristic(AdjGraph* g, int max_dim=kFMMaxDimensions, double threshold=kFMThreshold);

private:
  AdjGraph* g;
  FastMapHeuristic* fmh;
  bool use_reduced_edge_len, use_l1_dist;
  vector<double> gval, d1, d2;
  min_id_heap<double> q;

  bool AddDimension(double threshold=kFMThreshold);
  void GetFarthestPair(int& n1, int& n2, int num_restarts=kFMNumRestarts, int num_flips = kFMNumFlips);
  int GetFarthestNode(int n);
  void CalcReduceDistFromNode(int id);
  double GetReducedDist(int n1, int n2, double dist) {
    if (use_l1_dist)
      return max(0.0, dist - fmh->GetHeurDistance(n1, n2));
    else
      return max(0.0, sqrt(dist*dist - fmh->GetHeurDistance(n1, n2)*fmh->GetHeurDistance(n1, n2)));
  }
};
