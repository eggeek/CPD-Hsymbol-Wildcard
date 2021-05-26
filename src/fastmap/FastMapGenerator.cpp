#include "FastMapGenerator.h"

FastMapHeuristic* FastMapGenerator::CalcHeuristic(AdjGraph* g, int max_dim, double threshold) {
  this->g = g;
  fmh = new FastMapHeuristic(g->node_count(), use_l1_dist);
  gval.resize(g->node_count());
  d1.resize(g->node_count());
  d2.resize(g->node_count());
  q = min_id_heap<double>(g->node_count());
  srand(7);
  while(max_dim > 0 && AddDimension(threshold)) max_dim--;
  return fmh;
}

bool FastMapGenerator::AddDimension(double threshold) {
  int n1 = 0, n2 = 0;
  GetFarthestPair(n1, n2);

  CalcReduceDistFromNode(n1);
  d1 = vector<double>(gval);

  CalcReduceDistFromNode(n2);
  d2 = vector<double>(gval);
  
  double d12 = d2[n1];
  if (d12 < threshold)
    return false;

  for (int i=0; i<g->node_count(); i++) {
    double di1 = d1[i];
    double di2 = d2[i];
    double c   = -1;
    if (di1 < INF - EPS && di2 < INF - EPS) {
      if (use_l1_dist) c = (di1 - di2 + d2[n1]) / 2.0;
      else c = (di1*di1 + d12*d12 - di2*di2) / (2.0 * d12);
    }
    else c = INF;
    fmh->AddCoordinateToNode(i, c);
  }
  fmh->IncrementNumCoord();
  fmh->AddPivotPair(n1, n2);
  return true;
}

void FastMapGenerator::GetFarthestPair(int& n1, int& n2, int nRestart, int nFlips) {
  double bestd = 0;
  n1 = n2 = kNonNode;

  int t1, t2, prev_t1;
  for (int i=0; i<nRestart; i++) {
    // assuming there is only one connected component in road network
    t1 = rand() % g->node_count();

    prev_t1 = t1;
    for (int j=0; j<nFlips; j++) {
      t2 = GetFarthestNode(t1);
      if (t2 == prev_t1) break;
      else {
        prev_t1 = t1;
        t1 = t2;
        t2 = prev_t1;
      }
    }

    double d = gval[t2];
    if (d > bestd && d < INF - EPS) {
      bestd = d; n1 = t1; n2 = t2;
    }
  }
}

int FastMapGenerator::GetFarthestNode(int n1) {
  int res = n1;
  double d = 0;
  CalcReduceDistFromNode(n1);
  for (int n2=0; n2 < g->node_count(); n2++) {
    if (gval[n2] > d && d < INF - EPS) {
      d = gval[n2];
      res = n2;
    }
  }
  return res;
}

void FastMapGenerator::CalcReduceDistFromNode(int s) {
  fill(gval.begin(), gval.end(), INF);
  q.clear();

  gval[s] = 0;
  q.push_or_decrease_key(s, 0);
  while (!q.empty()) {
    int c = q.pop();
    for (const auto& a: g->out(c)) {
      double elen = a.weight;
      if (use_reduced_edge_len)
        elen = GetReducedDist(c, a.target, elen);
      double new_gval = gval[c] + elen;
      if (new_gval < gval[a.target]) {
        gval[a.target] = new_gval;
        q.push_or_decrease_key(a.target, new_gval);
      }
    }
  }
}
