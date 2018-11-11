#include <deque>
#include <vector>
#include <algorithm>
#include <assert.h>
#include "Entry.h"

#include "Timer.h"
#include "list_graph.h"
#include "mapper.h"
#include "cpd.h"
#include "order.h"
#include "adj_graph.h"
#include "dijkstra.h"
#include "balanced_min_cut.h"
#include "prefer_zero_cut.h"
#include <cstdio>
#include "stats.h"
#include <random>
#include <iterator>
#include <functional>
#include <string>
#include <sys/stat.h>
#include <iostream>
#include <limits.h>

using namespace std;

#ifdef USE_PARALLELISM
#include <omp.h>
#endif

const char *GetName()
{
	#ifndef USE_CUT_ORDER
	return "DFS-SRC-RLE";
	#else
	return "METIS-CUT-SRC-RLE";
	#endif
}

void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename, const bool random_row, const bool old_vector,
		const bool mix, const bool plus_heuristic, const bool plus_heuristic_2, const int k_factor, const int threshold)
{
	Mapper mapper(bits, width, height);
	printf("width = %d, height = %d, node_count = %d\n", width, height, mapper.node_count());

	printf("Computing node order\n");
	#ifndef USE_CUT_ORDER
	NodeOrdering order = compute_real_dfs_order(extract_graph(mapper));
	#else
	NodeOrdering order = compute_cut_order(extract_graph(mapper), prefer_zero_cut(balanced_min_cut));	
	#endif
	mapper.reorder(order);


	//With plus or plus2 I need to get only old or new array,
	//so i don't do anything in that case
	printf("Computing Row Order\n");
	AdjGraph g(extract_graph(mapper));

	vector<int> row_ordering;

	random_device rnd_device;
	mt19937 mersenne_engine(rnd_device());

	if(random_row){
//		 struct stat buffer;
//		if(stat ("rnd_order", &buffer) == 0){
//			FILE*f = fopen("rnd_order", "rb");
//			row_ordering = load_vector<int>(f);
//			fclose(f);
//
//		}else{

			for(int x = 0; x < g.node_count(); x++){
				row_ordering.push_back(x);
			}
			shuffle(row_ordering.begin(), row_ordering.end(), mersenne_engine);

//			FILE*f = fopen("rnd_order", "wb");
//			save_vector(f, row_ordering);
//			fclose(f);
//		}
	}else{
		//In case of plus or plus2 the program enters here because rnd is false
		//and with plus or plus2 we use the same vector
		if(old_vector)
			row_ordering = order.get_old_array();
		else
			row_ordering = order.get_new_array();

		if(mix){
			std::uniform_int_distribution<int> distribution(0, row_ordering.size()-1);
			int number_swap = (int)row_ordering.size()*k_factor/100;

			for(int i = 0; i < number_swap; i++){
				swap(row_ordering[distribution(mersenne_engine)], row_ordering[distribution(mersenne_engine)]);
				i++;
			}
		}
	}

	printf("Computing first-move matrix\n");
	vector<vector<Rect>> rectangles;
	CPD cpd;
	{
		{
			Dijkstra dij(g);
			Timer t;
			t.StartTimer();
			dij.run(0);
			t.EndTimer();
			printf("Estimated sequential running time : %dmin\n", static_cast<int>(t.GetElapsedTime()*g.node_count()/60.0));
		}

		//cmpute the treshold
		int abs_threshold = INT_MAX;
		if(plus_heuristic_2)
			abs_threshold = g.node_count()*threshold/100;

		#ifndef USE_PARALLELISM
		Dijkstra dij(g);
		for(int source_node=0; source_node < g.node_count(); ++source_node){
			if(source_node % (g.node_count()/10) == 0)
				printf("%d of %d done\n", source_node, g.node_count());

			const auto&allowed = dij.run(source_node);
			cpd.append_row(source_node, allowed, row_ordering, plus_heuristic, k_factor, plus_heuristic_2, abs_threshold);
		}
		#else
		printf("Using %d threads\n", omp_get_max_threads());
		vector<CPD>thread_cpd(omp_get_max_threads());
		vector<vector<vector<Rect>>> thread_rectangles(omp_get_max_threads());

		int progress = 0;

		#pragma omp parallel
		{
			const int thread_count = omp_get_num_threads();
			const int thread_id = omp_get_thread_num();
			const int node_count = g.node_count();

			int node_begin = (node_count*thread_id) / thread_count;
			int node_end = (node_count*(thread_id+1)) / thread_count;

			AdjGraph thread_adj_g(g);
			Dijkstra thread_dij(thread_adj_g);
			Mapper thread_mapper = mapper;

			for(int source_node=node_begin; source_node < node_end; ++source_node){
				vector<unsigned short> allowed = thread_dij.run(source_node, mapper, thread_rectangles[thread_id]);
				thread_cpd[thread_id].append_row(source_node, allowed,
						row_ordering, plus_heuristic, k_factor, plus_heuristic_2, abs_threshold, thread_mapper, *(thread_rectangles[thread_id].end()-1));
				#pragma omp critical 
				{
					++progress;
					if((g.node_count()/10 != 0)&&(progress % (g.node_count()/10) == 0)){
						printf("%d of %d done\n", progress, g.node_count());
						fflush(stdout);
					}
				}
			}
		}

		for(auto&x:thread_cpd)
			cpd.append_rows(x);
		for(auto&x:thread_rectangles)
			rectangles.insert(rectangles.end(), x.begin(), x.end());
		#endif
	}
	/*
	for(int i=0; i<rectangles.size(); i++)
	{
		for(int j=0; j<rectangles[i].size(); j++)
		{
			cout << rectangles[i][j].n << " "  << rectangles[i][j].e << " "  << rectangles[i][j].s << " "  << rectangles[i][j].w << " ";
		}
		cout << endl;
	}
	cout << endl;
	for(int i=0; i<g.node_count(); i++)
	{
		cout << row_ordering[i] << " ";
	}
	cout << endl;
	for(int i=0; i<cpd.getBegin().size()-1; i++)
	{
		for(int j= cpd.getBegin()[i]; j<cpd.getBegin()[i+1]; j++)
		{
			cout << hex << *(cpd.getEntry().begin() + j) << " ";
		}
		cout << endl;
	}*/

	printf("Saving data to %s\n", filename);
	//Save CPD + column order + row order
	FILE*f = fopen(filename, "wb");
	order.save(f);
	cpd.save(f);
	save_vector(f, row_ordering);
	fclose(f);

	//Save only CPD to check its size
	string my_file_name;

	if(GetName() == "DFS-SRC-RLE")
		my_file_name = "dfs";
	else
		my_file_name = "cut";

	if(random_row)
		my_file_name += "_rnd";

	if(old_vector)
		my_file_name += "_old";
	else
		my_file_name += "_new";

	if(plus_heuristic)
		my_file_name += "_plus_"+ std::to_string(k_factor);

	if(plus_heuristic_2)
		my_file_name += "_plus2_"+ std::to_string(k_factor)+"_"+ std::to_string(threshold);

	if(mix)
		my_file_name += "_mix_"+ std::to_string(k_factor);

	string cpd_file = my_file_name +".cpd";

	f = fopen(cpd_file.c_str(), "wb");
	cpd.save(f);
	fclose(f);

	//compute the size of CPD, and save it into a .csv file
	streampos begin,end;
	ifstream cpd_file_if(cpd_file, ios::binary);
	begin = cpd_file_if.tellg();
	cpd_file_if.seekg(0, ios::end);
	end = cpd_file_if.tellg();
	cpd_file_if.close();

	std::ofstream size_csv_file;
	string size_file_name = "size_"+my_file_name+".csv";

	size_csv_file.open(size_file_name.c_str(), std::ios_base::app);
	size_csv_file << filename << ", " << end-begin;

	struct stat results;
	if (stat(cpd_file.c_str(), &results) == 0)
		size_csv_file << ", " << results.st_size;
	size_csv_file << std::endl;

	size_csv_file.close();

	remove(cpd_file.c_str());

	printf("Done\n");
}

