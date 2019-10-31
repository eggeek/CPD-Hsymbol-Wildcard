#include "Hsymbol.h"
#include "query.h"

double GetPathCostSRC(const Index& state, xyLoc s, xyLoc t, int hLevel, Counter& c, Extracter& e, int limit) {
  int (*heuristic_func)(int, int, const Mapper&);
  if (hLevel == 1)
    heuristic_func = Hsymbol::get_heuristic_move1;
  else if (hLevel == 2)
    heuristic_func = Hsymbol::get_heuristic_move2;
  else if (hLevel == 3)
    heuristic_func = Hsymbol::get_heuristic_move3;

  int current_source = state.mapper(s);
  int current_target = state.mapper(t);
  const int16_t* dx = warthog::dx;
  const int16_t* dy = warthog::dy;
  double cost = 0.0;

  auto is_in_square = [&](int current_source, int current_target)
  {
      int side = state.square_sides[current_source];
      xyLoc loc_source = state.mapper.operator()(current_source);
      xyLoc loc_x = state.mapper.operator()(current_target);
      int dx = iabs(loc_source.x - loc_x.x);
      int dy = iabs(loc_source.y- loc_x.y);
      if(( (dx<<1) <= (side-1)) && ( (dy<<1) <= (side-1)))
      {
        return true;
      }
      return false;
  };

  //cerr << "(" << s.x << "," << s.y << ")" << endl;
  while (current_source != current_target) {
    int move;
    if(is_in_square(current_source, current_target))
    {
      move = Hsymbol::decode(current_source, current_target, state.mapper, heuristic_func);
    }
    else
    {
      move = state.cpd.get_first_move(current_source, current_target);
      c.access_cnt++;
    }
    // no path exist
    if (move == 0xF) break;
    if ((1<<move) == warthog::CENTMASK) {
      int cent = state.mapper.get_fa()[current_source];
      if (is_in_square(cent, current_target))
        move = warthog::m2i.at(warthog::HMASK);
      else 
        move = state.cpd.get_first_move(cent, current_target);
      c.access_cnt++;
    }
    if ((1 << move) == warthog::HMASK) {
      move = Hsymbol::decode(current_source, current_target, state.mapper, heuristic_func);
    }
    cost += warthog::doublew[move];
    s.x += dx[move];
    s.y += dy[move];
    e.add(move);
    c.steps++;
    if (limit != -1 && limit <= c.steps)
      break;
    current_source = state.mapper(s);
  }
  return cost;
}

double GetForwardCentroidCost(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c,
    Extracter& e1, Extracter& e2, int limit) {
  int gid = data.mapper(g);
  int cid = data.mapper.get_fa()[gid];
  xyLoc centroid = data.mapper(cid);
  double cost0 = GetPathCostSRC(data, s, centroid, hLevel, c, e1, limit);
  double cost1 = GetPathCostSRC(data, g, centroid, hLevel, c, e2, limit);
  double tot = cost0 + cost1;

  int l1 = e1.steps-1, l2 = e2.steps-1;
  while (l1 >= 0 && l2 >= 0 && e1.moves[l1] == e2.moves[l2]) {
    tot -= warthog::doublew[e1.moves[l1]];
    c.steps--, l1--, l2--;
  }

  return tot;
}

double GetRectWildCardCost(const Index& data, xyLoc s, xyLoc t, int hLevel, Counter& c, Extracter& e, int limit) {
  int (*heuristic_func)(int, int, const Mapper&);
  if (hLevel == 1)
    heuristic_func = Hsymbol::get_heuristic_move1;
  else if (hLevel == 2)
    heuristic_func = Hsymbol::get_heuristic_move2;
  else if (hLevel == 3)
    heuristic_func = Hsymbol::get_heuristic_move3;

  int curs = data.mapper(s);
  int curt = data.mapper(t);
  int move;
  const int16_t* dx = warthog::dx;
  const int16_t* dy = warthog::dy;
  double cost = 0.0;

  auto is_in_square = [&](int current_source, int current_target)
  {
      int side = data.square_sides[current_source];
      xyLoc loc_source = data.mapper.operator()(current_source);
      xyLoc loc_x = data.mapper.operator()(current_target);
      int dx = iabs(loc_source.x - loc_x.x);
      int dy = iabs(loc_source.y- loc_x.y);
      if(( (dx<<1) <= (side-1)) && ( (dy<<1) <= (side-1)))
      {
        return true;
      }
      return false;
  };

  auto to_next_pos = [&](xyLoc& source, xyLoc& target, int& sid, int& tid) {

    if (is_in_square(sid, tid)) {
      move = Hsymbol::decode(sid, tid, data.mapper, heuristic_func);
    }
    else {
      const RectInfo* rect = data.rwobj.get_rects(sid, data.mapper, target);
      if (rect == NULL) {
        move = data.cpd.get_first_move(sid, tid);
        c.access_cnt++;
      }
      else {
        move = rect->mask? warthog::m2i.at(warthog::lowb(rect->mask)): 0xF;
      }
      if ((1<<move)== warthog::NOMOVE) return;
      if ((1<<move) == warthog::HMASK) {
        move = Hsymbol::decode(sid, tid, data.mapper, heuristic_func);
      }
    }
    cost += warthog::doublew[move];
    source.x += dx[move];
    source.y += dy[move];
    e.add(move);
    sid = data.mapper(source);
  };

  while (curs != curt) {
    if (data.row_ordering[curs] >= data.row_ordering[curt]) {
      to_next_pos(s, t, curs, curt);
      if ((1<<move)== warthog::NOMOVE) break;
    }
    else {
      to_next_pos(t, s, curt, curs); 
      if ((1<<move)== warthog::NOMOVE) break;
    }
    c.steps++;
    if (limit != -1 && c.steps>= limit) break;
  }
  return cost;
}

