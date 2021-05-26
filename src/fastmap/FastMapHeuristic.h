#pragma once
#include <cmath>
#include <vector>

using namespace std;
class FastMapHeuristic {
public:
  FastMapHeuristic(int num_nodes, bool l1): use_l1(l1), num_coord(0) {
    coordinates = vector<vector<double>>(num_nodes, vector<double>());
    pivot_from.clear(); pivot_to.clear();
  }

  void AddCoordinateToNode(int nid, double coord) { coordinates[nid].push_back(coord); }
  void IncrementNumCoord() { num_coord++; }
  double GetHeurDistance(int nid1, int nid2) {
    return use_l1?GetL1Distance(nid1, nid2): GetL2Distance(nid1, nid2);
  }

  double GetL1Distance(int n1, int n2) {
    double d = 0;
    for (int c = 0; c < num_coord; c++)
      d += fabs((double)(coordinates[n1][c] - coordinates[n2][c]));
    return d;
  }

  double GetL2Distance(int n1, int n2) {
    double d = 0;
    for (int c = 0; c < num_coord; c++) {
      double diff = fabs((double)(coordinates[n1][c] - coordinates[n2][c]));
      d += diff*diff;
    }
    return sqrt(d);
  }

  void AddPivotPair(int from, int to) { 
    pivot_from.push_back(from); 
    pivot_to.push_back(to);
  }

  void GetPiviots(vector<int>& from, vector<int>& to) const {
    from = pivot_from;
    to = pivot_to;
  }

  bool use_l1;
  int num_coord;
  vector<vector<double>> coordinates;
  vector<int> pivot_from, pivot_to;
};
