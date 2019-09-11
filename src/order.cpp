#include "order.h"
#include "constants.h"
#include "coord.h"
#include <fstream>
#include <stdexcept>
#include <utility>
#include <vector>
#include <stack>

using namespace std;

// compile with -O3 -DNDEBUG

NodeOrdering compute_real_dfs_order(const ListGraph&g){

  std::vector<int>out_begin, out_dest;
  auto source_node = [&](int x){
    return g.arc[x].source;
  };

  auto target_node = [&](int x){
    return g.arc[x].target;
  };

  build_adj_array(
    out_begin, out_dest, 
    g.node_count(), g.arc.size(),
    source_node, target_node
  );

  NodeOrdering order(g.node_count());

  std::vector<bool>was_pushed(g.node_count(), false);
  std::vector<bool>was_popped(g.node_count(), false);
  std::vector<int>next_out = out_begin;
  std::stack<int>to_visit;
  int next_id = 0;
  for(int source_node=0; source_node<g.node_count(); ++source_node){
    if(!was_pushed[source_node]){

      to_visit.push(source_node);
      was_pushed[source_node] = true;

      while(!to_visit.empty()){
        int x = to_visit.top();
        to_visit.pop();
        if(!was_popped[x]){
          order.map(x, next_id++);
          was_popped[x] = true;
        }

        while(next_out[x] != out_begin[x+1]){
          int y = out_dest[next_out[x]];
          if(was_pushed[y])
            ++next_out[x];
          else{
            was_pushed[y] = true;
            to_visit.push(x);
            to_visit.push(y);
            ++next_out[x];
            break;
          }
        }
      }
    }
  }
  assert(order.is_complete());
  return order;
}

NodeOrdering compute_split_dfs_order(const ListGraph&g){

  vector<int>out_begin, out_dest;
  vector<int> mask(g.node_count());

  auto source_node = [&](int x){
    return g.arc[x].source;
  };

  auto target_node = [&](int x){
    return g.arc[x].target;
  };

  auto is_normal_tile = [&](int x) {
    return mask[x] == (warthog::STRAIGHTs | warthog::DIAGs);
  };

  build_adj_array(
    out_begin, out_dest, 
    g.node_count(), g.arc.size(),
    source_node, target_node
  );

  for (auto i: g.arc) {
    mask[i.source] |= 1<<i.direction;
  }

  NodeOrdering order(g.node_count());

  vector<bool>was_pushed(g.node_count(), false);
  vector<bool>was_popped(g.node_count(), false);
  vector<int>next_out = vector<int>(out_begin.begin(), out_begin.end());
  stack<int>to_visit;
  int next_id = 0;
  for(int source_node=0; source_node<g.node_count(); ++source_node){
    if(!was_pushed[source_node]){

      to_visit.push(source_node);
      was_pushed[source_node] = true;

      while(!to_visit.empty()){
        int x = to_visit.top();
        to_visit.pop();
        if(!was_popped[x] && is_normal_tile(x)){
          order.map(x, next_id++);
          was_popped[x] = true;
        }

        while(next_out[x] != out_begin[x+1]){
          int y = out_dest[next_out[x]];
          if(was_pushed[y])
            ++next_out[x];
          else{
            was_pushed[y] = true;
            to_visit.push(x);
            to_visit.push(y);
            ++next_out[x];
            break;
          }
        }
      }
    }
  }
  fill(was_pushed.begin(), was_pushed.end(), false);
  fill(was_popped.begin(), was_popped.end(), false);
  next_out = vector<int>(out_begin.begin(), out_begin.end());
  for (int source_node=0; source_node<g.node_count(); ++source_node) {
    if (!was_pushed[source_node]){

      to_visit.push(source_node);
      was_pushed[source_node] = true;

      while (!to_visit.empty()) {
        int x = to_visit.top();
        to_visit.pop();
        if (!was_popped[x] && !is_normal_tile(x)) {
          order.map(x, next_id++);
          was_popped[x] = true;
        }

        while (next_out[x] != out_begin[x+1]) {
          int y = out_dest[next_out[x]];
          if (was_pushed[y])
            ++next_out[x];
          else {
            was_pushed[y] = true;
            to_visit.push(x);
            to_visit.push(y);
            ++next_out[x];
            break;
          }
        }
      }
    }
  }

  assert(order.validate());
  return order;
}

void fractal_sort(
    int l, int r,
    std::vector<xyLoc>& nodes,
    std::vector<int>& ids)
{
  if (l >= r) return;
  int16_t xmin, xmax, ymin, ymax;
  xmin = xmax = nodes[l].x;
  ymin = ymax = nodes[l].y;
  for (int i=l; i<=r; i++) {
    xmin = min(xmin, nodes[i].x);
    xmax = max(xmax, nodes[i].x);
    ymin = min(ymin, nodes[i].y);
    ymax = max(ymax, nodes[i].y);
  }
  long long xmid = (xmax + xmin) / 2;
  long long ymid = (ymax + ymin) / 2;
  auto quat = [&](xyLoc node) {
    if (node.x <= xmid & node.y <= ymid) return 0;
    if (node.x >  xmid & node.y <= ymid) return 1;
    if (node.x >  xmid & node.y >  ymid) return 2;
    if (node.x <= xmid & node.y >  ymid) return 3;
    assert(false);
    return -1;
  };
  int idx = l;
  for (int i=l; i<=r; i++) {
    if (quat(nodes[i]) == 0) {
      if (idx != i) {
        swap(nodes[i], nodes[idx]);
        swap(ids[i], ids[idx]);
      }
      idx++;
    }
  }
  if (l < idx-1)
    fractal_sort(l, idx-1, nodes, ids);

  l = idx;
  for (int i=idx; i<=r; i++) {
    if (quat(nodes[i]) == 1) {
      if (idx != i) {
        swap(nodes[i], nodes[idx]);
        swap(ids[i], ids[idx]);
      }
      idx++;
    } 
  }
  if (l < idx-1)
    fractal_sort(l, idx-1, nodes, ids);

  l = idx;
  for (int i=idx; i<=r; i++) {
     if (quat(nodes[i]) == 2) {
      if (i != idx) {
        swap(nodes[i], nodes[idx]);
        swap(ids[i], ids[idx]);
      }
      idx++;
    } 
  }
  if (l < idx-1)
    fractal_sort(l, idx-1, nodes, ids);

  l = idx;
  for (int i=idx; i<=r; i++) {
     if (quat(nodes[i]) == 3) {
      if (i != idx) {
        swap(nodes[i], nodes[idx]);
        swap(ids[i], ids[idx]);
      }
      idx++;
    }
  }
  if (l < idx-1)
    fractal_sort(l, idx-1, nodes, ids);
}

NodeOrdering compute_fractal_order(const std::vector<xyLoc> nodes) {
  std::vector<xyLoc> cur_nodes;
  std::vector<int> cur_ids;
  int16_t xmin = nodes[0].x, ymin = nodes[0].y;
  for (int i=0; i<(int)nodes.size(); i++) {
    cur_nodes.push_back(nodes[i]);
    cur_ids.push_back(i);
    xmin = min(xmin, nodes[i].x);
    ymin = min(ymin , nodes[i].y);
  }
  for (int i=0; i<(int)nodes.size(); i++) {
    cur_nodes[i].x -= xmin;
    cur_nodes[i].y -= ymin;
  }
  NodeOrdering order(nodes.size());
  fractal_sort(0, nodes.size()-1, cur_nodes, cur_ids);
  for (int i=0; i<(int)nodes.size(); i++) {
    order.map(cur_ids[i], i);
  }
  return order;
}
