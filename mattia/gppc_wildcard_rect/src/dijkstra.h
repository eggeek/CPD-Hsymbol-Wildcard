#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <algorithm>
#include <vector>
#include <limits>
#include <cassert>
#include <iostream>

#include "adj_graph.h"
#include "heap.h"
#include "mapper.h"

class Dijkstra{
public:
	Dijkstra(const AdjGraph&g):
		g(g), q(g.node_count()), dist(g.node_count()), allowed(g.node_count()){}

	const std::vector<unsigned short>&run(int source_node){
		std::fill(dist.begin(), dist.end(), std::numeric_limits<int>::max());
		std::fill(allowed.begin(), allowed.end(), 0);

		dist[source_node] = 0;		
		allowed[source_node] = 0;

		auto reach = [&](int v, int d, unsigned short first_move){
			if(d < dist[v]){
				q.push_or_decrease_key(v, d);
				dist[v] = d;
				allowed[v] = first_move;
			}else if(d == dist[v]){
				allowed[v] |= first_move;
			}
		};

		for(int i=0; i<g.out_deg(source_node); ++i){
			auto a = g.out(source_node, i);
			reach(a.target, a.weight, 1 << i);
		}

		while(!q.empty()){
			int x = q.pop();

			for(auto a:g.out(x))
				reach(a.target, dist[x] + a.weight, allowed[x]);
		}

		#ifndef NDEBUG
		for(int u=0; u<g.node_count(); ++u)
			for(auto uv : g.out(u)){
				int v = uv.target;
				assert(dist[u] >= dist[v] - uv.weight);
			}
		#endif

		return allowed;
	}

	const std::vector<unsigned short>&run(int source_node, Mapper mapper, std::vector<std::vector<Rect>>&rectangles_dim){

		allowed = run(source_node);
		std::vector<Rect> rectangles = getMaxRect(mapper.operator()(source_node), mapper);
		rectangles_dim.push_back(rectangles);
		return allowed;

	}

	int distance(int target)const{
		return dist[target];
	}
private:
	const AdjGraph&g;
	min_id_heap<int>q;
	std::vector<int>dist;
	std::vector<unsigned short>allowed;

    std::vector<bool> check_surrounding(xyLoc loc, Mapper mapper)
    {
        /* allowed direction is a vector containing a boolean for each direction the rectangle can expand:
         * allowed_directions[0] -> NORTH-WEST
         * allowed_directions[1] -> NORTH
         * allowed_directions[2] -> NORTH-EAST
         * allowed_directions[3] -> EAST
         * allowed_directions[4] -> SOUTH-EAST
         * allowed_directions[5] -> SOUTH
         * allowed_directions[6] -> SOUTH-WEST
         * allowed_directions[7] -> WEST
         * */
        std::vector<bool> allowed_directions(8, false);
        if(mapper.operator()(loc) == -1)
        {
            bool try_access_obstacle = false;
            assert(try_access_obstacle);
        }
        else
        {
            auto is_available = [&](xyLoc loc, int dx, int dy)
            {
                xyLoc neighbour;
                neighbour.x = loc.x + dx;
                neighbour.y = loc.y + dy;
                if((!((dx == 0)&&(dy == 0)))&&(neighbour.x < mapper.width())&&(neighbour.x >= 0)&&(neighbour.y >= 0 )
                   &&(neighbour.y < mapper.height()))
                {
                    if(mapper.operator()(neighbour) != -1)
                    {
                        return true;
                    }
                }
                return false;
            };

            int index = 0;
            for(int dx = -1; dx<2; dx++)
            {
                int dy = -1;
                if(is_available(loc, dx, dy))
                {
                    allowed_directions[index] = true;
                }
                index++;
            }
            for(int dy  =0; dy<2; dy++)
            {
                int dx = 1;
                if(is_available(loc, dx, dy))
                {
                    allowed_directions[index] = true;
                }
                index++;
            }
            for(int dx = 0; dx > -2; dx--)
            {
                int dy = 1;
                if(is_available(loc, dx, dy))
                {
                    allowed_directions[index] = true;
                }
                index++;
            }
            int dx = -1;
            int dy =  0;
            if(is_available(loc, dx, dy))
            {
                allowed_directions[index] = true;
            }
        }
        return allowed_directions;
    }

