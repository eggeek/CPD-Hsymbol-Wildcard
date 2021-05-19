#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <map>
#include "list_graph.h"
#include "adj_graph.h"
#include "order.h"
#include "constants.h"
#include "jps.h"
#include "coord.h"
using namespace std;

class Mapper{
public:
  AdjGraph g;
  long long quant = 1;
  Mapper(){}
  Mapper(const ListGraph& listg, const vector<xyLoc>& coord):
    node_count_(listg.node_count()) {
      this->coord = coord;
      id2pos.clear();
      for (int i=0; i<node_count_; i++) {
        pos_to_node_[this->coord[i]] = i;
        id2pos[i] = i;
      }
      this->g = AdjGraph(listg);
  }

  int node_count()const{
    return node_count_;
  }

  xyLoc operator()(int x)const{ return coord[x]; }
  int operator()(xyLoc p)const{
    auto it = pos_to_node_.find(p);
    if (it == pos_to_node_.end()) return -1;
    else return it->second;
  }

  void reorder(const NodeOrdering&order){

    this->g.reorder(order);

    for (auto& it: pos_to_node_) {
      it.second = order.to_new(it.second);
    }

    vector<xyLoc>new_coord(node_count_);
    for(int new_node=0; new_node<node_count(); ++new_node){
      int old_node = order.to_old(new_node);
      new_coord[new_node] = coord[old_node];
      id2pos[old_node] = new_node;
    }
    new_coord.swap(coord);
  }

  int getPos(int id) {
    assert(id2pos.find(id) != id2pos.end());
    return id2pos[id];
  }

private:
  int node_count_;
  map<xyLoc, int> pos_to_node_;
  vector<xyLoc> coord;
  map<int, int> id2pos;
};
