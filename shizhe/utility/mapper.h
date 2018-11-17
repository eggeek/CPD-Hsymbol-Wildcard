#ifndef MAPPER_H
#define MAPPER_H
#include <vector>
#include <cstdint>
#include "list_graph.h"
#include "Entry.h"
#include "order.h"
#include "constants.h"
using namespace std;

struct ClosestMove {
  int move[4][4];
};

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
    //init_jps_tiles();
    init_neighbors();
    initClosestMove();
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
    std::vector<int> new_tiles(node_count_);
    for(int new_node=0; new_node<node_count(); ++new_node){
      int old_node = order.to_old(new_node);
      new_node_to_pos_[new_node] = node_to_pos_[old_node];
      //new_tiles[new_node] = jps_tiles[old_node];
    }
    new_node_to_pos_.swap(node_to_pos_);
    new_tiles.swap(jps_tiles);
  }

  static inline uint32_t str2tiles(const vector<string>& map) {
    uint32_t tiles = 0;
    for (int i=0; i<3; i++) {
      for (int j=0; j<3; j++) if (map[i][j] != '#') {
        tiles |= (1 << j) << (i * 8);
      }
    }
    return tiles;
  }

  static inline vector<string> tiles2str(uint32_t tiles) {
    vector<string> map = {
      "...",
      ".x.",
      "..."
    };
    for (int i=0; i<3; i++, tiles >>= 8) {
      for (int j=0; j<3; j++) {
        if (tiles & (1 << j)) map[i][j] = '.';
        else map[i][j] = '#';
      }
    }
    map[1][1] = 'x';
    return map;
  }

  static inline void set2direct(uint32_t moveset) {
    if (moveset & warthog::jps::NORTH) cout << "north ";
    if (moveset & warthog::jps::SOUTH) cout << "south ";
    if (moveset & warthog::jps::EAST) cout << "east ";
    if (moveset & warthog::jps::WEST) cout << "west ";
    if (moveset & warthog::jps::NORTHEAST) cout << "northeast ";
    if (moveset & warthog::jps::NORTHWEST) cout << "northwest ";
    if (moveset & warthog::jps::SOUTHEAST) cout << "southeast ";
    if (moveset & warthog::jps::SOUTHWEST) cout << "southwest ";
    cout << endl;
  }

  static inline int str2neighbors(vector<string> data) {
    int mask = 0;
    for (int i=0; i<8; i++) {
      int dx = warthog::dx[i];
      int dy = warthog::dy[i];
      if (data[1 + dx][1 + dy] == '.') mask &= 1<<i;
    }
    return mask;
  }

  static inline vector<string> neighbors2str(int mask) {
    vector<string> data = {
      "###",
      "#.#",
      "###",
    };
    for (int i=0; i<8; i++) if (mask & (1<<i)) {
      int dx = warthog::dx[i];
      int dy = warthog::dy[i];
      data[1 + dx][1 + dy] = '.';
    }
    return data;
  }

  int get_jps_tiles(int x) const {
    return this->jps_tiles[x];
  }

  int get_neighbor(int x) const {
    return this->neighbors[x];
  }

  inline int get_valid_move(int s, int quad, int part) const {
    //return getClosestMove(this->neighbors[s], quad, part);
    return this->mem[this->neighbors[s]].move[quad][part];
  }

private:
  int width_, height_, node_count_;
  std::vector<int>pos_to_node_;
  std::vector<xyLoc>node_to_pos_;
  vector<int> jps_tiles;
  vector<int> neighbors;
  vector<ClosestMove> mem;

  void init_neighbors() {
    neighbors.resize(node_count_);
    for (int i=0; i<node_count_; i++) {
      xyLoc cur = this->operator()(i);
      int mask = 0;
      for (int d=0; d<8; d++) {
        xyLoc nxt, p1, p2;
        nxt.x = cur.x + warthog::dx[d];
        nxt.y = cur.y + warthog::dy[d];

        p1.x = cur.x, p1.y = cur.y + warthog::dy[d];
        p2.x = cur.x + warthog ::dx[d], p2.y = cur.y;
        if (this->operator()(nxt) != -1 &&
            this->operator()(p1) != -1 &&
            this->operator()(p2) != -1 ) mask |= 1<<d;
      }
      neighbors[i] = mask;
    }
  }

  void init_jps_tiles() {
    jps_tiles.resize(node_count_);
    for (int i=0; i<node_count_; i++) {
      vector<string> data = {
        "...",
        ".x.",
        "..."
      };
      xyLoc cur = this->operator()(i);
      for(int j=0; j<8; j++) {
        xyLoc neighbor = cur;
        neighbor.x += warthog::dx[j];
        neighbor.y += warthog::dy[j];
        if (this->operator()(neighbor) != -1) {
          data[1 + warthog::dy[j]][1 + warthog::dx[j]] = '.';
        } else 
          data[1 + warthog::dy[j]][1 + warthog::dx[j]] = '#';
      } 
      jps_tiles[i] = str2tiles(data);
    }
  }

  int getClosestMove(int mask, int quad, int part) const {
    int dx, dy;
    switch (part) {
      case 0:dx=0, dy=100;
                break; 
      case 1:dx=90, dy=100;
                break;
      case 2:dx=110, dy=100;
                break;
      case 3:dx=100, dy=0;
    }
    if (quad == 3 || quad == 2) dx *= -1;
    if (quad == 2 || quad == 1) dy *= -1;
    auto octileDist = [&](xyLoc a, xyLoc b) {
      int common = min(abs(a.x - b.x), abs(a.y - b.y));
      double res = warthog::DBL_ROOT_TWO * common + abs(a.x - b.x) + abs(a.y - b.y) - 2.0 * common;
      return res;
    };

    //double cos = -1;
    double cost = warthog::INF;
    int move = warthog::INVALID_MOVE;
    for (int i=7; i>=0; i--) if (mask & (1<<i)) {
      xyLoc nxt;
      nxt.x = warthog::dx[i];
      nxt.y = warthog::dy[i];
      //nxt.x *= 100, nxt.y *= 100;
      //double t = edist(nxt, xyLoc{(int16_t)dx, (int16_t)dy}) + warthog::doublew[i];
      double t = octileDist(nxt, xyLoc{(int16_t)dx, (int16_t)dy}) + warthog::doublew[i];
      if (t < cost) {
        cost = t;
        move = i;
      }
      /*
      double crossp = nxt.x * dx + nxt.y * dy;
      double nxt_mag = sqrt((double)(nxt.x * nxt.x) + (double)(nxt.y * nxt.y));
      double d_mg = sqrt((double)(dx * dx) + (double)(dy * dy));
      double cosi = crossp / (nxt_mag * d_mg);
      if (cosi > cos) {
        cos = cosi;
        move = i;
      }
      */
    }
    return move;
  }

  void initClosestMove() {
    mem.resize(1<<8);
    for (int i=0; i<(1<<8); i++) {
      for (int quad=0; quad<4; quad++) {
        for (int part=0; part<4; part++) {
          mem[i].move[quad][part] = getClosestMove(i, quad, part);
        }
      }
    } 
  }
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

