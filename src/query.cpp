#include "Hsymbol.h"
#include "query.h"

double GetPathCostSRC(const Index& state, xyLoc s, xyLoc t, int hLevel, int limit) {
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
  int steps = 0;

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
    if(state.row_ordering[current_source] >= state.row_ordering[current_target])
    {
      if(is_in_square(current_source, current_target))
      {
        move = next_move(current_source, current_target);
      }
      else
      {
        move = state.cpd.get_first_move(current_source, current_target);
      }
      // no path exist
      if (move == 0xF) break;
      if ((1 << move) == warthog::HMASK) {
        move = Hsymbol::decode(current_source, current_target, state.mapper, heuristic_func);
      }
      cost += warthog::doublew[move];
      s.x += dx[move];
      s.y += dy[move];
      steps ++;
      if (limit != -1 && limit <= steps)
        break;
      current_source = state.mapper(s);
    }
    else
    {
      if(is_in_square(current_target, current_source))
      {
        move = next_move(current_target, current_source);
      }
      else
      {
        move = state.cpd.get_first_move(current_target, current_source);
      }
      if (move == 0xF) break;
      if ((1 << move) == warthog::HMASK) {
        move = Hsymbol::decode(current_target, current_source, state.mapper, heuristic_func);
      }
      cost += warthog::doublew[move];
      t.x += dx[move];
      t.y += dy[move];
      steps ++;
      if (limit != -1 && limit <= steps)
        break;
      current_target = state.mapper(t);
    }

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
