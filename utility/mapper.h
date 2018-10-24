#ifndef MAPPER_H
#define MAPPER_H
#include <vector>
#include <cstdint>
#include "list_graph.h"
#include "Entry.h"
#include "order.h"
#include "constants.h"

class Mapper{
public:
  Mapper(){}
  Mapper(const std::vector<bool>&field, int width, int height):
    width_(width), 
    height_(height),
    node_count_(0),
    pos_to_node_(width*height, -1)
  {
    for(int y=0; y<height_; ++y)
      for(int x=0; x<width_; ++x)
        if(field[x+y*width_]){
          node_to_pos_.push_back({static_cast<std::int16_t>(x), static_cast<std::int16_t>(y)});
          pos_to_node_[x+y*width_] = node_count_++;
        }else{
          pos_to_node_[x+y*width_] = -1;
        }
  }

  int width()const{
    return width_;
  }

  int height()const{
    return height_;
  }

  int node_count()const{
    return node_count_;
  } 

  xyLoc operator()(int x)const{ return node_to_pos_[x]; }
  int operator()(xyLoc p)const{ 
    if(p.x < 0 || p.x >= width_ || p.y < 0 || p.y >= height_)
      return -1;
    else
      return pos_to_node_[p.x + p.y*width_];
  }

  void reorder(const NodeOrdering&order){
    for(auto&x:pos_to_node_){
      if(x != -1){
        x = order.to_new(x);
      }
    }
    std::vector<xyLoc>new_node_to_pos_(node_count_);
    for(int new_node=0; new_node<node_count(); ++new_node){
      int old_node = order.to_old(new_node);
      new_node_to_pos_[new_node] = node_to_pos_[old_node];
    }
    new_node_to_pos_.swap(node_to_pos_);
  }
private:
  int width_, height_, node_count_;
  std::vector<int>pos_to_node_;
  std::vector<xyLoc>node_to_pos_;
};

inline
ListGraph extract_graph(const Mapper&mapper){
  static const int16_t* dx = warthog::dx;
  static const int16_t* dy = warthog::dy;
  static const int* dw = warthog::dw;

  ListGraph g(mapper.node_count());
  for(int u=0; u<mapper.node_count(); ++u){
    auto u_pos = mapper(u);
    for(int d = 0; d<8; ++d){
      xyLoc v_pos = {static_cast<std::int16_t>(u_pos.x + dx[d]), static_cast<std::int16_t>(u_pos.y + dy[d])};
      int v = mapper(v_pos);
      xyLoc p1 = xyLoc{u_pos.x, static_cast<std::int16_t>(u_pos.y+dy[d])};
      xyLoc p2 = xyLoc{static_cast<std::int16_t>(u_pos.x+dx[d]), u_pos.y};
      if(v != -1 && mapper(p1) != -1 &&  mapper(p2) != -1) { // obstacle cut
        g.arc.push_back({u, v, dw[d], d});
      }
    }
  }
  return g;
}

void dump_map(const Mapper&map, const char*file);

#endif

