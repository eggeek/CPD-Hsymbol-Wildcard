#include "Hsymbol.h"
#include "query.h"

double GetPathCostSRC(const Index& state, xyLoc s, xyLoc t, int hLevel, Counter& c, int limit) {
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

  auto next_move = [&](int current_source, int current_target)
  {
      xyLoc cs = state.mapper.operator()(current_source);
      xyLoc ct = state.mapper.operator()(current_target);
      int vx = signbit(ct.x - cs.x);
      int vy = signbit(ct.y - cs.y);
      return warthog::v2i[1+vx][1+vy];
  };

  while (current_source != current_target) {
    int move;
    if(is_in_square(current_source, current_target))
    {
      move = next_move(current_source, current_target);
    }
    else
    {
      move = state.cpd.get_first_move(current_source, current_target);
      c.access_cnt++;
    }
    // no path exist
    if (move == 0xF) break;
    if ((1 << move) == warthog::HMASK) {
      move = Hsymbol::decode(current_source, current_target, state.mapper, heuristic_func);
    }
    cost += warthog::doublew[move];
    s.x += dx[move];
    s.y += dy[move];
    c.steps++;
    if (limit != -1 && limit <= c.steps)
      break;
    current_source = state.mapper(s);
  }
  return cost;
}

double GetPath(const Index& state, xyLoc s, xyLoc t, std::vector<xyLoc> &path, warthog::jpsp_oracle& oracle,
    int hLevel, int limit)//, int &callCPD)
{
  int current_source = state.mapper(s);
  int current_target = state.mapper(t);
  const int16_t* dx = warthog::dx;
  const int16_t* dy = warthog::dy;
  double cost = 0.0;
  int move = state.cpd.get_first_move(current_source, current_target);
  if ((1 << move) == warthog::HMASK) {
    move = Hsymbol::decode(current_source, current_target, state.mapper, hLevel);
  }

  if(move != 0xF && current_source != current_target){
    oracle.set_goal_location(t.x,t.y);
    auto direction = (warthog::jps::direction)(1 << move);
    int number_step_to_turn = oracle.next_jump_point(s.x, s.y, direction);

    path.push_back(s);

    for(;;){

      for(int i = 0; i < number_step_to_turn; i++){
        s.x += dx[move];
        s.y += dy[move];
        cost += warthog::doublew[move];
        current_source = state.mapper(s);
        path.push_back(s);
        if(current_source == current_target)
          break;
        if (limit != -1 && (int)path.size() >= limit)
          break;
      }

      if(current_source == current_target)
        break;

      move = state.cpd.get_first_move(current_source, current_target);
      // decode the heuristic move
      if ((1 << move) == warthog::HMASK) {
        move = Hsymbol::decode(current_source, current_target, state.mapper, hLevel);
      }
      direction = (warthog::jps::direction)(1 << move);
      number_step_to_turn = oracle.next_jump_point(s.x, s.y, direction);

    }
  }
  return cost;
}


double GetRectWildCardCost(const Index& data, xyLoc s, xyLoc t, int hLevel, Counter& c, int limit) {
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

  auto next_move = [&](int current_source, int current_target)
  {
      xyLoc cs = data.mapper.operator()(current_source);
      xyLoc ct = data.mapper.operator()(current_target);
      int vx = signbit(ct.x - cs.x);
      int vy = signbit(ct.y - cs.y);
      return warthog::v2i[1+vx][1+vy];
  };

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
      move = next_move(sid, tid);
      if ((1<<move) == warthog::HMASK) {
        move = Hsymbol::decode(sid, tid, data.mapper, heuristic_func);
      }
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

double GetInvCPDCost(const Index& data, xyLoc s, xyLoc t, int hLevel, Counter& c, int limit) {
  int (*heuristic_func)(int, int, const Mapper&);
  if (hLevel == 1)
    heuristic_func = Hsymbol::get_heuristic_move1;
  else if (hLevel == 2)
    heuristic_func = Hsymbol::get_heuristic_move2;
  else if (hLevel == 3)
    heuristic_func = Hsymbol::get_heuristic_move3;

  bool source_changed = false;
  int curs = data.mapper(s);
  int curt = data.mapper(t);
  int lhs = -1, rhs = -1, cur_move = -1, move;
  const int16_t* dx = warthog::dx;
  const int16_t* dy = warthog::dy;
  double cost = 0.0;

  auto next_move = [&](int current_source, int current_target)
  {
      xyLoc cs = data.mapper.operator()(current_source);
      xyLoc ct = data.mapper.operator()(current_target);
      int vx = signbit(ct.x - cs.x);
      int vy = signbit(ct.y - cs.y);
      return warthog::v2i[1+vx][1+vy];
  };

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
      move = next_move(sid, tid);
      cost += warthog::doublew[move];
      source.x += dx[move];
      source.y += dy[move];
      sid = data.mapper(source);
      source_changed = true;
      return;
    }
    else {
      if (source_changed || !(tid >= lhs && tid <= rhs)) {
        data.cpd.get_interval(sid, tid, lhs, rhs, cur_move);
        source_changed = false;
        c.access_cnt++;
      }
      move = cur_move;
      if ((1<<move) == warthog::NOMOVE) return;
      if ((1<<move) == warthog::HMASK) {
        move = Hsymbol::decode(tid, sid, data.mapper, heuristic_func);
      }
    }
    cost += warthog::doublew[move];
    target.x += dx[move];
    target.y += dy[move];
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
