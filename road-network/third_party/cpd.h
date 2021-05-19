#ifndef CPD_H
#define CPD_H

#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include "adj_graph.h"
#include "binary_search.h"
#include "range.h"
#include "vec_io.h"
#include "constants.h"
#include "sector_wildcard.h"
#include "mapper.h"

//! Compressed Path database. Allows to quickly query the first out arc id of
//! any shortest source-target-path. There may be at most 15 outgoing arcs for
//! any node.
class CPD{
public:
  CPD():begin{0}{}

  //! Adds a new node s to the CPD. first_move should be an array that 
  //! maps every target node onto a 15-bit bitfield that has a bit set
  //! for every valid first move. get_first_move is free to return any of
  //! them.
  void append_row(int source_node, const std::vector<unsigned short>&first_move);
  void append_row(
    int source_node,
    const std::vector<unsigned short>& first_move,
    const Sectors& sectors,
    const Mapper& mapper);

  void append_rows(const CPD&other);

  //! Get the first move. 
  //! An ID of 0xF means that there is no path. 
  //! If source_node == target_node then return value is undefined. 
  unsigned char get_first_move(int source_node, int target_node)const{
    target_node <<= 4;
    target_node |= 0xF;
    return *binary_find_last_true(
      entry.begin() + begin[source_node],
      entry.begin() + begin[source_node+1],
      [=](int x){return x <= target_node;}
    )&0xF;
  }

  int node_count()const{
    return begin.size()-1;
  }

  int entry_count()const{
    return entry.size();
  }

  friend bool operator==(const CPD&l, const CPD&r){
    return l.begin == r.begin && l.entry == r.entry;
  }

  void save(std::FILE*f)const{
    save_vector(f, begin);
    save_vector(f, entry);
  }

  void load(std::FILE*f){
    begin = load_vector<int>(f);
    entry = load_vector<int>(f);
  }

  int get_begin_size() {
    return begin.size();
  }

  int get_row_length(int i) {
    return begin[i+1] - begin[i];
  }

  int get_entry_size() {
    return entry.size();
  }

  // how many h symbol before compression
  int get_h_num(int r) {
    int len = get_row_length(r);
    int res = 0;
    for (int i=0; i<len; i++) {
      int mask = entry[begin[r] + i];
      int move = mask & 0xF;
      int vid = mask >> 4;
      if ((1 << move) == warthog::HMASK) {
        if (i+1 == len) {
          res += node_count() - vid + 1;
          if (r >= vid) res--;
        } else {
          int nxt_id = (entry[begin[r] + i+1] >> 4);
          res += nxt_id - vid;
          if (r >= vid && r <= nxt_id)
            res--;
        }
      }
    } 
    return res;
  }

  vector<int> get_ids(int r, bool head_only=false) {
    vector<int> res;
    int len = get_row_length(r);
    for (int i=0; i<len; i++) {
      int mask = entry[begin[r] + i];
      int id = mask >> 4;
      res.push_back(id);
      if (head_only) continue;
      if (i + 1 < len) {
        int id2 = (entry[begin[r] + i + 1] >> 4) - 1;
        if (id != id2) res.push_back(id2);
      }
    }
    return res;
  }

  int get_run_size(int r, int i, int n) {
    int len = get_row_length(r);
    int id = entry[begin[r] + i] >> 4;
    if (i + 1 < len) {
      int id2 = (entry[begin[r] + i + 1] >> 4) -1;
      return id2 - id + 1;
    }
    else return n - id + 1;
  }

  map<int, int> get_statistic(int r) {
    map<int, int> res;
    int len = get_row_length(r);
    for (int i=0; i<len; i++) {
      int mask = entry[begin[r] + i];
      int id = mask >> 4;
      res[id]++;
    }
    return res;
  }

  // how many entries store h symbol
  int get_h_entry(int r) {
    int len = get_row_length(r);
    int res = 0;
    for (int i=0; i<len; i++) {
      int mask = entry[begin[r] + i];
      int move = mask & 0xF;
      if ((1 << move) == warthog::HMASK)
        res ++;
    }
    return res;
  }

  // how many entries contains the given mask
  int get_mask_entry(int r, int mask) {
    int len = get_row_length(r);
    int res = 0;
    for (int i=0; i<len; i++) {
      int cur = entry[begin[r] + i];
      int move = cur & 0xF;
      if ((1 << move) & mask) res++;
    }
    return res;
  }

  vector<int> compress(int s, const vector<unsigned short>& allowed);
  vector<int> compress(int s, const vector<unsigned short>& allowed,
    const Sectors& sectors, const Mapper& mapper);

private:
  std::vector<int>begin;
  std::vector<int>entry;
};

inline
bool operator!=(const CPD&l, const CPD&r){
  return !(l == r);
}

#endif