struct State{
	CPD cpd;
	Mapper mapper;
	AdjGraph graph;
	vector<int> row_ordering;
	vector<unsigned char> reverse_moves;
	int source_node;
	int target_node;
};

/**
 * Load all data necessary to compute the full path
 */
void *PrepareForSearch(std::vector<bool> &bits, int w, int h, const char *filename)
{
	printf("Loading preprocessing data\n");
	State*state = new State;
	
	state->mapper = Mapper(bits, w, h);

	FILE*f = fopen(filename, "rb");
	NodeOrdering order;
	order.load(f);
	state->cpd.load(f);
	state->row_ordering = load_vector<int>(f);
	fclose(f);

	state->mapper.reorder(order);

	state->graph = AdjGraph(extract_graph(state->mapper));
	state->source_node = -1;

	printf("Loading done\n");

	return state;
}

/*
 * It return the full path from s toward g
 */
void GetPath(void *data, const xyLoc s, const xyLoc g, vector<xyLoc> &thePath, vector<xyLoc> &backward_segment,
		const bool plus_heuristic, const bool plus_heuristic_2, const int k_factor, const int threshold)
{
	State*state = static_cast<State*>(data);

	int current_source = state->mapper(s);
	int current_target = state->mapper(g);

	unsigned char move;

	//check if there is no path between s and g, or s and g are the same node
	if(state->row_ordering[current_source] < state->row_ordering[current_target]){
		move = state->cpd.get_first_move(current_target, current_source);
	}else{
		move = state->cpd.get_first_move(current_source, current_target);
	}

	if(move != 0xF && current_source != current_target){

		int abs_threshold = INT_MAX;
		if(plus_heuristic_2)
			abs_threshold = state->graph.node_count() * threshold /100;

		//Push back the source into the "forward" path vector
		thePath.push_back(s);

		for(;;){

			if(state->row_ordering[current_source] < state->row_ordering[current_target]){

				int distance = current_source - current_target;

				//in case the if condition is true, it means we are lucky, and we compute a move forward,
				//otherwise we compute a move backward
				if((plus_heuristic_2 && abs(distance) > abs_threshold && current_target % k_factor == 0) || (plus_heuristic && current_target % k_factor == 0)){

					move = state->cpd.get_first_move(current_source, current_target);
					current_source = state->graph.out(current_source, move).target;
					if(current_source == current_target)
						break;

					thePath.push_back(state->mapper(current_source));
				}else{
					move = state->cpd.get_first_move(current_target, current_source);
					current_target = state->graph.out(current_target, move).target;
					if(current_source == current_target)
						break;

					backward_segment.push_back(state->mapper(current_target));
				}
			}else{
				//forward move
				move = state->cpd.get_first_move(current_source, current_target);
				current_source = state->graph.out(current_source, move).target;
				if(current_source == current_target)
					break;

				thePath.push_back(state->mapper(current_source));
			}
		}

		//get the "backward" nodes in reversed order and push them into the path
		for(int q = backward_segment.size()-1; q >= 0; q--)
			thePath.push_back(backward_segment[q]);

		//push the goal node
		thePath.push_back(g);
	}
}

