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

double InvNodeToCentroid(const Index& data, xyLoc cent, xyLoc t, int hLevel, Counter& c, Extracter& e, int limit) {
  // cent is a centroid location
  int ranks = data.mapper.get_centroid_rank(data.mapper(cent));
  assert(ranks != -1);
  int (*heuristic_func)(int, int, const Mapper&);
  if (hLevel == 1)
    heuristic_func = Hsymbol::get_heuristic_move1;
  else if (hLevel == 2)
    heuristic_func = Hsymbol::get_heuristic_move2;
  else if (hLevel == 3)
    heuristic_func = Hsymbol::get_heuristic_move3;

  int curs = data.mapper(cent);
  int curt = data.mapper(t);
  int lhs = -1, rhs = -1, cur_move = -1, move = warthog::NOMOVE;
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

  e.last = curt;
  while (data.mapper.get_fa()[curt] != curs) {
    to_next_pos(cent, t, curs, curt);
    e.last = curt;
    if ((1<<move)== warthog::NOMOVE) break;
    c.steps++;
    if (limit != -1 && limit <= c.steps) break;
  }
  return cost;
}

double InvNodeToNodeInSameCentroid(const Index& data, Extracter& ea, Extracter& eb, 
    Counter& c, int hLevel=3) {
  xyLoc a = data.mapper(ea.last);
  xyLoc b = data.mapper(eb.last);
  int cura = ea.last, cura2 = ea.last;
  int curb = eb.last, curb2 = eb.last;
  int cid = data.mapper.get_fa()[cura];
  xyLoc cent = data.mapper(cid);
  assert(data.mapper.get_fa()[curb] == cid);
  int ranks = data.mapper.get_centroid_rank(cid);
  assert(ranks != -1);

  int (*heuristic_func)(int, int, const Mapper&);
  if (hLevel == 1)
    heuristic_func = Hsymbol::get_heuristic_move1;
  else if (hLevel == 2)
    heuristic_func = Hsymbol::get_heuristic_move2;
  else if (hLevel == 3)
    heuristic_func = Hsymbol::get_heuristic_move3;

  int move = warthog::NOMOVE;
  const int16_t* dx = warthog::dx;
  const int16_t* dy = warthog::dy;
  double cost = 0.0;
  double cost2 = 0.0;

  auto to_next_pos = [&](xyLoc& target, int& tid, Extracter& e) {
    if (tid == cid) return;
    move = data.cpd.get_first_move(ranks, tid);
    if ((1<<move) == warthog::NOMOVE) return;

    int pruned = data.mapper.get_pruned_neighbor(tid);
    if ((1<<move) == warthog::HMASK) {
      move = Hsymbol::decode(tid, cid, data.mapper, heuristic_func);
    } else if (!(pruned & (1 << move))) { // pseudo obstacle move 
      // if not move to target, then decode
      bool reached = (target.x + dx[move] == cent.x && target.y + dy[move] == cent.y);
      if (!reached || !((1<<move) & data.mapper.get_neighbor(tid)))
        move = Hsymbol::get_closest_valid_move(tid, move, data.mapper);
    }
    cost += warthog::doublew[move];
    target.x += dx[move];
    target.y += dy[move];
    tid = data.mapper(target);
    e.last = tid;
    e.add(move);
  };

  xyLoc ta, tb, ta2, tb2;
  ta = ta2 = a, tb = tb2 = b;
  cura = cura2 = data.mapper(ta), curb = curb2 = data.mapper(tb);
  while (cura != curb) {
    to_next_pos(ta, cura, ea);
    if ((1<<move) == warthog::NOMOVE && cid != cura) break;
    to_next_pos(tb, curb, eb);
    if ((1<<move) == warthog::NOMOVE && cid != curb) break;
    c.steps++;
  }

  while (cura2 != curb2) {
    move = Hsymbol::decode(cura2, curb2, data.mapper, 3);
    if ((1<<move)== warthog::NOMOVE) {
      cost2 = 1e10;
      break;
    }
    ta2.x += dx[move];
    ta2.y += dy[move];
    cost2 += warthog::doublew[move];
    cura2 = data.mapper(ta2);
    if (ea.isVis(cura2) || cost2 >= cost) {
      cost2 = 1e10;
      break;
    }
    ea.mark(cura2);
  }
  return min(cost, cost2);
}


double GetInvCentroidCost(const Index& data, xyLoc s, xyLoc g, int hLevel, Counter& c, 
    Extracter& eg, Extracter& es, int limit) {
  int gid = data.mapper(g);
  int cgid = data.mapper.get_fa()[gid];
  xyLoc cg = data.mapper(cgid);
  eg.last = gid;

  int sid = data.mapper(s);
  int csid = data.mapper.get_fa()[sid];
  xyLoc cs = data.mapper(csid);
  es.last = sid;

  double cost0 = 0, cost1 = 0;
  if (abs(s.x - cg.x) + abs(s.y - cg.y) < abs(g.x - cs.x) + abs(g.y - cs.y))
    cost0 = InvNodeToCentroid(data, cg, s, hLevel, c, es, limit);
  else 
    cost0 = InvNodeToCentroid(data, cs, g, hLevel, c, eg, limit);
  assert(cost0 <= warthog::EPS || data.mapper.get_fa()[es.last] == data.mapper.get_fa()[eg.last]);
  if (data.mapper.get_fa()[es.last] == data.mapper.get_fa()[eg.last]) // if there is a path
    cost1 = InvNodeToNodeInSameCentroid(data, es, eg, c, hLevel);
  return cost0 + cost1;
}
