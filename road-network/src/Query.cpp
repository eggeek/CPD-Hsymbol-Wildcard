#include "Query.h"
#include "Hsymbol.h"
using namespace std;
namespace H = Hsymbol;
long long GetPathCostSRC(void *data, int s, int t, int hLevel, int limit) {
  State* state = static_cast<State*>(data);
  int current_source = state->mapper.getPos(s);
  int current_target = state->mapper.getPos(t);
  long long cost = 0;
  int steps = 0;
  while (current_source != current_target) {
    int move = state->cpd.get_first_move(current_source, current_target);
    // no path exist
    if (move == 0xF) {
      cost = -1;
      break;
    }
    if ((1 << move) == warthog::HMASK) {
      move = H::decode(current_source, current_target, state->mapper, hLevel);
    }
    const OutArc& a= state->mapper.g.out(current_source, move);
    cost += a.weight;
    steps ++;
    if (limit != -1 && limit <= steps)
      break;
    current_source = a.target;
  }
  return cost;
}

long long GetPath(void *data, int s, int t, vector<int> &path, int hLevel, int limit) {
  State* state = static_cast<State*>(data);
  int current_source = state->mapper.getPos(s);
  int current_target = state->mapper.getPos(t);
  long long cost = 0;
  int steps = 0;
  while (current_source != current_target) {
    int move = state->cpd.get_first_move(current_source, current_target);
    // no path exist
    if (move == 0xF) {
      cost = -1;
      break;
    }
    path.push_back(current_source);
    if ((1 << move) == warthog::HMASK) {
      move = H::decode(current_source, current_target, state->mapper, hLevel);
    }
    const OutArc& a= state->mapper.g.out(current_source, move);
    cost += a.weight;
    steps ++;
    if (limit != -1 && limit <= steps)
      break;
    current_source = a.target;
  }
  path.push_back(current_target);
  return cost;
}

long long GetPathWithSectorWildCard(void* data, int s, int t, vector<int>& path, int hLevel, int limit) {
  State* state = static_cast<State*>(data);
  int current_source = state->mapper.getPos(s);
  int current_target = state->mapper.getPos(t);
  long long cost = 0;
  int steps = 0;
  while (current_source != current_target) {
    int move;
    // membership detection: is in sectors
    int d = Sectors::find_sector(
        state->sectors[current_source],
        state->mapper(current_source),
        state->mapper(current_target));
    if (d != -1) // use wildcard
      move = d;
    else {// use cpd
      move = state->cpd.get_first_move(current_source, current_target);
      // no path exist
      if (move == 0xF) {
        cost = -1;
        break;
      }
      if ((1 << move) == warthog::HMASK) {
        move = H::decode(current_source, current_target, state->mapper, hLevel);
      }
    }
    path.push_back(current_source);
    const OutArc& a= state->mapper.g.out(current_source, move);
    cost += a.weight;
    steps ++;
    if (limit != -1 && limit <= steps)
      break;
    current_source = a.target;
  }
  path.push_back(current_target);
  return cost;
}


long long GetPathCostSRCWithSectorWildCard(void* data, int s, int t, int hLevel, int limit) {
  State* state = static_cast<State*>(data);
  int current_source = state->mapper.getPos(s);
  int current_target = state->mapper.getPos(t);
  long long cost = 0;
  int steps = 0;
  while (current_source != current_target) {
    int move;
    // membership detection: is in sectors
    int d = Sectors::find_sector(
        state->sectors[current_source],
        state->mapper(current_source),
        state->mapper(current_target));
    if (d != -1) // use wildcard
      move = d;
    else {
       move = state->cpd.get_first_move(current_source, current_target);
      // no path exist
      if (move == 0xF) {
        cost = -1;
        break;
      }
      if ((1 << move) == warthog::HMASK) {
        move = H::decode(current_source, current_target, state->mapper, hLevel);
      }
    }
    const OutArc& a= state->mapper.g.out(current_source, move);
    cost += a.weight;
    steps ++;
    if (limit != -1 && limit <= steps)
      break;
    current_source = a.target;
  }
  return cost;
}