double GetInvCPDCost(const Index& data, xyLoc s, xyLoc t, int hLevel, Counter& c, Extracter& e, int limit) {
  int (*heuristic_func)(int, int, const Mapper&);
  if (hLevel == 1)
    heuristic_func = Hsymbol::get_heuristic_move1;
  else if (hLevel == 2)
    heuristic_func = Hsymbol::get_heuristic_move2;
  else if (hLevel == 3)
    heuristic_func = Hsymbol::get_heuristic_move3;

  int curs = data.mapper(s);
  int curt = data.mapper(t);
  int lhs = -1, rhs = -1, cur_move = -1, move;
  const int16_t* dx = warthog::dx;
  const int16_t* dy = warthog::dy;
  double cost = 0.0;
  vector<int>::const_iterator it = data.cpd.get_entry().end();

  auto to_next_pos = [&](xyLoc& source, xyLoc& target, int& sid, int& tid) {
    
    if (!(tid >= lhs && tid <= rhs)) {
      it = data.cpd.get_interval(sid, tid, lhs, rhs, cur_move, it, data.mapper);
      c.access_cnt++;
    }
    move = cur_move;
    if ((1<<move) == warthog::NOMOVE) return;

    int pruned = data.mapper.get_pruned_neighbor(tid);
    if ((1<<move) == warthog::HMASK) {
      move = Hsymbol::decode(tid, sid, data.mapper, heuristic_func);
    } else if (!(pruned & (1 << move))) { // pseudo obstacle move 
      // if not move to target, then decode
      bool reached = (target.x + dx[move] == source.x && target.y + dy[move] == source.y);
      if (!reached || !((1<<move) & data.mapper.get_neighbor(tid)))
        move = Hsymbol::get_closest_valid_move(tid, move, data.mapper);
    }
   
    cost += warthog::doublew[move];
    target.x += dx[move];
    target.y += dy[move];
    e.add(move);
    tid = data.mapper(target);
  };

  while (curs != curt) {
    to_next_pos(s, t, curs, curt);
    if ((1<<move)== warthog::NOMOVE) break;
    c.steps++;
    if (limit != -1 && limit <= c.steps) break;
  }
  return cost;
}

double GetCentroid2TargetCost(const Index& data, xyLoc s, xyLoc t, int hLevel, Counter& c, Extracter& e, int limit) {
  // s is a centroid location
  int ranks = data.mapper.get_centroid_rank(data.mapper(s));
  assert(ranks != -1);
  int (*heuristic_func)(int, int, const Mapper&);
  if (hLevel == 1)
    heuristic_func = Hsymbol::get_heuristic_move1;
  else if (hLevel == 2)
    heuristic_func = Hsymbol::get_heuristic_move2;
  else if (hLevel == 3)
    heuristic_func = Hsymbol::get_heuristic_move3;

  int curs = data.mapper(s);
  int curt = data.mapper(t);
  int lhs = -1, rhs = -1, cur_move = -1, move;
  const int16_t* dx = warthog::dx;
  const int16_t* dy = warthog::dy;
  double cost = 0.0;
  auto it = data.cpd.get_entry().end();
  auto to_next_pos = [&](xyLoc& source, xyLoc& target, int& sid, int& tid) {
    if (!(tid >= lhs && tid <= rhs)) {
      it = data.cpd.get_interval(ranks, tid, lhs, rhs, cur_move, it, data.mapper);
      c.access_cnt++;
    }
    move = cur_move;
    if ((1<<move) == warthog::NOMOVE) return;

    int pruned = data.mapper.get_pruned_neighbor(tid);
    if ((1<<move) == warthog::HMASK) {
      move = Hsymbol::decode(tid, sid, data.mapper, heuristic_func);
    } else if (!(pruned & (1 << move))) { // pseudo obstacle move 
      // if not move to target, then decode
      bool reached = (target.x + dx[move] == source.x && target.y + dy[move] == source.y);
      if (!reached || !((1<<move) & data.mapper.get_neighbor(tid)))
        move = Hsymbol::get_closest_valid_move(tid, move, data.mapper);
    }
    cost += warthog::doublew[move];
    target.x += dx[move];
    target.y += dy[move];
    e.add(move);
    tid = data.mapper(target);
  };

  while (curs != curt) {
    to_next_pos(s, t, curs, curt);
    if ((1<<move)== warthog::NOMOVE) break;
    c.steps++;
    if (limit != -1 && limit <= c.steps) break;
  }
  return cost;
}


double GetInvCentroidCost(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c, 
    Extracter& e1, Extracter& e2, int limit) {
  int gid = data.mapper(g);
  int cid = data.mapper.get_fa()[gid];
  double cost0 = GetCentroid2TargetCost(data, data.mapper(cid), s, hLevel, c, e1, limit);
  double cost1 = GetCentroid2TargetCost(data, data.mapper(cid), g, hLevel, c, e2, limit);
  double tot = cost0 + cost1;

  int l1 = e1.steps-1, l2 = e2.steps-1;
  while (l1 >= 0 && l2 >= 0 && e1.moves[l1] == e2.moves[l2]) {
    tot -= warthog::doublew[e1.moves[l1]];
    c.steps--, l1--, l2--;
  }
  return tot;
}
