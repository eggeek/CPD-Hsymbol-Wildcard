#include <stdio.h>
#include <stdint.h>
#include <numeric>
#include <algorithm>
#include "ScenarioLoader.h"
#include "Entry.h"
#include "stats.h"
#include "Timer.h"
#include <iostream>
#include <fstream>
#include <unistd.h>

void LoadMap(const char *fname, std::vector<bool> &map, int &w, int &h);

int main(int argc, char **argv)
{

	char filename[255];
	std::vector<xyLoc> thePath;
	std::vector<xyLoc> backward_segment;
	int k_factor, threshold;
	std::vector<bool> mapData;
	int width, height;

	bool pre = false;
	bool run = false;

	//heuristic variables
	bool old_vector = true;
	bool random_order = false;
	bool combined = false;
	bool plus_heuristic = false;
	bool plus_heuristic_2 = false;

	char *heur, *map, *scenario, *data_file;
	char usage[] = "\nUsage: %s <flag> <map> <scenario> <file.csv> [-v] [-e heuristic] [-k k_factor] [-t threshold]\n"
			"\nOptions:\n\t-v: to use new_vector (old vector is the default),\n\t-e: default is same ordering for both\n\t\trnd random,\n\t\tcomb combined,\n\t\tplus plus heur, \n\t\tplus2 plus v.2 heur;"
			"\n\t-k: k factor for comb or plus or plus2, \n\t-t: threshold for plus2 (is % )\n\n";

	//getopt's variables
	extern char *optarg;
	extern int optind, optopt;
	int c, option_counter = 0;

	if(argc >= 5){
		while ((c = getopt(argc, (char **)argv, "ve:k:t:")) != -1) {
			option_counter++;
			switch (c) {
			case 'v':
				old_vector = false;
				break;
			case 'e':
				heur = optarg;
				option_counter++;
				break;
			case 'k':
				k_factor = atoi(optarg);
				option_counter++;
				break;
			case 't':
				threshold = atoi(optarg);
				option_counter++;
				break;
			case ':':
				fprintf(stderr, "Option -%c requires an operand\n", optopt);
				fprintf(stderr, usage, argv[0]);
				break;
			case '?':
				fprintf(stderr, "Unrecognized option: -%c\n", optopt);
				fprintf(stderr, usage, argv[0]);
				break;
			default:
			  abort();
			}
		}

		if(heur != NULL){
			if(strcmp(heur, "rnd") == 0)
			{
				random_order = true;
			}
			else if(strcmp(heur, "comb") == 0)
			{
				combined = true;
				if(k_factor < 1 || k_factor > 100){
					fprintf(stderr, "\n%i: K_factor error (1 <= k <= 100)\n", k_factor);
					fprintf(stderr, usage, argv[0]);
					exit(1);
				}
			}
			else if(strcmp(heur, "plus") == 0)
			{
				plus_heuristic = true;
				if(k_factor < 1 || k_factor > 100){
					fprintf(stderr, "\n%i: K_factor error (1 <= k <= 100)\n", k_factor);
					fprintf(stderr, usage, argv[0]);
					exit(1);
				}
			}
			else if(strcmp(heur, "plus2") == 0)
			{
				plus_heuristic_2 = true;
				if(k_factor < 1 || k_factor > 100 || threshold < 0 || threshold > 100){
					fprintf(stderr, "\nK_factor or threshold error (1 <= k/t <= 100)\n");
					fprintf(stderr, usage, argv[0]);
					exit(1);
				}
			}else{
				fprintf(stderr, "\nHeuristic not recognized \n");
				fprintf(stderr, usage, argv[0]);
				exit(1);
			}
		}

		//check the number of mandatory parameters
		if(argc - option_counter < 5){
			fprintf(stderr, usage, argv[0]);
			exit(1);
		}

		//get value for each mandatory parameter
		while (optind < argc){
			if(strcmp(argv[optind], "full") == 0)
			{
				pre = run = true;
			}
			else if (strcmp(argv[optind], "pre") == 0)
			{
				pre = true;
			}
			else if (strcmp(argv[optind], "run") == 0)
			{
				run = true;
			}
			else if(strstr(argv[optind], ".map") && strstr(argv[optind], ".scen") == NULL)
			{
				map = argv[optind];
			}
			else if(strstr(argv[optind], ".map") && strstr(argv[optind], ".scen"))
			{
				scenario = argv[optind];
			}
			else if(strstr(argv[optind], ".csv"))
			{
				data_file = argv[optind];
			}else{
				fprintf(stderr, usage, argv[0]);
				exit(1);
			}

			if(map == NULL || scenario == NULL || data_file == NULL){
				fprintf(stderr, usage, argv[0]);
				exit(1);
			}
			optind++;
		}
	}else{
		fprintf(stderr, usage, argv[0]);
		exit(1);
	}

	LoadMap(map, mapData, width, height);
	sprintf(filename, "%s-%s", map, GetName());

	if (pre)
	{
		PreprocessMap(mapData, width, height, filename, random_order, old_vector, combined, plus_heuristic, plus_heuristic_2, k_factor, threshold);
	}

	if (!run)
	{
		return 0;
	}

	void *reference = PrepareForSearch(mapData, width, height, filename);

	ScenarioLoader scen(scenario);

	std::vector<Stats> experimentStats;
	Timer t;

	//Get full path, and time to compute it
	for (int x = 0; x < scen.GetNumExperiments(); x++)
    {
		thePath.resize(0);
		backward_segment.resize(0);
		experimentStats.resize(x+1);

		xyLoc s, g;
		s.x = scen.GetNthExperiment(x).GetStartX();
		s.y = scen.GetNthExperiment(x).GetStartY();
		g.x = scen.GetNthExperiment(x).GetGoalX();
		g.y = scen.GetNthExperiment(x).GetGoalY();

		t.StartTimer();
		GetPath(reference, s, g, thePath, backward_segment, plus_heuristic, plus_heuristic_2, k_factor, threshold);
		t.EndTimer();

		experimentStats[x].total_time = t.GetElapsedTime();
		experimentStats[x].lengths.push_back(thePath.size());
		for (unsigned int i = experimentStats[x].path.size(); i < thePath.size(); i++)
			experimentStats[x].path.push_back(thePath[i]);
    }

	//get only 20 moves to count number of steps, and their time, and then get only first move s->t and its time and number of steps
#ifdef DETAILS
	std::vector<xyLoc> first_20_moves;
	int step_counter;
	for (int x = 0; x < scen.GetNumExperiments(); x++)
    {
		first_20_moves.resize(0);
		backward_segment.resize(0);
		step_counter= 0;

		xyLoc s, g;
		s.x = scen.GetNthExperiment(x).GetStartX();
		s.y = scen.GetNthExperiment(x).GetStartY();
		g.x = scen.GetNthExperiment(x).GetGoalX();
		g.y = scen.GetNthExperiment(x).GetGoalY();

		t.StartTimer();
		experimentStats[x].step_20_forward_move = Get20ForwardMoveDetails(reference, s, g, first_20_moves, backward_segment, plus_heuristic, plus_heuristic_2, k_factor, threshold);
		t.EndTimer();
		experimentStats[x].first_20_moves_fwd_time = t.GetElapsedTime();

		thePath.resize(0);
		backward_segment.resize(0);

		t.StartTimer();
		experimentStats[x].step_first_forward_move = GetStepFirstForwardMove(reference, s, g, thePath, backward_segment, plus_heuristic, plus_heuristic_2, k_factor, threshold);
		t.EndTimer();
		experimentStats[x].first_move_fwd_time = t.GetElapsedTime();
    }

#endif

	double sum_total_time = 0.0;
	double sum_20_m_time = 0.0;
	double sum_f_m_time = 0.0;
	double sum_20_m_fwd_step = 0.0;
	double sum_20_m_fwd_step_long_path = 0.0;
	double sum_first_m_fwd_step = 0.0;
	int path_longer_20 = 0;

	for (unsigned int x = 0; x < experimentStats.size(); x++)
	{
		printf("%s\ttotal-time\t%f\ttime-20-moves\t%f\ttotal-len\t%f\tsubopt\t%f\t", map,
			   experimentStats[x].GetTotalTime(), experimentStats[x].Get20MoveTime(),
			   experimentStats[x].GetPathLength(),
			   experimentStats[x].GetPathLength() == scen.GetNthExperiment(x).GetDistance() ? 1.0 :
			   experimentStats[x].GetPathLength() / scen.GetNthExperiment(x).GetDistance()
		);
		if (experimentStats[x].path.size() == 0 ||
			(experimentStats[x].ValidatePath(width, height, mapData) &&
			 scen.GetNthExperiment(x).GetStartX() == experimentStats[x].path[0].x &&
			 scen.GetNthExperiment(x).GetStartY() == experimentStats[x].path[0].y &&
			 scen.GetNthExperiment(x).GetGoalX() == experimentStats[x].path.back().x &&
			 scen.GetNthExperiment(x).GetGoalY() == experimentStats[x].path.back().y))
		{
			printf("valid\n");
		}
		else {
			printf("invalid\n");
		}

		//Time
		sum_total_time += experimentStats[x].GetTotalTime();
		sum_20_m_time += experimentStats[x].Get20MoveTime();
		sum_f_m_time += experimentStats[x].GetFirstMoveTime();

		//Step to compute first 20 moves forward
		sum_20_m_fwd_step += experimentStats[x].step_20_forward_move;
		if(experimentStats[x].path.size() >= 20){
			sum_20_m_fwd_step_long_path += experimentStats[x].step_20_forward_move;
			path_longer_20++;
		}

		//Step to compute first move forward
		sum_first_m_fwd_step += experimentStats[x].step_first_forward_move;
	}

	printf("Save data to csv: %s\n\n", data_file);
	int experiment_size = experimentStats.size();

	std::ofstream csv_file;
	csv_file.open (data_file, std::ios_base::app);

	csv_file << filename << ", "
			<< sum_f_m_time / experiment_size << ", "
			<< sum_20_m_time / experiment_size << ", "
			<< sum_total_time / experiment_size << ", "
			<< sum_20_m_fwd_step / experiment_size << ", "
			<< sum_20_m_fwd_step_long_path / path_longer_20 << ", "
			<< sum_first_m_fwd_step / experiment_size << std::endl;

	csv_file.close();

	remove(filename);

	return 0;
}

void LoadMap(const char *fname, std::vector<bool> &map, int &width, int &height)
{
	FILE *f;
	f = fopen(fname, "r");
	if (f)
    {
		fscanf(f, "type octile\nheight %d\nwidth %d\nmap\n", &height, &width);
		map.resize(height*width);
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				char c;
				do {
					fscanf(f, "%c", &c);
				} while (isspace(c));
				map[y*width+x] = (c == '.' || c == 'G' || c == 'S');
				//printf("%c", c);
			}
			//printf("\n");
		}
		fclose(f);
    }
}
