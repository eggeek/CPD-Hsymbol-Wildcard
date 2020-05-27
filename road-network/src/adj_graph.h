#ifndef ADJ_GRAPH_H 
#define ADJ_GRAPH_H

#include "range.h"
#include "list_graph.h"
#include "order.h"

using namespace std;

struct OutArc{
  int target;
  long long weight;
  int direction;
};

class AdjGraph{
public:
  AdjGraph(){}

  AdjGraph(ListGraph g){
    out_arcs.resize(g.node_count());
    for (const auto& it: g.arc) {
      out_arcs[it.source].push_back(OutArc{it.target, it.weight, it.direction});
    }
  }

  AdjGraph&operator=(const ListGraph&o){
    return *this = AdjGraph(o);
  }

  int node_count()const{
    return out_arcs.size();
  }

  Range<std::vector<OutArc>::const_iterator>out(int v)const{
    return make_range(out_arcs[v].begin(), out_arcs[v].end());
  }

  OutArc out(int v, int i)const{
    return out_arcs[v][i];
  }

  int out_deg(int v)const{
    return out_arcs[v].size();
  }

  void reorder(const NodeOrdering& order) {
    vector<vector<OutArc>> old_arcs = out_arcs;
    for (int i=0; i < this->node_count(); i++) {
      out_arcs[order.to_new(i)] = old_arcs[i];
    }

    for (int i=0; i < this->node_count(); i++) {
      for (auto& it: out_arcs[i]) {
        it.target = order.to_new(it.target); 
      }
    };
  }

  int max_degree() {
    int res = (int)out_arcs[0].size();
    for (int i=1; i<(int)out_arcs.size(); i++) {
      res = max(res, (int)out_arcs[i].size());
    } 
    return res;
  }

  int max_degree_vert() {
    int maxd = max_degree();
    for (int i=0; i<(int)out_arcs.size(); i++) {
      if ((int)out_arcs[i].size() == maxd) return i;
    }
    return -1;
  }

private:
  vector<vector<OutArc>> out_arcs;
};

#endif