#ifdef DETAILS
/*
 *It return the number of step necessary to compute the first 20 move.
 */
int Get20ForwardMoveDetails(void *data, const xyLoc s, const xyLoc g, vector<xyLoc> &first_20_moves, vector<xyLoc> &backward_segment,
		const bool plus_heuristic, const bool plus_heuristic_2, const int k_factor, const int threshold)
{
	State*state = static_cast<State*>(data);

	int current_source = state->mapper(s);
	int current_target = state->mapper(g);

	unsigned char move;
	int counter = 0; //count the number of step to compute first 20 move forward

	if(state->row_ordering[current_source] < state->row_ordering[current_target]){
		move = state->cpd.get_first_move(current_target, current_source);
	}else{
		move = state->cpd.get_first_move(current_source, current_target);
	}

	if(move != 0xF && current_source != current_target){

		first_20_moves.push_back(s);
		int abs_threshold = state->graph.node_count()*threshold/100;

		for(;;){

			counter++;

			if(state->row_ordering[current_source] < state->row_ordering[current_target]){

				int distance = current_source - current_target;
				if((plus_heuristic_2 && abs(distance) > abs_threshold && current_target % k_factor == 0) || (plus_heuristic && current_target % k_factor == 0)){

					move = state->cpd.get_first_move(current_source, current_target);
					current_source = state->graph.out(current_source, move).target;

					first_20_moves.push_back(state->mapper(current_source));

					if(current_source == current_target || (first_20_moves.size() > 20))
						return counter;
				}else{
					move = state->cpd.get_first_move(current_target, current_source);
					current_target = state->graph.out(current_target, move).target;

					backward_segment.push_back(state->mapper(current_target));

					if(current_source == current_target || (first_20_moves.size() > 20))
						return counter;
				}
			}else{
				move = state->cpd.get_first_move(current_source, current_target);
				current_source = state->graph.out(current_source, move).target;

				first_20_moves.push_back(state->mapper(current_source));

				if(current_source == current_target || (first_20_moves.size() > 20))
					return counter;
			}
		}
	}
	return counter;
}

/**
 * It return the number of step to compute only the first move forward
 */
int GetStepFirstForwardMove(void *data, const xyLoc s, const xyLoc g, vector<xyLoc> &thePath, vector<xyLoc> &backward_segment,
		const bool plus_heuristic, const bool plus_heuristic_2, const int k_factor, const int threshold){
	State*state = static_cast<State*>(data);

	int current_source = state->mapper(s);
	int current_target = state->mapper(g);

	unsigned char move;
	int step_counter = 0;

	if(state->row_ordering[current_source] < state->row_ordering[current_target]){
		move = state->cpd.get_first_move(current_target, current_source);
	}else{
		move = state->cpd.get_first_move(current_source, current_target);
	}

	if(move != 0xF && current_source != current_target){
		int abs_threshold = state->graph.node_count() * threshold / 100;

		for(;;){

			step_counter++;

			if(state->row_ordering[current_source] < state->row_ordering[current_target]){

				int distance = current_source - current_target;
				if((plus_heuristic_2 && abs(distance) > abs_threshold && current_target % k_factor == 0) || (plus_heuristic && current_target % k_factor == 0)){
					move = state->cpd.get_first_move(current_source, current_target);
					current_source = state->graph.out(current_source, move).target;

					thePath.push_back(state->mapper(current_source));
					return step_counter;
				}else{
					move = state->cpd.get_first_move(current_target, current_source);
					current_target = state->graph.out(current_target, move).target;

					backward_segment.push_back(state->mapper(current_target));

					if(current_source == current_target)
						return step_counter;
				}
			}else{
				move = state->cpd.get_first_move(current_source, current_target);
				current_source = state->graph.out(current_source, move).target;

				thePath.push_back(state->mapper(current_source));
				return step_counter;
			}
		}
	}
	return step_counter;
}

#endif

