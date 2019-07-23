#pragma once
#include <vector>
#include <algorithm>
#include <string>
#include "adj_graph.h"
#include "binary_search.h"
#include "range.h"
#include "vec_io.h"
#include "mapper.h"
#include "rect_wildcard.h"
using namespace std;
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
  void append_row(int source_node, const std::vector<unsigned short>&first_move,
                  Mapper mapper, const int side);

  void append_rows(const CPD&other);

  vector<RectInfo> append_row(int s, const vector<unsigned short>& allowed, const Mapper& mapper, 
      const vector<RectInfo>& rects, const vector<int>& row_ordering,
      const int side);
  vector<int> compress(int s, const vector<unsigned short>& allowed,
      const RectInfo& rect, const Mapper& mapper, const vector<int>& row_ordering,
      const int side);
  //! Get the first move. 
  //! An ID of 0xF means that there is no path. 
  //! If source_node == target_node then return value is undefined. 
  unsigned char get_first_move(int source_node, int target_node)const{
    assert(source_node != -1);
    assert(target_node != -1);
    target_node <<= 4;
    target_node |= 0xF;
    return *binary_find_last_true(
      entry.begin() + begin[source_node],
      entry.begin() + begin[source_node+1],
      [=](int x){return x <= target_node;}
    )&0xF;
  }

  vector<int>::const_iterator get_first_iter(int lhs, int rhs, int t) const {
    t <<= 4;
    t |= 0xF;
    return binary_find_last_true(
        entry.begin() + lhs,
        entry.begin() + rhs,
        [=](int x){return x <= t;}
    );
  }

  vector<int>::const_iterator get_interval(int s, int t, int& lhs, int& rhs, int& move,
      vector<int>::const_iterator pre, const Mapper& mapper) const {
    vector<int>::const_iterator it;
    if (pre == entry.end()) {
      it = get_first_iter(begin[s], begin[s+1], t);
    }
    else if (t > rhs) {
      int lb = pre - entry.begin();
      it = get_first_iter(lb+1, begin[s+1], t);
    }
    else if (t < lhs) {
      int ub = pre - entry.begin();
      it = get_first_iter(begin[s], ub, t);
    }
    else {
      return pre;
    }

    lhs = (*it) >> 4;
    if (std::next(it) == entry.end() || std::next(it) == entry.begin() + begin[s+1])
      rhs = mapper.node_count();
    else
      rhs = ((*std::next(it))>>4)-1;
    move = (*it)&0xF;
    return it;
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

  int get_entry_size() {
    return entry.size();
  }
  const vector<int>& get_entry() const {
    return entry;
  }

  const vector<int>& get_begin() const {
    return begin;
  }

  int get_heuristic_cnt(int row) const {
    int hcnt = 0;
    for (int i=begin[row]; i<begin[row+1]; i++) {
      if ((1<<(entry[i]&0xF) == warthog::HMASK)) {
        int node_begin = entry[i]>>4;
        int node_end = i+1==begin[row+1]?node_count(): entry[i+1]>>4;
        hcnt += node_end - node_begin;
      }
    }
    return hcnt;
  }

  int get_heuristic_run(int row) const {
    int hrun= 0;
    for (int i=begin[row]; i<begin[row+1]; i++) {
      if ((1<<(entry[i]&0xF) == warthog::HMASK)) hrun++;
    }
    return hrun;
  }

private:
  std::vector<int>begin;
  std::vector<int>entry;
};

inline
bool operator!=(const CPD&l, const CPD&r){
  return !(l == r);
}
