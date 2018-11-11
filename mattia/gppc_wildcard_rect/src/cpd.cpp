#include "cpd.h"

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>

// compile with -O3 -DNDEBUG

static unsigned char find_first_allowed_out_arc(unsigned short allowed){
	assert(allowed != 0);
	for(int i=0; i<=0xF; ++i)
		if(allowed & (1u << i))
			return i;
	assert(false);
	return 0;
}

void CPD::append_row(int source_node, const std::vector<unsigned short>&allowed_first_move, const std::vector<int>&row_ordering,
		const bool plus_heuristic, const int k_factor, const bool plus_heuristic_2, const int abs_threshold, Mapper mapper,
		std::vector<Rect>&rectangles){

	/**
	 * If current target equals to source node return don't care 0x7FFF
	 * Using row ordering, if current target come before to current source,
	 *
	 *  --- START plus2 ---
	 * if these 2 nodes are far apart each other, more than abs_threshold, and current target is multiple of k_factor
	 * it doesn't substitute with don't care
	 * --- FINISH plus2 ---
	 *
	 *  --- START plus ---
	 * if current target is multiple of k_factor then the program doesn't substitute with don't care
	 * --- FINISH plus ---
	 *
	 * otherwise substitute with don't care
	 */

	auto is_in_rect = [&](int x)
	{
		xyLoc loc_source = mapper.operator()(source_node);
		xyLoc loc_x = mapper.operator()(x);
		for(int i= 0; i<rectangles.size(); i++)
		{
			Rect r = rectangles[i];
			if((loc_x.y >= (loc_source.y-r.n))&&(loc_x.y <= (loc_source.y+r.s)))
			{
				if((loc_x.x >= (loc_source.x - r.w))&&(loc_x.x <= (loc_source.x + r.e)))
				{
					return true;
				}
			}
		}
		return false;
	};

	auto get_allowed = [&](int x){
		if(x == source_node)
			return (unsigned short)0x7FFF;
		if(is_in_rect(x))
			return (unsigned short)0xFFFF;
		else if(row_ordering[source_node] < row_ordering[x]){

			int distance = source_node - x;
			if((plus_heuristic_2 && abs(distance) > abs_threshold && x % k_factor == 0) || (plus_heuristic && x % k_factor == 0)){
				if(allowed_first_move[x] == 0)
					return (unsigned short)0x8000;
				else
					return allowed_first_move[x];
			}else{
				return (unsigned short) 0xFFFF;
			}

		}else if(allowed_first_move[x] == 0)
			return (unsigned short)0x8000;
		else
			return allowed_first_move[x];
	};

	int node_begin = 0;
	unsigned short allowed_up_to_now = get_allowed(0);
	
	for(int i=1; i<(int)allowed_first_move.size(); ++i){
		int allowed_next = allowed_up_to_now & get_allowed(i);
		if(allowed_next == 0){
			entry.push_back((node_begin << 4) | find_first_allowed_out_arc(allowed_up_to_now));
			node_begin = i;
			allowed_up_to_now = get_allowed(i);
		}else
			allowed_up_to_now = allowed_next;
	}
	entry.push_back((node_begin << 4) | find_first_allowed_out_arc(allowed_up_to_now));

	begin.push_back(entry.size());


	#ifndef NDEBUG
	int pos = 0;
	const int node_count = allowed_first_move.size();

	for(int target_node = 0; target_node < node_count; ++target_node){
		if(begin[source_node]+pos+1 != (int)begin[source_node+1] && entry[begin[source_node]+pos+1] <= ((target_node << 4) | 0xF)){
			++pos;
		}

		if(target_node != source_node && allowed_first_move[target_node] != 0){
			assert(allowed_first_move[target_node] & (1<<(entry[begin[source_node]+pos] & 0xF)));
		}
	}
	#endif
}

void CPD::append_rows(const CPD&other){
	int offset = begin.back();
	for(auto x:make_range(other.begin.begin()+1, other.begin.end()))
		begin.push_back(x + offset);
	std::copy(other.entry.begin(), other.entry.end(), back_inserter(entry));
}