    Rect find_rectangle(xyLoc source_node, Mapper mapper, std::vector<int>&allowed_directions_int)
    {
        std::vector<bool> direction_available(4, false);
        std::vector<int> dimensions(4,0);
        for(int i=0; i<allowed_directions_int.size(); i++)
        {

            int index = allowed_directions_int[i]/2;
            direction_available[index] = true;
            dimensions[index] = 1;
        }

        auto is_direction_allowed = [&](int direction)
        {
            if(direction_available[direction])
            {
                xyLoc neighbour;
                int direction_pre, direction_next;
                if((direction == 1)||(direction == 3))
                {
                    if(direction ==3)
                    {
                        neighbour.x = source_node.x - dimensions[direction] - 1;
                    }
                    else
                    {
                        //assert(direction == 1);
                        neighbour.x = source_node.x + dimensions[direction] + 1;
                    }
                    if((neighbour.x > -1) && (neighbour.x < mapper.width()))
                    {
                        int up = source_node.y-dimensions[0];
                        int down = source_node.y+dimensions[2];
                        for(int y= up; y<= down; y++)
                        {
                            neighbour.y = y;
                            if((y > -1)&&(y<mapper.height())&&(mapper.operator()(neighbour) != -1))
                            {
                                bool is_natural_move = false;
                                for(int j=0; j<8; j++) {
                                    if ((allowed[mapper.operator()(neighbour)] & (1 << j)) != 0) {
                                        int dest = g.out(mapper.operator()(source_node), j).target;
                                        if (is_nat(source_node, mapper.operator()(dest), neighbour)) {
                                            is_natural_move = true;
                                            break;
                                        }
                                    }
                                }
                                if(!is_natural_move)
                                {
                                    /*std:: cout << "not natural source " << mapper.operator()(source_node) << " " <<
                                    neighbour.x << " " << neighbour.y << "\n";*/
                                    return false;
                                }
                            }
                        }
                        return true;
                    }
                    else
                    {
                        //std:: cout << "out of bound source " << mapper.operator()(source_node) << "\n";
                        return false;
                    }
                }
                else
                {
                    if(direction == 0)
                    {
                        neighbour.y = source_node.y - dimensions[direction] - 1;
                    }
                    else
                    {
                        //assert(direction == 2);
                        neighbour.y = source_node.y + dimensions[direction] + 1;
                    }
                    if((neighbour.y > -1) && (neighbour.y < mapper.height()))
                    {

                        int left = source_node.x-dimensions[3];
                        int right = source_node.x+dimensions[1];
                        for(int x= left; x<=right; x++)
                        {
                            neighbour.x = x;
                            if((x > -1)&&(x<mapper.width())&&(mapper.operator()(neighbour) != -1))
                            {
                                //std::cout << mapper.operator()(source_node) << " " << mapper.operator()(neighbour) << "\n";
                                bool is_natural_move = false;
                                for(int j=0; j<8; j++) {
                                    if ((allowed[mapper.operator()(neighbour)] & (1 << j)) != 0) {
                                        int dest = g.out(mapper.operator()(source_node), j).target;
                                        if (is_nat(source_node, mapper.operator()(dest), neighbour)) {
                                            is_natural_move = true;
                                            break;
                                        }
                                    }
                                }
                                if(!is_natural_move)
                                {
                                    return false;
                                }
                            }
                        }
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }

            }
        };

        while(direction_available[0]||direction_available[1]||direction_available[2]||direction_available[3])
        {
            for(int i=0; i<4; i++)
            {
                if(direction_available[i])
                {
                    if(is_direction_allowed(i))
                    {
                        dimensions[i]++;
                    }
                    else
                    {
                        direction_available[i] = false;
                    }
                }
            }
        }
        Rect toReturn;
        toReturn.n = dimensions[0];
        toReturn.e = dimensions[1];
        toReturn.s = dimensions[2];
        toReturn.w = dimensions[3];
        return toReturn;
    }

    std::vector<Rect> getMaxRect(xyLoc source_node, Mapper mapper)
    {
        std::vector<Rect> toReturn;
        if(mapper.operator()(source_node) == -1)
        {
            bool source_is_obstacle = false;
            assert(source_is_obstacle);
        }
        std::vector<bool> allowed_directions = check_surrounding(source_node, mapper);
        std::vector<int> allowed_directions_pos;
        int dimension = allowed_directions.size();
        for(int i=0; i<dimension; i++)
        {
            //std::cout << std::boolalpha << allowed_directions[i];
            if((i == 0)&&(allowed_directions[0]))
            {
                while((dimension>0)&&(allowed_directions[dimension-1]))
                {
                    if((dimension-1)%2 == 1)
                    {
                        allowed_directions_pos.push_back(dimension-1);
                    }
                    dimension--;
                }
            }
            else if(allowed_directions[i])
            {
                if(i%2 == 1)
                {
                    allowed_directions_pos.push_back(i);
                }
            }
            else
            {
                if(allowed_directions_pos.size() > 0)
                {
                    toReturn.push_back(find_rectangle(source_node, mapper, allowed_directions_pos));
                    allowed_directions_pos.clear();
                }

            }
        }
        if(allowed_directions_pos.size() > 0)
        {
            toReturn.push_back(find_rectangle(source_node, mapper, allowed_directions_pos));
        }
        return toReturn;
    }

	bool is_nat(xyLoc source, xyLoc target, xyLoc node)
	{
		int dx_target = (source.x - target.x);
		int dy_target = (source.y - target.y);
		int dx_node = (source.x - node.x);
		int dy_node = (source.y - node.y);
		if(((dx_node > 0) && (dx_target > 0))||((dx_node < 0) && (dx_target < 0))||((dx_node == 0) && (dx_target == 0)))
		{
			if((dy_node > 0) && (dy_target > 0))
				return true;
			if((dy_node < 0) && (dy_target < 0))
				return true;
			if((dy_node == 0) && (dy_target == 0))
				return true;
		}
		//std::cout << source_node << " " << dx_node << " " << dy_node << " " << dx_target << " " << dy_target << "\n";
		return false;
	}
};

#endif
